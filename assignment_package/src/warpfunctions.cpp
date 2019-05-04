#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

glm::vec3 WarpFunctions::squareToDiskUniform(const glm::vec2 &sample)
{
    //(r,theta) = (sqrt(E1), 2pi(E2))
    float r = std::sqrt(sample.x);
    float theta = 2 * M_PI * sample.y;
    return glm::vec3(r * cos(theta), r * sin(theta), 0.f);
}

glm::vec3 WarpFunctions::squareToDiskConcentric(const glm::vec2 &sample)
{
    float phi, r, u, v;
    //are.X-1 // (a,b) is now on [-1,1]^2
    float a = 2 * sample.x - 1;
    float b = 2 * sample.y - 1;

    if (a > -b) { //region 1 or 2
        if (a > b) { //region 1; |a| > |b|
            r = a;
            phi = (M_PI / 4.f) * (b / a);
        } else { //region 2; |b| > |a|
            r = b;
            phi = (M_PI / 4.f) * (2.f - (a / b));
        }
    } else { //region 3 or 4
        if (a < b) { //region 3; |a| >= |b|, a != 0
            r = -a;
            phi = (M_PI / 4.f) * (4.f + (b / a));
        } else { //region 4; |b| >= |a|, but a == 0 and b == 0 could occur
            r = -b;
            if(b != 0) {
                phi = (M_PI / 4.f) * (6.f - (a / b));
            } else {
                phi = 0;
            }
        }
    }

    u = r * cos(phi);
    v = r * sin(phi);
    return glm::vec3(u, v, 0.f);
}

float WarpFunctions::squareToDiskPDF(const glm::vec3 &sample)
{
    // PDF of disc is 1/PI because area of disc is PI*r^2, and r = 1.
    return 1.f / M_PI;
}

glm::vec3 WarpFunctions::squareToSphereUniform(const glm::vec2 &sample)
{
    //x = cos(2pi(E2)) * 2 * sqrt(1-z^2)
    //y = sin(2pi(E2)) * 2 * sqrt(1-z^2)
    //z = 1-2(E1)
    float z = 1.f - 2.f * sample.x;
    float x = cos(2 * M_PI * sample.y) * sqrt(1.f - z * z);
    float y = sin(2 * M_PI * sample.y) * sqrt(1.f - z * z);

    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToSphereUniformPDF(const glm::vec3 &sample)
{
    // PDF of sphere is 1/(4PI) because area of sphere is 4*PI*r^2, and r = 1.
    return 1.f / (4 * M_PI);
}

glm::vec3 WarpFunctions::squareToSphereCapUniform(const glm::vec2 &sample, float thetaMin)
{
    //shrink z-coord's possible range based on thetaMin between 0 and 180 degrees.
    float z = 1.f - 2.f * sample.x * (180.f - thetaMin) / 180.f;
    float x = cos(2 * M_PI * sample.y) * sqrt(1.f - z * z);
    float y = sin(2 * M_PI * sample.y) * sqrt(1.f - z * z);

    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToSphereCapUniformPDF(const glm::vec3 &sample, float thetaMin)
{
    //surface area of spherical cap = 2*pi*r*(1 - cos(theta))
    float theta = ((180.f - thetaMin) / 180.f) * M_PI;
    float surface_area = 2.f * M_PI * (1.f - cos(theta));
    return 1.f / surface_area;
}

glm::vec3 WarpFunctions::squareToHemisphereUniform(const glm::vec2 &sample)
{
    //x = cos(2pi(E2)) * sqrt(1-(E1)^2)
    //y = sin(2pi(E2)) * sqrt(1-(E1)^2)
    //z = E1
    float z = sample.x;
    float x = cos(2 * M_PI * sample.y) * sqrt(1.f - z * z);
    float y = sin(2 * M_PI * sample.y) * sqrt(1.f - z * z);
    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const glm::vec3 &sample)
{
    // PDF of hemisphere is 1/(2PI) because area of disc is 2*PI*r^2, and r = 1.
    return 1.f / (2 * M_PI);
}

glm::vec3 WarpFunctions::squareToHemisphereCosine(const glm::vec2 &sample)
{
    //x and y same as disk
    //z = sqrt(1 - x^2 - y^2)
    glm::vec3 disk = squareToDiskConcentric(sample);
    disk.z = sqrt(1.f - disk.x * disk.x - disk.y * disk.y);
    return disk;
}

float WarpFunctions::squareToHemisphereCosinePDF(const glm::vec3 &sample)
{
    return sample.z / M_PI;
//    return 1.f / (M_PI);
//    return cos(acos(sample.z)) / M_PI;
}
