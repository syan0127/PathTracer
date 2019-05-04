#include "lambertbrdf.h"
#include <warpfunctions.h>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    return R * InvPi;
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereCosine(u);
//    *wi = WarpFunctions::squareToHemisphereUniform(u);
    if (wo.z < 0) {
        wi->z *= -1;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    return SameHemisphere(wo, wi) ?
//                WarpFunctions::squareToHemisphereUniformPDF(wi) : 0;
                WarpFunctions::squareToHemisphereCosinePDF(wi) : 0;

}

Color3f OrenNayarBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    float sinThetaI = SinTheta(wi);
    float sinThetaO = SinTheta(wo);

    float maxCos = 0.f;
    if (sinThetaI > 0.0001 && sinThetaO > 0.0001) {
        float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
        float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
        float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max(0.f, dCos);

    }

    float sinAlpha, tanBeta;
    if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / AbsCosTheta(wi);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / AbsCosTheta(wo);
    }

    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}

Color3f OrenNayarBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereCosine(u);
    if (wo.z < 0) {
        wi->z *= -1;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float OrenNayarBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    return SameHemisphere(wo, wi) ?
                WarpFunctions::squareToHemisphereCosinePDF(wi) : 0;

}
