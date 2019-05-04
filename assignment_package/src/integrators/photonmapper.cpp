#include "photonmapper.h"
#include <scene/lights/diffusearealight.h>
#include <warpfunctions.h>
#include <integrators/fulllightingintegrator.h>
#include <integrators/directlightingintegrator.h>

PhotonMapper::PhotonMapper(int numPhotons, std::vector<Photon> *photons, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit, KDTree *kdtree, float searchRadius)
    : Integrator(Bounds2i(Point2i(0,0), Point2i(0,0)), s, sampler, recursionLimit), preprocessing(true), numPhotons(numPhotons), photons(photons), kdtree(kdtree), searchRadius(searchRadius)
{}

PhotonMapper::PhotonMapper(Bounds2i bounds, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit, KDTree *kdtree, float searchRadius)
    : Integrator(bounds, s, sampler, recursionLimit), preprocessing(false), numPhotons(0), photons(nullptr), kdtree(kdtree), searchRadius(searchRadius)
{
    directIntegrator = new DirectLightingIntegrator(bounds, s, sampler, recursionLimit);
}


float getMaxEmittedLightChannel(Color3f color) {
    return std::max(std::max(color[0], color[1]), color[2]);
}

void PhotonMapper::Render()
{
    // PhotonMapper's Render() function has
    // two modes: when preprocessing, it traces
    // a collection of photons through the scene
    // and stores them in the given k-d tree.
    // If not preprocessing, it runs like a regular Integrator
    // and invokes Li().
    if(preprocessing)
    {
        // Determine how many photons to assign to each light source
        // given numPhotons and the intensity of each light.
        // Shoot a number of photons equal to numPhotons from
        // the lights, bouncing them through the scene and pushing
        // back the result of each bounce to the photons vector
        // stored in the PhotonMapper.

        // Find the total emitted light in the scene

        int photonsPerLight = numPhotons / scene->lights.size();
        for(std::shared_ptr<Light> light: scene->lights) {
            // Send out the correct number of protons
            for(int i = 0; i < photonsPerLight; ++i) {
                Color3f Le = ((DiffuseAreaLight*) light.get())->emittedLight;
                Point2f xi = sampler->Get2D();
                float pdf = 0.f;
                Intersection shapeIsect = ((DiffuseAreaLight*) light.get())->shape->Sample(xi, &pdf);

                xi = sampler->Get2D();
                //get random direction
                Point3f direction = WarpFunctions::squareToHemisphereCosine(xi);

                //compute N, T, B
                Vector3f normal = shapeIsect.normalGeometric;
                Vector3f tangent = glm::cross(Vector3f(0, 1, 0), normal);
                Vector3f bitangent = glm::cross(normal, tangent);

                //compute transformation matrix
                glm::mat4 trans_mat (glm::normalize(glm::vec4(tangent, 0)),
                                     glm::normalize(glm::vec4(bitangent, 0)),
                                     glm::normalize(glm::vec4(normal, 0)),
                                     glm::vec4(0, 0, 0, 1));
                trans_mat = glm::translate(glm::mat4(), shapeIsect.point) * trans_mat;
                glm::vec4 temp_dir = trans_mat * glm::vec4(direction, 0);

                //change direction to correct space
                direction.x = temp_dir.x;
                direction.y = temp_dir.y;
                direction.z = temp_dir.z;


                Ray ray = shapeIsect.SpawnRay(direction);

                //throughput color
                Color3f beta = Color3f(1.0);
                int depth = recursionLimit;

                //compute initial photonColor
                Color3f photonColor = Le / float(photonsPerLight);

                while(depth > 0) {
                    Intersection isect;
                    if(scene->Intersect(ray, &isect)) {
                        // if there is no material break from loop
                        if(!isect.ProduceBSDF()) {
                            break;
                        }

                        Vector3f woW = -ray.direction;
                        // If not first iteration add current photon to photons
                        if(depth != recursionLimit) {
                            photons->push_back(Photon(isect.point, photonColor, woW));
                        }

                        xi = sampler->Get2D();
                        Vector3f wiW;
                        pdf = 0.f;
                        BxDFType type = BSDF_ALL;
                        BxDFType sampled_type;
                        //compute lambertian term
                        Color3f f = isect.bsdf.get()->Sample_f(woW, &wiW, xi, &pdf, type, &sampled_type);

                        // Update the throughput
                        if (pdf > 0.f) {
                            beta = beta * f * AbsDot(wiW, isect.normalGeometric) / pdf;
                        }

                        //Russian Roulette Ray Termination
                        float q = sampler->Get1D();
                        float max = glm::max(beta.x, glm::max(beta.y, beta.z));
                        if (max < q) break;

                        //update photonColor
                        photonColor *= beta;
                        //update ray
                        ray = isect.SpawnRay(wiW);
                    } else {
                        break;
                    }

                    depth--;
                }
            }
        }
    }
    else
    {
        Integrator::Render(); // Invokes Li for each ray from a pixel
    }
}

Color3f PhotonMapper::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //compute the direct illumination on the point of intersection just as
    //you would in a DirectLightingIntegrator
    Color3f direct = directIntegrator->Li(ray, scene, sampler, depth);
    Intersection isect;
    if (!scene.Intersect(ray, &isect)) return direct;
    if (!isect.ProduceBSDF()) return direct;

    //estimate the indirect illumination at that point by gathering all the photons in your defined search radius
    std::vector<Photon> photons = kdtree->particlesInSphere(isect.point, searchRadius);
    float area = M_PI * searchRadius * searchRadius;

    Color3f indirect(0.f);
    for(Photon photon: photons) {
        Point2f xi = sampler.get()->Get2D();
        Vector3f wiW;
        float pdf = 0.f;
        BxDFType sampled_type = BSDF_ALL;
        BxDFType type = BSDF_ALL;

        Color3f f = isect.bsdf->Sample_f(photon.wi, &wiW, xi, &pdf, type, &sampled_type);
        float dot = AbsDot(wiW, isect.normalGeometric);
        //sum the results of their individual light transport equation evaluations
        if(pdf > 0.f) {
            indirect += photon.color * f * dot / pdf;
        }
    }
    //divide the sum by the area of the space in which you searched for photons
    indirect /= area;
    return direct + indirect;
}
