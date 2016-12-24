/*
 * Copyright (C) 2016 eschao <esc.chao@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GLShader.h"
#include "constant.h"
#include "Error.h"

namespace eschao {

GLShader::GLShader()
        : mShaderRef(Constant::kGlInvalidRef) {
}

GLShader::~GLShader() {
    clean();
}

int GLShader::load(GLenum type, const char* shaderGLSL) {
    clean();

    mShaderRef = glCreateShader(type);
    if (mShaderRef == Constant::kGlInvalidRef) {
        return gError.set(Error::ERR_GL_CREATE_SHADER_REF);
    }

    glShaderSource(mShaderRef, 1, &shaderGLSL, NULL);
    glCompileShader(mShaderRef);

    GLint compiled = 0;
    glGetShaderiv(mShaderRef, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        gError.set(Error::ERR_GL_COMPILE_SHADER);
        GLint infoLen = 0;
        glGetShaderiv(mShaderRef, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen) {
            if (infoLen > Error::MAX_ERR_DESC_LENGTH) {
                infoLen = Error::MAX_ERR_DESC_LENGTH;
            }
            glGetShaderInfoLog(mShaderRef, infoLen, NULL,
                               const_cast<char*>(gError.desc()));
            gError.endDesc(infoLen + 1);
        }

        glDeleteShader(mShaderRef);
        mShaderRef = Constant::kGlInvalidRef;
        return gError.code();
    }

    return Error::OK;
}

void GLShader::clean() {
    if (mShaderRef != Constant::kGlInvalidRef) {
        glDeleteShader(mShaderRef);
        mShaderRef = Constant::kGlInvalidRef;
    }
}

}
