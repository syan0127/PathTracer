#include "implicitsurface.h"
#include <warpfunctions.h>

float ImplicitSurface::Area() const
{
    //dont really need it; copy pasted from other shape
    return 0;
}


bool ImplicitSurface::Intersect(const Ray &ray, Intersection *isect) const
{
    //Sphere-marching: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields
    Ray r_loc = ray.GetTransformedCopy(transform.invT());
    float stepSize = 0.1;

    float t = 0;
    Vector3f leftPoint = r_loc.origin + t * r_loc.direction;
    bool currentSign = std::signbit(sceneSDF(leftPoint)); // return true if x < 0
    bool intersected = false;
    float phi = 0.005;
    if(surface == HEART) phi = 0.000125f;
    for(int i = 0; i < MAX_MARCHING_STEPS; i++) {
        t += stepSize;
        Vector3f rightPoint = r_loc.origin + t * r_loc.direction;
        // if the sign changed, then clearly we moved from one side of the surface to the other,
        // i.e. the segment straddles the surface

        float sdf = sceneSDF(rightPoint);
        if ((!currentSign && sdf < phi) || (currentSign && sdf > phi)) {
            // we have an intersection
            if(!intersected) {
                intersected = true;
                t -= stepSize;
                stepSize /= 100.0;
                i -= 100;
            } else {
                if(currentSign) {
                    t+= 0.005;
                } else {
                    t-= 0.005;
                }
                rightPoint = r_loc.origin + t * r_loc.direction;
                InitializeIntersection(isect, t, INFINITY, rightPoint);
                return true;
            }
        }
    }
    return false;
}

float ImplicitSurface::raymarch(float start, float end, Vector3f eye, Vector3f viewRayDirection) const
{
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sceneSDF(eye + depth * viewRayDirection);
        if (dist < 0.00001) {
            // We're inside the scene surface!
            return depth;
        }
        // Move along the view ray
        depth += dist;

        if (depth >= end) {
            // Gone too far; give up
            return end;
        }
    }
    return end;
}

void ImplicitSurface::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    Normal3f nor_local = estimateNormal(P);
    *nor = transform.invTransT() * nor_local;
    *tan = glm::normalize(transform.T3() * glm::cross(Vector3f(0,1,0), nor_local));
    *bit = glm::normalize(glm::cross(*nor, *tan));
}


float ImplicitSurface::sceneSDF(Point3f p) const
{
    float x = p.x, y = p.y, z = p.z, x2 = x * x, y2 = y * y, z2 = z * z;

    if(surface == TORUS) {
        Vector2f t = Vector2f(0.8, 0.3);
        Vector2f q = Vector2f(glm::length(Vector2f(p.x, p.z)) - t.x, p.y);
        return glm::length(q) - t.y;
    } else if(surface == HEART) {
        //heart equation #1
//        float c = x2 + (9.f / 4.f) * y2 + z2 - 1;
//        return c * c * c - x2 * z2 * z - (9.f / 80.f) * y2 * z2 * z;

        //heart equation #2
        float k = 2 * x2 + y2 + z2 -1;
        return k * k * k - 0.1 * x2 * z2 * z - y2 * z2 * z;
    } else if(surface == TANGLECUBE) {
        return x2 * x2 - 5 * x2 + y2 * y2 - 5 * y2 + z2 * z2 - 5 * z2 + 11.8;
    } else if(surface == APPLE) {
        float r = 0.5, R = 1.0, r2 = r * r, r4 = r2 * r2, R2 = R * R;
        return r4 - 2 * r2 * (R2 + x2 + y2 - z2) + (-R2 + x2 + y2 + z2) * (-R2 + x2 + y2 + z2);
    } else if(surface == CHAIR) {
        float k = 5, a = 0.95, b = 0.8;
        return (x2 + y2 + z2 - a * k * k) * (x2 + y2 + z2 - a * k * k)
                - b * ((z - k) * (z - k) - 2 * x2) * ((z + k) * (z + k) - 2 * y2);
    }
}

Normal3f ImplicitSurface::estimateNormal(Point3f p) const
{
    //this function estimates normal at the given point p
    return glm::normalize(Vector3f(
                                sceneSDF(Point3f(p.x + EPSILON, p.y, p.z)) - sceneSDF(Point3f(p.x - EPSILON, p.y, p.z)),
                                sceneSDF(Point3f(p.x, p.y + EPSILON, p.z)) - sceneSDF(Point3f(p.x, p.y - EPSILON, p.z)),
                                sceneSDF(Point3f(p.x, p.y, p.z  + EPSILON)) - sceneSDF(Point3f(p.x, p.y, p.z - EPSILON))));
}

Point2f ImplicitSurface::GetUVCoordinates(const Point3f &point) const
{
    //dont really need it; copy pasted from other shape
    Point3f p = glm::normalize(point);
    float phi = atan2f(p.z, p.x);
    if(phi < 0)
    {
        phi += TwoPi;
    }
    float theta = glm::acos(p.y);
    return Point2f(1 - phi/TwoPi, 1 - theta / Pi);
}

Intersection ImplicitSurface::Sample(const Point2f &xi, Float *pdf) const
{
    //dont really need it; copy pasted from other shape
    Point3f pObj = WarpFunctions::squareToSphereUniform(xi);

    Intersection it;
    it.normalGeometric = glm::normalize( transform.invTransT() *pObj );
    it.point = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));

    *pdf = 1.0f / Area();

    return it;
}
