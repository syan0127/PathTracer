#pragma once

#include "light.h"

class SpotLight : public Light
{
public:
    SpotLight(const Transform &t,
              const Color3f &Le,
              float totalWidth,
              float falloffStart)
        : Light(t),
          pLight(t.position()),
          emittedLight(Le),
          cosTotalWidth(std::cos(glm::radians(totalWidth))),
          cosFalloffStart(std::cos(glm::radians(falloffStart))) {}

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                       Vector3f *wi, Float *pdf) const;


    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;
    float Falloff(const Vector3f &w) const;

    const Point3f pLight;
    const Color3f emittedLight;
    const float cosTotalWidth, cosFalloffStart;
};

