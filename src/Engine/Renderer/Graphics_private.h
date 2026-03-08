#pragma once
#include "Matrices.h"

// only included by renderer implementations' (XYZInterface.cpp) source files
struct GraphicsPrivateData {
    std::vector<Graphics::ScreenshotParams> pendingScreenshots;

    // transforms
    std::vector<Matrix4> worldTransformStack;
    std::vector<Matrix4> projectionTransformStack;

    std::vector<std::array<int, 4>> viewportStack;
    std::vector<vec2> resolutionStack;

    // 3d gui scenes
    std::vector<bool> scene3d_stack;
    Matrix4 scene3d_world_matrix;
    Matrix4 scene3d_projection_matrix;

    // transforms
    Matrix4 projectionMatrix;
    Matrix4 worldMatrix;
    Matrix4 MP;

    McRect scene3d_region;
    vec3 v3dSceneOffset{0.f};

    // info
    DrawBlendMode currentBlendMode{DrawBlendMode::ALPHA};
    bool bBlendingEnabled{true};
    bool bTransformUpToDate{false};
    bool bIs3dScene{false};
};
