#include "specularbTdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //check whether your ray is entering or leaving the object with
    //which it has intersected
    bool isEntering = CosTheta(wo) > 0;
    float n1 = isEntering ? etaA : etaB;
    float n2 = isEntering ? etaB : etaA;

    //check for total internal reflection and return black if it would occur
    if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), n1 / n2, wi)) {
        return Color3f(0.f);
    }
    *pdf = 1;
    Color3f color = T * (Color3f(1.f) - fresnel->Evaluate(CosTheta(*wi)));

    return color / AbsCosTheta(*wi);
}
