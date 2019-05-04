#include "csg.h"

CSGNode::CSGNode()
    :leftChild(nullptr), rightChild(nullptr), oper(SHAPE), primitive(NULL)
{}

CSGNode::CSGNode(Operator oper) :leftChild(nullptr), rightChild(nullptr), oper(oper), primitive(NULL)
{}

CSGNode::~CSGNode()
{
    delete leftChild;
    delete rightChild;
}

CSG::CSG(std::vector<std::shared_ptr<Primitive>> primitives, std::vector<Operator> operators)
    : primitives(primitives), operators(operators), name(), oper_idx(0), shape_idx(0)
{
    root = buildRecursive();
}

CSG::~CSG()
{
    delete root;
}

CSGNode* CSG::buildRecursive()
{
    //get the current operator using the index
    Operator curr_oper = operators[oper_idx++];
    //create new node with the curr_oper
    CSGNode* node = new CSGNode(curr_oper);

    //check if current operater is shape
    if(curr_oper == SHAPE) {
        node->idx = shape_idx++; //update the idx that indicates the index of the node's shape
    } else {
        //recursively build left and right subtrees
        node->leftChild = buildRecursive();
        node->rightChild = buildRecursive();
    }
    //return root
    return node;
}

bool CSG::Intersect(const Ray &ray, Intersection *isect) const
{
    //call recursive intersect
    return intersectRecursive(ray, isect, root);
}

bool CSG::intersectRecursive(const Ray &ray, Intersection *isect, CSGNode* node) const
{
    Operator oper = node->oper;

    //reaching here means that we are on the leaf node
    if(oper == SHAPE) {
        bool isIntersecting = primitives[node->idx]->Intersect(ray, isect);
        return isIntersecting;
    }

    //create two intersections to store data for left and right childs
    Intersection isectLeft = Intersection();
    Intersection isectRight = Intersection();

    //recursive call; preorder: root=>left=>right
    bool left = intersectRecursive(ray, &isectLeft, node->leftChild);
    bool right = intersectRecursive(ray, &isectRight, node->rightChild);
    bool res = false;

    if(oper == UNION) {
        res = left || right;
        if (res) {
            if (left && right) {
                *isect = (isectLeft.t < isectRight.t) ? isectLeft : isectRight;
            } else {
                *isect = left ? isectLeft : isectRight;
            }
        }
    } else if(oper == INTER) {
        res = left && right;
        if (res) { //left = A right = B
            if (isectLeft.t < isectRight.t && isectRight.t_exit < isectLeft.t_exit) {
                //case +A +B -B -A => normal = B color = A
                //[Rin                  Rout]
                //      [Lin    Lout]
                isectRight.objectHit = isectLeft.objectHit;
                *isect = isectRight;
            } else if (isectLeft.t > isectRight.t && isectRight.t_exit > isectLeft.t_exit) {
                //case +B +A -A -B => normal = A color = A
                //[Lin                  Lout]
                //      [Rin    Rout]
                *isect = isectLeft;
            } else if (isectLeft.t < isectRight.t && isectRight.t < isectLeft.t_exit) {
                //case +A +B -A -B => normal = B color = A
                //[Lin       Lout]
                //      [Rin        Rout]
                isectRight.objectHit = isectLeft.objectHit;
                *isect = isectRight;
            } else if (isectLeft.t > isectRight.t && isectRight.t_exit > isectLeft.t) {
                //case +B +A -B -A => normal = A color = A
                //[Rin       Rout]
                //      [Lin        Lout]
                *isect = isectLeft;
            } else {
                res = false;
            }
        }
    } else if(oper == DIFF) {
        res = left && right;
        if (res) {
            if (isectLeft.t < isectRight.t || isectRight.t_exit < isectLeft.t) {
                //case +A -A +B -B
                //case +A +B -A -B
                //case +A +B -B -A
                //case +B -B +A -A
                *isect = isectLeft;
            } else if(isectLeft.t_exit > isectRight.t_exit) {
                //case +B -B +A -A
                isectRight.objectHit = isectLeft.objectHit;
                *isect = isectRight;
            } else {
                //case +B +A -A -B
                res = false;
            }
        }
        else if (left) {
            //no intersection b/w the objects
            *isect = isectLeft;
            res = true;
        }
    }
    return res;
}
