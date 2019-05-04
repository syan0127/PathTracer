#include "lensbasedcamera.h"

LensBasedCamera::LensBasedCamera() : Camera() {
    lensRadius = 0.f;
    focalDistance = 0.f;
}

LensBasedCamera::LensBasedCamera(float lensr, float focald) :
    lensRadius(lensr),
    focalDistance(focald),
    Camera()
{}

LensBasedCamera::LensBasedCamera(float lensr, float focald, unsigned int w, unsigned int h) :
    lensRadius(lensr),
    focalDistance(focald),
    Camera(w, h)
{}

LensBasedCamera::LensBasedCamera(float lensr, float focald, unsigned int w, unsigned int h, const Vector3f &e, const Vector3f &r, const Vector3f &worldUp) :
    lensRadius(lensr),
    focalDistance(focald),
    Camera(w, h, e, r, worldUp)
{}
