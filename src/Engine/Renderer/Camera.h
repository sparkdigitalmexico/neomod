#pragma once
// Copyright (c) 2015, PG, All rights reserved.
#ifndef CAMERA_H
#define CAMERA_H

#include "StaticPImpl.h"

#include "Vectors_fwd.h"
class Quaternion;
struct Matrix4;

class Camera {
   public:
    static Matrix4 buildMatrixOrtho2D(float left, float right, float bottom, float top, float zn,
                                      float zf);  // DEPRECATED (OpenGL specific)
    static Matrix4 buildMatrixOrtho2DGLLH(float left, float right, float bottom, float top, float zn,
                                          float zf);  // OpenGL
    static Matrix4 buildMatrixOrtho2DDXLH(float left, float right, float bottom, float top, float zn,
                                          float zf);                   // DirectX
    static Matrix4 buildMatrixLookAt(vec3 eye, vec3 target, vec3 up);  // DEPRECATED
    static Matrix4 buildMatrixLookAtLH(vec3 eye, vec3 target, vec3 up);
    static Matrix4 buildMatrixPerspectiveFov(float fovRad, float aspect, float zn,
                                             float zf);  // DEPRECATED (OpenGL specific)
    static Matrix4 buildMatrixPerspectiveFovVertical(float fovRad, float aspectRatioWidthToHeight, float zn,
                                                     float zf);  // DEPRECATED
    static Matrix4 buildMatrixPerspectiveFovVerticalDXLH(float fovRad, float aspectRatioWidthToHeight, float zn,
                                                         float zf);
    static Matrix4 buildMatrixPerspectiveFovHorizontal(float fovRad, float aspectRatioHeightToWidth, float zn,
                                                       float zf);  // DEPRECATED
    static Matrix4 buildMatrixPerspectiveFovHorizontalDXLH(float fovRad, float aspectRatioHeightToWidth, float zn,
                                                           float zf);

   public:
    enum CAMERA_TYPE : uint8_t { CAMERA_TYPE_FIRST_PERSON, CAMERA_TYPE_ORBIT };

    Camera(vec3 pos /* = vec3(0, 0, 0) */, vec3 viewDir /* = vec3(0, 0, 1) */, float fovDeg = 90.0f,
           CAMERA_TYPE camType = CAMERA_TYPE_FIRST_PERSON);

    void rotateX(float pitchDeg);
    void rotateY(float yawDeg);

    void lookAt(vec3 target);

    // set
    void setType(CAMERA_TYPE camType);
    void setPos(vec3 pos);
    void setFov(float fovDeg);
    void setFovRad(float fovRad);
    void setOrbitDistance(float orbitDistance);
    void setOrbitYAxis(bool orbitYAxis);

    void setRotation(float yawDeg, float pitchDeg, float rollDeg);
    void setYaw(float yawDeg);
    void setPitch(float pitchDeg);
    void setRoll(float rollDeg);
    void setWorldOrientation(Quaternion worldRotation);

    // get
    [[nodiscard]] CAMERA_TYPE getType() const;
    [[nodiscard]] vec3 getPos() const;
    [[nodiscard]] vec3 getNextPosition(vec3 velocity) const;

    [[nodiscard]] float getFov() const;
    [[nodiscard]] float getFovRad() const;
    [[nodiscard]] float getOrbitDistance() const;

    [[nodiscard]] vec3 getWorldXAxis() const;
    [[nodiscard]] vec3 getWorldYAxis() const;
    [[nodiscard]] vec3 getWorldZAxis() const;

    [[nodiscard]] vec3 getViewDirection() const;
    [[nodiscard]] vec3 getViewUp() const;
    [[nodiscard]] vec3 getViewRight() const;

    [[nodiscard]] float getPitch() const;
    [[nodiscard]] float getYaw() const;
    [[nodiscard]] float getRoll() const;

    [[nodiscard]] Quaternion getRotation() const;

    [[nodiscard]] vec3 getProjectedVector(vec3 point, float screenWidth, float screenHeight, float zn = 0.1f,
                                          float zf = 1.0f) const;
    [[nodiscard]] vec3 getUnProjectedVector(vec2 point, float screenWidth, float screenHeight, float zn = 0.1f,
                                            float zf = 1.0f) const;

    [[nodiscard]] bool isPointVisibleFrustum(vec3 point) const;  // within our viewing frustum
    [[nodiscard]] bool isPointVisiblePlane(vec3 point) const;    // just in front of the camera plane

   private:
    struct CamImpl;
    StaticPImpl<CamImpl, 256> m_impl;
};

#endif
