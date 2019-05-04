#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{

    //To begin, your Li should instantiate an accumulated ray color that begins as black,
    //and an accumulated ray throughput color that begins as white. The throughput will be
    //used to determine when your ray path terminates via the Russian Roulette heuristic.
    //Next, simply declare a while loop that compares some current depth value to 0,
    //assuming that depth began as the maximum depth value. Within this loop, we will
    //add a check that breaks the loop early if the Russian Roulette conditions are satisfied.
    //Just after this while loop, your Li code should return the accumulated ray color,
    //which will be computed within the loop.

    Color3f light_color(0.f);
    Color3f brdf_color(0.f);
    Color3f L(0.f);
    Color3f beta(1.f);
    BxDFType sampled_type = BSDF_DIFFUSE;
    BxDFType type = BSDF_ALL;
    int limit = depth;
    int len = scene.lights.length();
    if (len == 0) {
        return Color3f(0.f);
    }

    Ray ray_copy = ray;
    while (depth > 0) {
        Intersection isect = Intersection();
        //checks if there is an intersection
        if (!scene.Intersect(ray_copy, &isect)) {
            break;
        }

        Vector3f woW = - ray_copy.direction;
        //check if the intersected object has a material
        if (!isect.objectHit->GetMaterial()) {
            if ((BSDF_SPECULAR & sampled_type) != 0 || depth == limit) {
                L += beta * isect.Le(woW);
            }
            break;
        }

        isect.ProduceBSDF();
        //light pdf sampling
        int idx = std::min((int) std::floor(sampler->Get1D() * len),
                           len - 1);
        const std::shared_ptr<Light> &light = scene.lights[idx];
        Vector3f wiW_light(0.f);
        float light_pdf = 0.f;
        Point2f xi = sampler->Get2D();
        Color3f Li1 = light->Sample_Li(isect, xi, &wiW_light, &light_pdf);
        light_pdf /= len;
        Li1 /= light_pdf;

        //create a new ray with wiW
        Ray newRay_light = isect.SpawnRay(wiW_light);
        Intersection closest = Intersection();
        //check for intersection b/w newray and the scene
        bool isIntersecting = scene.Intersect(newRay_light, &closest);

        //shadow test
        if (!isIntersecting ||
                closest.objectHit == nullptr ||
                closest.objectHit->areaLight != scene.lights[idx]) {
            Li1 = Color3f(0.f);
        }
        //Set ωi to the normalized vector from the reference
        //Intersection's point to the Shape's intersection point.
        wiW_light = glm::normalize(wiW_light);
        Color3f f1 = isect.bsdf->f(woW, wiW_light);
        float dot = AbsDot(wiW_light, isect.normalGeometric);
        light_color = f1 * Li1 * dot;
        float w_light = PowerHeuristic(1, light_pdf, 1, isect.bsdf->Pdf(woW, wiW_light));
        light_color *= w_light;

        //brdf sampling
        Vector3f wiW_brdf(0.f);
        xi = sampler->Get2D();
        float brdf_pdf = 0.f;
        Color3f f2 = isect.bsdf->Sample_f(woW, &wiW_brdf, xi, &brdf_pdf);

        //direct illumination sampling ray
        Ray bsdf_ray = isect.SpawnRay(wiW_brdf);
        Intersection bsdf_isect;
        Color3f Li2(0.0f);
        if (scene.Intersect(bsdf_ray, &bsdf_isect)) {
            if (bsdf_isect.objectHit->areaLight.get() == scene.lights[idx].get()) {
                Li2 = light->L(bsdf_isect, -wiW_brdf);
            }
        }

        //if pdf is not zero divide f by pdf
        if(brdf_pdf > 0.f) {
            f2 /= brdf_pdf;
        } else {
            Li2 = Color3f(0.f);
        }

        //get the lambertian term
        dot = AbsDot(wiW_brdf, isect.normalGeometric);
        brdf_color = f2 * Li2 * dot;
        float w_brdf = PowerHeuristic(1, brdf_pdf, 1,
                                      scene.lights[idx]->Pdf_Li(isect, wiW_brdf) / len);
        brdf_color *= w_brdf;

        Color3f color = light_color + brdf_color;
        L += beta * color;


        xi = sampler->Get2D();
        Vector3f wiW;
        float pdf;

        //gloabl illumination
        Color3f f = isect.bsdf->Sample_f(woW, &wiW, xi, &pdf, type, &sampled_type);
        if (pdf > 0.f) {
            beta = beta * f * AbsDot(wiW, isect.normalGeometric) / pdf;
        }

        //Russian Roulette Ray Termination
        float q = sampler->Get1D();
        float max = glm::max(beta.x, glm::max(beta.y, beta.z));
        if (depth < 3) {
            if (max < q) break;
            beta /= max;

        }

        //update ray and decrease the depth
        depth--;
        ray_copy = isect.SpawnRay(wiW);
    }

    return L;
}


float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //compute the denominator
    float denom = nf * fPdf + ng * gPdf;
    //check division by 0 and if either fPdf or gPDF is 0 return 0
    if (denom == 0.f || fPdf * gPdf == 0) {
        return 0.f;
    }
    return nf * fPdf / denom;
}

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //compute the numerator
    float num = nf * fPdf * nf * fPdf;
    //compute the denominator
    float denom = nf * nf * fPdf * fPdf + ng * ng * gPdf * gPdf;
    //check division by 0 and if either fPdf or gPDF is 0 return 0
    if (denom == 0.f || fPdf * gPdf == 0) {
        return 0.f;
    }
    return num / denom;
}
