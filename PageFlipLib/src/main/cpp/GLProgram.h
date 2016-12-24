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

#ifndef ANDROID_PAGEFLIP_GLPROGRAM_H
#define ANDROID_PAGEFLIP_GLPROGRAM_H

#include <GLES2/gl2.h>
#include "GLShader.h"

namespace eschao {

class GLShader;

class GLProgram {

public:
    GLProgram();
    virtual ~GLProgram();

    int init(const char *shaderGLSL, const char *fragmentGLSL);
    virtual void clean();

    inline int programRef() {
        return mProgramRef;
    }

protected:
    virtual void getVarsLocation() = 0;

protected:
    GLuint mProgramRef;
    GLShader mShader;
    GLShader mFragment;
};

}
#endif //ANDROID_PAGEFLIP_GLPROGRAM_H
