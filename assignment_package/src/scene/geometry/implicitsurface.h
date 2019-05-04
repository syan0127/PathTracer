#pragma once

#include <scene/geometry/shape.h>

#define EPSILON 0.0001f
#define MAX_MARCHING_STEPS 1000
enum Surface
{
    TORUS,
    HEART,
    TANGLECUBE,
    APPLE,
    CHAIR
};

class ImplicitSurface : public Shape
{
public:
    virtual bool Intersect(const Ray &ray, Intersection *isect) const;
    virtual Point2f GetUVCoordinates(const Point3f &point) const;
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;

    virtual float Area() const;

    // Sample a point on the surface of the shape and return the PDF with
    // respect to area on the surface.
    virtual Intersection Sample(const Point2f &xi, Float *pdf) const;

    void create();

    float sceneSDF(Point3f p) const;
    Normal3f estimateNormal(Point3f p) const;
    float raymarch(float start, float end, Vector3f eye, Vector3f viewRayDirection) const;

    int surface;

};

