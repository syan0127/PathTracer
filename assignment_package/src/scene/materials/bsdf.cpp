#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//TODO: Properly set worldToTangent and tangentToWorld
    : worldToTangent(),
      tangentToWorld(),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{
    UpdateTangentSpaceMatrices(normal, isect.tangent, isect.bitangent);
}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    //TODO: Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
    tangentToWorld = glm::mat3(t, b, n);
    worldToTangent = glm::inverse(tangentToWorld);
}

Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    //TODO
    Color3f color(0.f);

    //convert woW and wiW from world space to tangent space
    Vector3f woT = worldToTangent * woW;
    Vector3f wiT = worldToTangent * wiW;

    //compute the f() result of all of our BxDFs which match
    //the BxDFType indicated
    for (int i = 0; i < numBxDFs; i++) {
        if (bxdfs[i]->MatchesFlags(flags)) {
            color += bxdfs[i]->f(woT, wiT);
        }
    }

    return color;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    //TODO
    //select which BxDF to sample
    int matchingComps = BxDFsMatchingFlags(type);
    if (matchingComps == 0) {
        *pdf = 0;
        if (sampledType) {
            *sampledType = BxDFType(0);
            return Color3f(0.f);
        }
    }
    int comp = std::min((int) std::floor(xi[0] * matchingComps), matchingComps -1);

    //update the pointer of bxdf
    BxDF* bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < numBxDFs; ++i) {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    }
    //remap xi so that we don't bias the wi sample generated from BxDF::Sample_f
    Point2f xiRemapped(std::min(xi[0] * matchingComps - comp,
                                OneMinusEpsilon), xi[1]);

    //sample chosen bxdf
    Vector3f wi;
    Vector3f wo = worldToTangent * woW;
    *pdf = 0;

    if (sampledType) {
        *sampledType = bxdf->type;
    }
    Color3f color = bxdf->Sample_f(wo, &wi, xiRemapped, pdf, sampledType);
    if (*pdf == 0) {
        return Color3f(0.f);
    }
    *wiW = tangentToWorld * wi;
    if (wo.z == 0) return Color3f(0.f);

    //update pdf HW5
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        for (int i = 0; i < numBxDFs; i++) {
            if (bxdfs[i]->MatchesFlags(type) && bxdfs[i] != bxdf) {
                *pdf += bxdfs[i]->Pdf(wo, wi);
            }
        }
    }
    if (matchingComps > 1) {
        *pdf /= matchingComps;
    }

    //iterate over all BxDFs and sum together the results of their
    //f() for the chosen wo and wi, then return that sum.
    //update pdf HW5
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        return f(woW, *wiW, type);
    }
    return color;
}

float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    //TODO

    //transfrom wo and wi to tangent space
    Vector3f woT = worldToTangent * woW;
    Vector3f wiT = worldToTangent * wiW;

    float sum = 0.f;
    float num = 0.f;

    //iterate over bxdfs and update sum and num
    for (int i = 0; i < numBxDFs; i++) {
        if (bxdfs[i]->MatchesFlags(flags)) {
            sum += bxdfs[i]->Pdf(woT, wiT);
            num++;
        }
    }

    //if there is no bxdfs that matches return sum
    if (num < 1.f) return sum;
    return sum / num;
}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereCosine(xi);
    if (wo.z < 0) {
        wi->z *= -1;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}
