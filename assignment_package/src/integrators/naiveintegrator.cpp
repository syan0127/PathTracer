#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //TODO

    Color3f color(0.f);
    Color3f Le(0.f);
    Intersection isect = Intersection();
    //checks if there is an intersection
    if (scene.Intersect(ray, &isect)) {
        Vector3f woW = -ray.direction;
        //get Light emitted
        Le = isect.Le(woW);

        //if maxdepth is reached, return Le
        if (depth == 0) {
            return Le;
        }

        //check if the intersected object
        //has a material
        if (isect.ProduceBSDF()) {
            Vector3f wiW(0.f);
            Point2f xi = sampler->Get2D();
            float pdf = 0.f;
            Color3f f = isect.bsdf->Sample_f(woW, &wiW, xi, &pdf);
            //if pdf is not zero divide f by pdf
            if(pdf > 0.f) {
                f /= pdf;
            } else {
                return Color3f(0.f);
            }
            //create a new ray with wiW
            Ray newRay = isect.SpawnRay(wiW);
            //recursive call for Li term
            Color3f L = Li(newRay, scene, sampler, depth - 1);
            //get the lambertian term
            float dot = AbsDot(wiW, isect.normalGeometric);
            color = f * L * dot;
        } else {
            return Le;
        }

    }

    return Le + color;
}
