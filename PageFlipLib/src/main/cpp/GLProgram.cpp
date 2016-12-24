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

#include <iostream>
#include "GLProgram.h"
#include "Error.h"
#include "constant.h"
#include "Utility.h"

using namespace std;

namespace eschao {

GLProgram::GLProgram() {
    mProgramRef = Constant::kGlInvalidRef;
}

GLProgram::~GLProgram() {
    clean();
}

void GLProgram::clean() {
    mShader.clean();
    mFragment.clean();

    if (mProgramRef != Constant::kGlInvalidRef) {
        glDeleteProgram(mProgramRef);
        mProgramRef = Constant::kGlInvalidRef;
    }

    Error::cleanGlError();
}

int GLProgram::init(const char* shaderGLSL, const char* fragmentGLSL) {
    if (shaderGLSL == NULL || fragmentGLSL == NULL) {
        return gError.set(Error::ERR_NULL_PARAMETER);
    }

    if (Error::OK != mShader.load(GL_VERTEX_SHADER, shaderGLSL)) {
        return gError.code();
    }
    if (Error::OK != mFragment.load(GL_FRAGMENT_SHADER, fragmentGLSL)) {
        return gError.code();
    }

    mProgramRef = glCreateProgram();
    if (mProgramRef == Constant::kGlInvalidRef) {
        mShader.clean();
        mFragment.clean();
        return gError.set(Error::ERR_GL_CREATE_PROGRAM_REF);
    }

    glAttachShader(mProgramRef, mShader.shaderRef());
    int error = gError.checkGlError("When attach mShader(glAttachShader)");
    if (error != Error::OK) {
        clean();
        LOGV("GLProgram", "%s", gError.desc());
        return gError.set(Error::ERR_GL_ATTACH_SHADER);
    }

    glAttachShader(mProgramRef, mFragment.shaderRef());
    error = gError.checkGlError("When attach mFragment(glAttachShader");
    if (error != Error::OK) {
        clean();
        return gError.set(Error::ERR_GL_ATTACH_FRAGMENT);
    }

    glLinkProgram(mProgramRef);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(mProgramRef, GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
        gError.set(Error::ERR_GL_LINK_PROGRAM);
        GLint infoLen = 0;
        glGetProgramiv(mProgramRef, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen) {
            if (infoLen > Error::MAX_ERR_DESC_LENGTH) {
                infoLen = Error::MAX_ERR_DESC_LENGTH;
            }
            glGetProgramInfoLog(mProgramRef, infoLen, NULL,
                                const_cast<char*>(gError.desc()));
            gError.endDesc(infoLen + 1);
        }

        clean();
        return gError.code();
    }

    glUseProgram(mProgramRef);
    getVarsLocation();
    return Error::OK;
}

}
