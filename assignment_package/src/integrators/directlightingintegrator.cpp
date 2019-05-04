#include "directlightingintegrator.h"
#include "fulllightingintegrator.h"
Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f light_color(0.f);
    Color3f brdf_color(0.f);
    Color3f Le(0.f);
    Intersection isect = Intersection();
    //checks if there is an intersection
    if (scene.Intersect(ray, &isect)) {
        Vector3f woW = -ray.direction;
        //get emitted light
        Le = isect.Le(woW);

        //if max depth is reached return Le
        if (depth == 0) {
            return Le;
        }

        //check if the intersected object has a material
        if (isect.ProduceBSDF()) {

            //light pdf sampling
            int len = scene.lights.length();
            if (len == 0) {
                return Color3f(0.f);
            }
            int idx = std::min((int) std::floor(sampler->Get1D() * len),
                               len - 1);

            Vector3f wiW_light(0.f);
            float light_pdf = 0.f;
            //get xi
            Point2f xi = sampler->Get2D();
            Color3f Li1 = scene.lights[idx]->Sample_Li(isect, xi, &wiW_light, &light_pdf);

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

            //direct illumination sampling ray***
            Ray bsdf_ray = isect.SpawnRay(wiW_brdf);
            Intersection bsdf_isect;
            Color3f Li2(0.0f);
            if (scene.Intersect(bsdf_ray, &bsdf_isect)) {
                if (bsdf_isect.objectHit->areaLight.get() == scene.lights[idx].get()) {
                    Li2 = bsdf_isect.Le(-wiW_brdf);
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

        } else {
            return Le;
        }
    }
    //Return the light emitted along ωi from our intersection point.
    return Le + light_color + brdf_color;
}
