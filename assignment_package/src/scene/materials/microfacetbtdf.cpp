#include "microfacetbtdf.h"

Color3f MicrofacetBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    if (SameHemisphere(wo, wi)) {
        return Color3f(0.f);
    }
    //Compute cosThetaO and cosThetaI
    float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    //Handle degenerate cases for microfacet transmission
    if (cosThetaI == 0 || cosThetaO == 0) {
        return Color3f(0.f);
    }
    //Compute wh from wo and wi for microfacet transmission
    bool isEntering = CosTheta(wo) > 0;
    float eta = isEntering ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = glm::normalize(wi * eta + wo);

    if (wh.z < 0.f) {
        wh = -wh;
    }

    //compute F, D, G and denom to compute R
    Color3f F = fresnel->Evaluate(glm::dot(wo, wh));
    float D = distribution->D(wh);
    float G = distribution->G(wo, wi);
    float denom = glm::dot(wo,wh) + eta * glm::dot(wi,wh);
    return R * D * G * (Color3f(1.f) - F) * eta * eta
            * AbsDot(wi, wh) * AbsDot(wo, wh)
            / (cosThetaI * cosThetaO * denom * denom);
}

Color3f MicrofacetBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    if (wo.z == 0.f) {
        return Color3f(0.f);
    }
    //Compute wh from wo and wi for microfacet transmission
    Vector3f wh = distribution->Sample_wh(wo, xi);
    bool isEntering = CosTheta(wo) > 0;
    float eta = isEntering ? (etaA / etaB) : (etaB / etaA);

    if (!Refract(wo, wh, eta, wi)) {
        return Color3f(0.f);
    }
    //update pdf
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float MicrofacetBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    if (SameHemisphere(wo, wi)) return 0.f;
    //Compute wh from wo and wi for microfacet transmission
    bool isEntering = CosTheta(wo) > 0;
    float eta = isEntering ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = glm::normalize(wo + wi * eta);
    //Compute change of variables dwh_dwi for microfacet transmission
    float denom = glm::dot(wo, wh) + eta * glm::dot(wi, wh);
    float dwh_dwi = glm::abs(eta * eta * glm::dot(wi, wh) / (denom * denom));
    return distribution->Pdf(wo, wh) * dwh_dwi;
}
