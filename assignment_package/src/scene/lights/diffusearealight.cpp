#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO
    //if twosided, return emittedLight
    if (twoSided) return emittedLight;
    //otherwise, check if w and normal are same direction
    return (glm::dot(w, isect.normalGeometric) > 0.f) ? emittedLight
                                                      : Color3f(0.f);

}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
//    Get an Intersection on the surface of its Shape by invoking shape->Sample.
//    Check if the resultant PDF is zero or that the reference Intersection and the resultant
//    Intersection are the same point in space, and return black if this is the case.
//    Set ωi to the normalized vector from the reference Intersection's point to the Shape's intersection point.
//    Return the light emitted along ωi from our intersection point.

    Intersection itsc = shape->Sample(ref, xi, pdf);
    if (*pdf == 0.f || ref.point == itsc.point) {
        *pdf = 0.f;
        return Color3f(0.f);
    }

    *wi = glm::normalize(itsc.point - ref.point);
    return L(itsc, -*wi);
}

//returns probability density with respect to the solid angle for light's
//Sample_Li to sample the direction wi from the ref point
float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    Intersection isect = Intersection();
    Ray ray = ref.SpawnRay(wi);
    //Intersect sample ray with area light geometry
    if (!shape->Intersect(ray, &isect)) {
        return 0.f;
    }
    if (AbsDot(isect.normalGeometric, -glm::normalize(wi)) * area == 0) {
        return 0.f;
    }
    //Convert light sample weight to solid angle measure
    float pdf = glm::distance2(ref.point, isect.point) /
            (AbsDot(isect.normalGeometric, -glm::normalize(wi) * area));

    //check if pdf is infinite
    if (std::isinf(pdf))
    {
        pdf = 0.f;
    }
    return pdf;
}
