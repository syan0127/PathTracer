#include "camera.h"
#include "warpfunctions.h"
#include <la.h>
#include <iostream>


Camera::Camera():
    Camera(400, 400)
{
    look = Vector3f(0,0,-1);
    up = Vector3f(0,1,0);
    right = Vector3f(1,0,0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, Vector3f(0,0,10), Vector3f(0,0,0), Vector3f(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const Vector3f &e, const Vector3f &r, const Vector3f &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    lensRadius(0.5f),
    focalDistance(30),
    eye(e),
    ref(r),
    world_up(worldUp),
    sampler(new Sampler(100,0))
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    lensRadius(c.lensRadius),
    focalDistance(c.focalDistance),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H),
    sampler(c.sampler)
{}

void Camera::CopyAttributes(const Camera &c)
{
    fovy = c.fovy;
    near_clip = c.near_clip;
    far_clip = c.far_clip;
    lensRadius = c.lensRadius;
    focalDistance = c.focalDistance;
    eye = c.eye;
    ref = c.ref;
    look = c.look;
    up = c.up;
    right = c.right;
    width = c.width;
    height = c.height;
    aspect = c.aspect;
    V = c.V;
    H = c.H;
    sampler = c.sampler;
}

void Camera::RecomputeAttributes()
{
    look = glm::normalize(ref - eye);
    right = glm::normalize(glm::cross(look, world_up));
    up = glm::cross(right, look);

    float tan_fovy = tan(glm::radians(fovy/2));
    float len = glm::length(ref - eye);
    aspect = width/(float)height;
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
}

glm::mat4 Camera::GetViewProj() const
{
    return glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

void Camera::RotateAboutUp(float deg)
{
    deg = glm::radians(deg);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), deg, up);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}
void Camera::RotateAboutRight(float deg)
{
    deg = glm::radians(deg);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), deg, right);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}

void Camera::TranslateAlongLook(float amt)
{
    glm::vec3 translation = look * amt;
    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
    eye += translation;
    ref += translation;
}
void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
    eye += translation;
    ref += translation;
}

Ray Camera::Raycast(const Point2f &pt) const
{
    return Raycast(pt.x, pt.y);
}

Ray Camera::Raycast(float x, float y) const
{
    float ndc_x = (2.f*x/width - 1);
    float ndc_y = (1 - 2.f*y/height);
//    return RaycastNDC(ndc_x, ndc_y);     //for normal camera
    return RayCastLensBased(ndc_x, ndc_y); //for lens based camera
}

Ray Camera::RaycastNDC(float ndc_x, float ndc_y) const
{
    glm::vec3 P = ref + ndc_x*H + ndc_y*V;
    Ray result(eye, glm::normalize(P - eye));
    return result;
}

Ray Camera::RayCastLensBased(float ndc_x, float ndc_y) const
{
    glm::vec3 P = ref + ndc_x*H + ndc_y*V;
    Ray result(Point3f(0.0), glm::normalize(P - eye));
    //Modify ray for depth of field
    if(lensRadius > 0.f) {
        //Sample point on lens
        Point3f pLens = lensRadius * WarpFunctions::squareToDiskConcentric(sampler->Get2D());

        //Compute point on plane of focus
        float ft = focalDistance / result.direction.z;
        Point3f pFocus = result.origin + result.direction * ft;

        //Update ray for effect of lens
        result.origin = pLens;
        result.direction = glm::normalize(pFocus - result.origin);
    }
    result.origin += eye;
    return result;
}

void Camera::create()
{
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> col;
    std::vector<GLuint> idx;

    //0: Eye position
    pos.push_back(eye);
    //1 - 4: Near clip
        //Lower-left
        Ray r = this->RaycastNDC(-1,-1);
        pos.push_back(eye + r.direction * near_clip);
        //Lower-right
        r = this->RaycastNDC(1,-1);
        pos.push_back(eye + r.direction * near_clip);
        //Upper-right
        r = this->RaycastNDC(1,1);
        pos.push_back(eye + r.direction * near_clip);
        //Upper-left
        r = this->RaycastNDC(-1,1);
        pos.push_back(eye + r.direction * near_clip);
    //5 - 8: Far clip
        //Lower-left
        r = this->RaycastNDC(-1,-1);
        pos.push_back(eye + r.direction * far_clip);
        //Lower-right
        r = this->RaycastNDC(1,-1);
        pos.push_back(eye + r.direction * far_clip);
        //Upper-right
        r = this->RaycastNDC(1,1);
        pos.push_back(eye + r.direction * far_clip);
        //Upper-left
        r = this->RaycastNDC(-1,1);
        pos.push_back(eye + r.direction * far_clip);

    for(int i = 0; i < 9; i++){
        col.push_back(glm::vec3(1,1,1));
    }

    //Frustum lines
    idx.push_back(1);idx.push_back(5);
    idx.push_back(2);idx.push_back(6);
    idx.push_back(3);idx.push_back(7);
    idx.push_back(4);idx.push_back(8);
    //Near clip
    idx.push_back(1);idx.push_back(2);
    idx.push_back(2);idx.push_back(3);
    idx.push_back(3);idx.push_back(4);
    idx.push_back(4);idx.push_back(1);
    //Far clip
    idx.push_back(5);idx.push_back(6);
    idx.push_back(6);idx.push_back(7);
    idx.push_back(7);idx.push_back(8);
    idx.push_back(8);idx.push_back(5);

    //Camera axis
    pos.push_back(eye); col.push_back(glm::vec3(0,0,1)); idx.push_back(9);
    pos.push_back(eye + look); col.push_back(glm::vec3(0,0,1));idx.push_back(10);
    pos.push_back(eye); col.push_back(glm::vec3(1,0,0));idx.push_back(11);
    pos.push_back(eye + right); col.push_back(glm::vec3(1,0,0));idx.push_back(12);
    pos.push_back(eye); col.push_back(glm::vec3(0,1,0));idx.push_back(13);
    pos.push_back(eye + up); col.push_back(glm::vec3(0,1,0));idx.push_back(14);

    count = idx.size();

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(idx.data(), count * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(pos.data(), pos.size() * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(col.data(), col.size() * sizeof(glm::vec3));
}

GLenum Camera::drawMode() const {return GL_LINES;}
