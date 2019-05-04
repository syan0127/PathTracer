#include "kdtree.h"

KDNode::KDNode()
    : leftChild(nullptr), rightChild(nullptr), axis(0), minCorner(), maxCorner(), particles()
{}

KDNode::~KDNode()
{
    delete leftChild;
    delete rightChild;
}

KDTree::KDTree()
    : root(nullptr)
{}

KDTree::~KDTree()
{
    delete root;
}

// Comparator functions you can use with std::sort to sort vec3s along the cardinal axes
bool xSort(Photon a, Photon b) { return a.pos.x < b.pos.x; }
bool ySort(Photon a, Photon b) { return a.pos.y < b.pos.y; }
bool zSort(Photon a, Photon b) { return a.pos.z < b.pos.z; }

void KDTree::build(const std::vector<Photon> *points)
{
    //TODO
    minCorner = getMinCorner(*points);
    maxCorner = getMaxCorner(*points);
    root = buildRecursive(*points, 0);
}

glm::vec3 KDTree::getMinCorner(const std::vector<Photon> &points)
{
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();

    int size = points.size();
    //iterate over points and find minimum x, y, and z
    for (int i = 0; i < size; i++) {
        Point3f p = points.at(i).pos;
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        minZ = std::min(minZ, p.z);
    }

    return glm::vec3(minX, minY, minZ);
}

glm::vec3 KDTree::getMaxCorner(const std::vector<Photon> &points)
{
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();
    float maxZ = std::numeric_limits<float>::min();

    int size = points.size();
    //iterate over points and find minimum x, y, and z
    for (int i = 0; i < size; i++) {
        Point3f p = points.at(i).pos;
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
        maxZ = std::max(maxZ, p.z);
    }

    return glm::vec3(maxX, maxY, maxZ);
}

KDNode* KDTree::buildRecursive(const std::vector<Photon> &points, int level)
{
    unsigned int axis = level % 3;
    int size = points.size();
    KDNode* node = new KDNode();
    node->axis = axis;

    //base base - if size of points is either 0 or 1 we stop the recursion
    if (size == 1) {
        node->minCorner = points[0].pos;
        node->maxCorner = points[0].pos;
        node->particles.push_back(points[0]);
        return node;
    }


    //make a copy of points
    std::vector<Photon> sortedPts = points;

    //sort the points based on the axis we are currently on
    switch (axis) {
        case 0: // x axis
            std::sort(sortedPts.begin(), sortedPts.end(), xSort);
            break;
        case 1: // y axis
            std::sort(sortedPts.begin(), sortedPts.end(), ySort);
            break;
        case 2: // z axis
            std::sort(sortedPts.begin(), sortedPts.end(), zSort);
            break;
        default:
            break;
    }

    //create two vectors to store left points and right points
    std::vector<Photon> leftPts, rightPts;
    int m = size / 2;
    for (int i = 0; i < m; i++) {
        leftPts.push_back(sortedPts.at(i));
    }

    for (int i = m; i < size; i++) {
        rightPts.push_back(sortedPts.at(i));
    }

    node->minCorner = getMinCorner(points);
    node->maxCorner = getMaxCorner(points);

    //recursion on left node
    node->leftChild = buildRecursive(leftPts, level + 1);
    //recursion on right node
    node->rightChild = buildRecursive(rightPts, level + 1);

    return node;
}

std::vector<Photon> KDTree::particlesInSphere(glm::vec3 c, float r)
{
    std::vector<Photon> res;
    //traverse tree and find nodes that lie inside the sphere
    traverse(&res, this->root, c, r);
    return res;
}

void KDTree::traverse(std::vector<Photon> *res, KDNode* node, glm::vec3 c, float r)
{
    //return if node is null
    if (node == nullptr) return;
    //check if the node is a leaf node
    if (node->leftChild == nullptr && node->rightChild == nullptr) {
        Photon p = node->particles[0];
        //check if the point lies inside the sphere
        if(glm::distance(p.pos, c) <= r) {
            res->push_back(p);
        }
    } else {
        //check the boundaries
        glm::vec3 min = c - glm::vec3(r);
        glm::vec3 max = c + glm::vec3(r);
        if (node->maxCorner.x >= min.x && node->minCorner.x <= max.x &&
                node->maxCorner.y >= min.y && node->minCorner.y <= max.y &&
                node->maxCorner.z >= min.z && node->minCorner.z <= max.z) {
            //recurse over left subtree
            traverse(res, node->leftChild, c, r);
            //recurse over right subtree
            traverse(res, node->rightChild, c, r);
        }
    }
}

void KDTree::clear()
{
    delete root;
    root = nullptr;
}
