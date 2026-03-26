// Copyright (c) 2026, WH, All rights reserved.
// dummy Shader

#pragma once
#include "Shader.h"

class NullShader : public Shader {
   public:
    NullShader();

    void enable() override;
    void disable() override;

   protected:
    void init() override;
    void initAsync() override;
    void destroy() override;

    void writeUniform(std::string_view name, UniformType type, const void *data, unsigned int dataSize) override;
};
