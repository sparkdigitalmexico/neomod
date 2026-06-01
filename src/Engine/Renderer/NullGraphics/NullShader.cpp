// Copyright (c) 2026, WH, All rights reserved.
#include "NullShader.h"

NullShader::NullShader() : Shader() {}

void NullShader::enable() {}
void NullShader::disable() {}

void NullShader::init() { this->setReady(true); }
void NullShader::initAsync() { this->setAsyncReady(true); }
void NullShader::destroy() {}

void NullShader::writeUniform(std::string_view name, UniformType type, const void *const data, unsigned int dataSize) {
    (void)name;
    (void)type;
    (void)data;
    (void)dataSize;
}
