// Copyright (c) 2015, PG, All rights reserved.
#include "Camera.h"

#include "ConVar.h"
#include "Engine.h"
#include "Matrices.h"
#include "Quaternion.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

struct Camera::CamImpl {
    struct CAM_PLANE {
        float a, b, c, d;
    };

    static float planeDotCoord(CAM_PLANE plane, vec3 point);
    static float planeDotCoord(vec3 planeNormal, vec3 planePoint, vec3 &pv);

    void setPos(vec3 pos);

    void updateVectors();
    void updateViewFrustum();

    void lookAt(vec3 eye, vec3 target);

    // vars
    CAMERA_TYPE camType{CAMERA_TYPE_FIRST_PERSON};
    vec3 vPos{0.f};
    vec3 vOrbitTarget{0.f};
    float fFov{103.f};
    float fOrbitDistance{5.f};
    bool bOrbitYAxis{true};

    // base axes
    vec3 vWorldXAxis{1.f, 0.f, 0.f};
    vec3 vWorldYAxis{0.f, 1.f, 0.f};
    vec3 vWorldZAxis{0.f, 0.f, 1.f};

    // derived axes
    vec3 vXAxis{1.f, 0.f, 0.f};
    vec3 vYAxis{0.f, 1.f, 0.f};
    vec3 vZAxis{0.f, 0.f, 1.f};

    // rotation
    Quaternion rotation;
    Quaternion worldRotation;
    float fPitch{0.f};
    float fYaw{0.f};
    float fRoll{0.f};

    // relative coordinate system
    vec3 vViewDir{0.f};
    vec3 vViewRight{1.f, 0.f, 0.f};
    vec3 vViewUp{0.f, 1.f, 0.f};

    // custom
    CAM_PLANE viewFrustum[4]{};
};

Matrix4 Camera::buildMatrixOrtho2D(float left, float right, float bottom, float top, float zn, float zf) {
    return buildMatrixOrtho2DGLLH(left, right, bottom, top, zn, zf);
}

Matrix4 Camera::buildMatrixOrtho2DGLLH(float left, float right, float bottom, float top, float zn, float zf) {
    return glm::ortho(left, right, bottom, top, zn, zf);
}

Matrix4 Camera::buildMatrixOrtho2DDXLH(float left, float right, float bottom, float top, float zn, float zf) {
    return glm::orthoLH_ZO(left, right, bottom, top, zn, zf);
}

Matrix4 Camera::buildMatrixLookAt(vec3 eye, vec3 target, vec3 up) { return glm::lookAt(eye, target, up); }

Matrix4 Camera::buildMatrixLookAtLH(vec3 eye, vec3 target, vec3 up) { return glm::lookAtLH(eye, target, up); }

Matrix4 Camera::buildMatrixPerspectiveFov(float fovRad, float aspect, float zn, float zf) {
    return buildMatrixPerspectiveFovVertical(fovRad, aspect, zn, zf);
}

Matrix4 Camera::buildMatrixPerspectiveFovVertical(float fovRad, float aspectRatioWidthToHeight, float zn, float zf) {
    return glm::perspective(fovRad, aspectRatioWidthToHeight, zn, zf);
}

Matrix4 Camera::buildMatrixPerspectiveFovVerticalDXLH(float fovRad, float aspectRatioWidthToHeight, float zn,
                                                      float zf) {
    return glm::perspectiveLH_ZO(fovRad, aspectRatioWidthToHeight, zn, zf);
}

Matrix4 Camera::buildMatrixPerspectiveFovHorizontal(float fovRad, float aspectRatioHeightToWidth, float zn, float zf) {
    float verticalFov = 2.0f * std::atan(std::tan(fovRad * 0.5f) * aspectRatioHeightToWidth);
    return buildMatrixPerspectiveFovVertical(verticalFov, 1.0f / aspectRatioHeightToWidth, zn, zf);
}

Matrix4 Camera::buildMatrixPerspectiveFovHorizontalDXLH(float fovRad, float aspectRatioHeightToWidth, float zn,
                                                        float zf) {
    float verticalFov = 2.0f * std::atan(std::tan(fovRad * 0.5f) * aspectRatioHeightToWidth);
    return buildMatrixPerspectiveFovVerticalDXLH(verticalFov, 1.0f / aspectRatioHeightToWidth, zn, zf);
}

static vec3 vec3TransformCoord(vec3 in, const Matrix4 &mat) {
    vec4 result = mat * vec4{in, 1.f};

    // perspective divide
    if(result.w != 0.0f) {
        result.x /= result.w;
        result.y /= result.w;
        result.z /= result.w;
    }

    return {result.x, result.y, result.z};
}

//*************************//
//	Camera implementation  //
//*************************//

Camera::Camera(vec3 pos, vec3 viewDir, float fovDeg, CAMERA_TYPE camType) : m_impl() {
    m_impl->vPos = pos;
    m_impl->vViewDir = viewDir;
    m_impl->fFov = vec::radians(fovDeg);
    m_impl->camType = camType;

    this->lookAt(pos + viewDir);
}

Camera::~Camera() = default;

Camera::Camera(const Camera &) = default;
Camera &Camera::operator=(const Camera &) = default;
Camera::Camera(Camera &&) = default;
Camera &Camera::operator=(Camera &&) = default;

void Camera::setFov(float fovDeg) { m_impl->fFov = vec::radians(fovDeg); }
void Camera::setFovRad(float fovRad) { m_impl->fFov = fovRad; }
void Camera::setOrbitYAxis(bool orbitYAxis) { m_impl->bOrbitYAxis = orbitYAxis; }

void Camera::setWorldOrientation(Quaternion worldRotation) {
    m_impl->worldRotation = worldRotation;
    m_impl->updateVectors();
}

// get
Camera::CAMERA_TYPE Camera::getType() const { return m_impl->camType; }
vec3 Camera::getPos() const { return m_impl->vPos; }

float Camera::getFov() const { return vec::degrees(m_impl->fFov); }
float Camera::getFovRad() const { return m_impl->fFov; }
float Camera::getOrbitDistance() const { return m_impl->fOrbitDistance; }

vec3 Camera::getWorldXAxis() const { return m_impl->worldRotation * m_impl->vXAxis; }
vec3 Camera::getWorldYAxis() const { return m_impl->worldRotation * m_impl->vYAxis; }
vec3 Camera::getWorldZAxis() const { return m_impl->worldRotation * m_impl->vZAxis; }

vec3 Camera::getViewDirection() const { return m_impl->vViewDir; }
vec3 Camera::getViewUp() const { return m_impl->vViewUp; }
vec3 Camera::getViewRight() const { return m_impl->vViewRight; }

float Camera::getPitch() const { return m_impl->fPitch; }
float Camera::getYaw() const { return m_impl->fYaw; }
float Camera::getRoll() const { return m_impl->fRoll; }

Quaternion Camera::getRotation() const { return m_impl->rotation; }

void Camera::CamImpl::updateVectors() {
    // update rotation
    if(this->camType == CAMERA_TYPE_FIRST_PERSON)
        this->rotation.fromEuler(this->fRoll, this->fYaw, -this->fPitch);
    else if(this->camType == CAMERA_TYPE_ORBIT) {
        this->rotation.identity();

        if(this->bOrbitYAxis) {
            // yaw
            Quaternion tempQuat;
            tempQuat.fromAxis(this->vYAxis, this->fYaw);

            this->rotation = tempQuat * this->rotation;

            // pitch
            tempQuat.fromAxis(this->vXAxis, -this->fPitch);
            this->rotation = this->rotation * tempQuat;

            this->rotation.normalize();
        }
    }

    // calculate new coordinate system
    this->vViewDir = (this->worldRotation * this->rotation) * this->vZAxis;
    this->vViewRight = (this->worldRotation * this->rotation) * this->vXAxis;
    this->vViewUp = (this->worldRotation * this->rotation) * this->vYAxis;

    // update pos if we are orbiting (with the new coordinate system from above)
    if(this->camType == CAMERA_TYPE_ORBIT) this->setPos(this->vOrbitTarget);

    this->updateViewFrustum();
}

// using view matrix from camera position
void Camera::CamImpl::updateViewFrustum() {
    Matrix4 viewMatrix = Camera::buildMatrixLookAt(this->vPos, this->vPos + this->vViewDir, this->vViewUp);
    Matrix4 projectionMatrix = Camera::buildMatrixPerspectiveFov(
        this->fFov, (float)engine->getScreenWidth() / (float)engine->getScreenHeight(), 0.1f, 100.0f);
    Matrix4 viewProj = projectionMatrix * viewMatrix;

    // extract frustum planes from view-projection matrix
    // left plane
    this->viewFrustum[0].a = viewProj[3] + viewProj[0];
    this->viewFrustum[0].b = viewProj[7] + viewProj[4];
    this->viewFrustum[0].c = viewProj[11] + viewProj[8];
    this->viewFrustum[0].d = viewProj[15] + viewProj[12];

    // right plane
    this->viewFrustum[1].a = viewProj[3] - viewProj[0];
    this->viewFrustum[1].b = viewProj[7] - viewProj[4];
    this->viewFrustum[1].c = viewProj[11] - viewProj[8];
    this->viewFrustum[1].d = viewProj[15] - viewProj[12];

    // top plane
    this->viewFrustum[2].a = viewProj[3] - viewProj[1];
    this->viewFrustum[2].b = viewProj[7] - viewProj[5];
    this->viewFrustum[2].c = viewProj[11] - viewProj[9];
    this->viewFrustum[2].d = viewProj[15] - viewProj[13];

    // bottom plane
    this->viewFrustum[3].a = viewProj[3] + viewProj[1];
    this->viewFrustum[3].b = viewProj[7] + viewProj[5];
    this->viewFrustum[3].c = viewProj[11] + viewProj[9];
    this->viewFrustum[3].d = viewProj[15] + viewProj[13];

    // normalize planes
    for(auto &i : this->viewFrustum) {
        vec3 normal(i.a, i.b, i.c);
        float length = vec::length(normal);

        if(length > 0.0f) {
            normal = vec::normalize(normal);
            i.a = normal.x;
            i.b = normal.y;
            i.c = normal.z;
            i.d = i.d / length;
        } else {
            i.a = 0.0f;
            i.b = 0.0f;
            i.c = 0.0f;
            i.d = 0.0f;
        }
    }
}

void Camera::rotateX(float pitchDeg) {
    m_impl->fPitch += pitchDeg;

    if(m_impl->fPitch > 89.0f)
        m_impl->fPitch = 89.0f;
    else if(m_impl->fPitch < -89.0f)
        m_impl->fPitch = -89.0f;

    m_impl->updateVectors();
}

void Camera::rotateY(float yawDeg) {
    m_impl->fYaw += yawDeg;

    if(m_impl->fYaw > 360.0f)
        m_impl->fYaw = m_impl->fYaw - 360.0f;
    else if(m_impl->fYaw < 0.0f)
        m_impl->fYaw = 360.0f + m_impl->fYaw;

    m_impl->updateVectors();
}

void Camera::lookAt(vec3 target) { m_impl->lookAt(m_impl->vPos, target); }

void Camera::CamImpl::lookAt(vec3 eye, vec3 target) {
    if(vec::length(eye - target) < 0.001f) return;

    this->vPos = eye;

    // https://stackoverflow.com/questions/1996957/conversion-euler-to-matrix-and-matrix-to-euler
    // https://gamedev.stackexchange.com/questions/50963/how-to-extract-euler-angles-from-transformation-matrix

    Matrix4 lookAtMatrix = Camera::buildMatrixLookAt(eye, target, this->vYAxis);

    // extract Euler angles from the matrix (NOTE: glm::extractEulerAngleYXZ works differently for some reason?)
    const float yaw = std::atan2(-lookAtMatrix[8], lookAtMatrix[0]);
    const float pitch = std::asin(-lookAtMatrix[6]);
    /// float roll = std::atan2(lookAtMatrix[4], lookAtMatrix[5]);

    this->fYaw = 180.0f + vec::degrees(yaw);
    this->fPitch = vec::degrees(pitch);

    this->updateVectors();
}

void Camera::setType(CAMERA_TYPE camType) {
    if(camType == m_impl->camType) return;

    m_impl->camType = camType;

    if(m_impl->camType == CAMERA_TYPE_ORBIT)
        m_impl->setPos(m_impl->vOrbitTarget);
    else
        m_impl->vPos = m_impl->vOrbitTarget;
}

void Camera::setPos(vec3 pos) { m_impl->setPos(pos); }

void Camera::CamImpl::setPos(vec3 pos) {
    this->vOrbitTarget = pos;

    if(this->camType == CAMERA_TYPE_ORBIT)
        this->vPos = this->vOrbitTarget + this->vViewDir * -this->fOrbitDistance;
    else if(this->camType == CAMERA_TYPE_FIRST_PERSON)
        this->vPos = pos;
}

void Camera::setOrbitDistance(float orbitDistance) {
    m_impl->fOrbitDistance = orbitDistance;
    if(m_impl->fOrbitDistance < 0) m_impl->fOrbitDistance = 0;
}

void Camera::setRotation(float yawDeg, float pitchDeg, float rollDeg) {
    m_impl->fYaw = yawDeg;
    m_impl->fPitch = pitchDeg;
    m_impl->fRoll = rollDeg;
    m_impl->updateVectors();
}

void Camera::setYaw(float yawDeg) {
    m_impl->fYaw = yawDeg;
    m_impl->updateVectors();
}

void Camera::setPitch(float pitchDeg) {
    m_impl->fPitch = pitchDeg;
    m_impl->updateVectors();
}

void Camera::setRoll(float rollDeg) {
    m_impl->fRoll = rollDeg;
    m_impl->updateVectors();
}

vec3 Camera::getNextPosition(vec3 velocity) const {
    return m_impl->vPos + ((m_impl->worldRotation * m_impl->rotation) * velocity);
}

vec3 Camera::getProjectedVector(vec3 point, float screenWidth, float screenHeight, float zn, float zf) const {
    Matrix4 viewMatrix = Camera::buildMatrixLookAt(m_impl->vPos, m_impl->vPos + m_impl->vViewDir, m_impl->vViewUp);
    Matrix4 projectionMatrix = Camera::buildMatrixPerspectiveFov(m_impl->fFov, screenWidth / screenHeight, zn, zf);

    // complete matrix
    Matrix4 viewProj = projectionMatrix * viewMatrix;

    // transform 3d point to 2d
    vec3 result = vec3TransformCoord(point, viewProj);

    // convert projected screen coordinates to real screen coordinates
    result.x = screenWidth * ((result.x + 1.0f) / 2.0f);
    result.y = screenHeight * ((1.0f - result.y) / 2.0f);  // flip Y for screen coordinates
    result.z = zn + result.z * (zf - zn);

    return result;
}

vec3 Camera::getUnProjectedVector(vec2 point, float screenWidth, float screenHeight, float zn, float zf) const {
    Matrix4 projectionMatrix = Camera::buildMatrixPerspectiveFov(m_impl->fFov, screenWidth / screenHeight, zn, zf);
    Matrix4 viewMatrix = Camera::buildMatrixLookAt(m_impl->vPos, m_impl->vPos + m_impl->vViewDir, m_impl->vViewUp);

    // transform pick position from screen space into 3d space
    vec4 viewport{0, 0, screenWidth, screenHeight};
    Matrix4 model{1.0f};  // identity model matrix

    // combine model and view matrices (required by glm::unProject)
    Matrix4 modelView = viewMatrix * model;

    return glm::unProject(vec3{point.x, screenHeight - point.y, 0.0f},  // flip Y for OpenGL
                          modelView, projectionMatrix, viewport);
}

bool Camera::isPointVisibleFrustum(vec3 point) const {
    const float epsilon = 0.01f;

    // left
    float d11 = CamImpl::planeDotCoord(m_impl->viewFrustum[0], point);

    // right
    float d21 = CamImpl::planeDotCoord(m_impl->viewFrustum[1], point);

    // top
    float d31 = CamImpl::planeDotCoord(m_impl->viewFrustum[2], point);

    // bottom
    float d41 = CamImpl::planeDotCoord(m_impl->viewFrustum[3], point);

    if((d11 < epsilon) || (d21 < epsilon) || (d31 < epsilon) || (d41 < epsilon)) return false;

    return true;
}

bool Camera::isPointVisiblePlane(vec3 point) const {
    constexpr float epsilon = 0.0f;  // ?

    if(!(CamImpl::planeDotCoord(m_impl->vViewDir, m_impl->vPos, point) < epsilon)) return true;

    return false;
}

float Camera::CamImpl::planeDotCoord(CAM_PLANE plane, vec3 point) {
    return ((plane.a * point.x) + (plane.b * point.y) + (plane.c * point.z) + plane.d);
}

float Camera::CamImpl::planeDotCoord(vec3 planeNormal, vec3 planePoint, vec3 &pv) {
    return vec::dot(planeNormal, pv - planePoint);
}
