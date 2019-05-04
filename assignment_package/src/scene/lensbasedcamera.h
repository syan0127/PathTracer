#pragma once
#include "camera.h"


class LensBasedCamera : public Camera
{
public:
    float lensRadius, focalDistance;
    LensBasedCamera();
    LensBasedCamera(float lensr, float focald);
    LensBasedCamera(float lensr, float focald, unsigned int w, unsigned int h);
    LensBasedCamera(float lensr, float focald, unsigned int w, unsigned int h, const Vector3f &e, const Vector3f &r, const Vector3f &worldUp);

};

