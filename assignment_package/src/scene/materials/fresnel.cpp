#include "fresnel.h"

//computes how reflective a surface point on a dielectric
//material should be given its indices of refraction and the angle
//between the incident ray and the surface normal
Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    cosThetaI = glm::clamp(cosThetaI, -1.f, 1.f);
    //check whether your ray is entering or leaving the object
    //with which it has intersected
    bool isEntering = cosThetaI > 0.f;

    //compute index of refraction
    float n1 = isEntering ? etaI : etaT;
    float n2 = isEntering ? etaT : etaI;
    cosThetaI = isEntering ? cosThetaI : std::abs(cosThetaI);

    float sinThetaI = std::sqrt(std::max(0.f, 1 - cosThetaI * cosThetaI));
    float sinThetaT = n1 / n2 * sinThetaI;
    if (sinThetaT >= 1) {
        return Color3f(1.0f);
    }

    float cosThetaT = std::sqrt(std::max(0.f, 1 - sinThetaT * sinThetaT));
    float rperp = ((n2 * cosThetaI) - (n1 * cosThetaT)) /
                  ((n2 * cosThetaI) + (n1 * cosThetaT));
    float rparl = ((n1 * cosThetaI) - (n2 * cosThetaT)) /
                  ((n1 * cosThetaI) + (n2 * cosThetaT));

    return Color3f((rparl * rparl + rperp * rperp) / 2.f);
}
