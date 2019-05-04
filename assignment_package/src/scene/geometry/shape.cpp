#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t_in, float t_out, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t_in;
    isect->t_exit = t_out;
}

//This function must invoke the two-input Sample, to be implemented by subclasses,
//and generate a ωi from the resulting Intersection. From there, it must convert the PDF
//obtained from the other Sample function from a PDF with respect to surface area to a PDF
//with respect to solid angle at the reference point of intersection. As always, be careful
//of dividing by zero! Should this occur, you should set your PDF to zero.
Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    Intersection itsc = Sample(xi, pdf);
    //compute wi
    Vector3f wi = glm::normalize(itsc.point - ref.point);
    //find absolute dot product between the normal and wi
    float adot = AbsDot(itsc.normalGeometric, -wi) * Area();
    //if adot is zero set pdf to zero
    if (adot == 0.f) {
        *pdf = 0.f;
    } else {
        *pdf = glm::distance2(itsc.point, ref.point) / adot;
    }
    return itsc;
}
