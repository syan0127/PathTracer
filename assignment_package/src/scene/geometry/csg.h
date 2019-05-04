#pragma once
#include "primitive.h"

enum Operator
{
    SHAPE,
    UNION,
    INTER,
    DIFF
};

class CSGNode
{
public:
    CSGNode();
    CSGNode(Operator oper);
    ~CSGNode();

    CSGNode* leftChild;
    CSGNode* rightChild;
    Operator oper;
    Primitive primitive;
    int idx;
};

class CSG : public Primitive
{
public:
    CSG(std::vector<std::shared_ptr<Primitive>> primitives, std::vector<Operator> operators);
    ~CSG();
    CSGNode* buildRecursive();
    virtual bool Intersect(const Ray &ray, Intersection *isect) const;
    bool intersectRecursive(const Ray &ray, Intersection *isect, CSGNode* node) const;

    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<Operator> operators;

    CSGNode* root;
    int shape_idx;
    int oper_idx;
    QString name;
};


