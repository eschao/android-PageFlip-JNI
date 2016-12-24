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

#include "VertexProgram.h"
#include "constant.h"
#include "Matrix.h"

namespace eschao {

static const auto g_vertex_shader =
        "precision mediump float;\n"
        "uniform mat4 u_MVPMatrix;\n"
        "attribute vec4 a_vexPosition;\n"
        "attribute vec2 a_texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = u_MVPMatrix * a_vexPosition;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";

static const auto g_fragment_shader =
        "precision mediump float;\n"
        "uniform sampler2D u_texture;\n"
        "varying vec2 v_texCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(u_texture, v_texCoord);\n"
        "}";

static const char* VAR_MVP_MATRIX       = "u_MVPMatrix";
static const char* VAR_VERTEX_POS       = "a_vexPosition";
static const char* VAR_TEXTURE_COORD    = "a_texCoord";
static const char* VAR_TEXTURE          = "u_texture";

float VertexProgram::MVMatrix[16]   = {0};
float VertexProgram::MVPMatrix[16]  = {0};

VertexProgram::VertexProgram()
        : mMVPMatrixLoc(Constant::kGlInValidLocation),
          mVertexPosLoc(Constant::kGlInValidLocation),
          mTexCoordLoc(Constant::kGlInValidLocation),
          mTextureLoc(Constant::kGlInValidLocation) {
}

VertexProgram::~VertexProgram() {
    clean();
}

void VertexProgram::clean() {
    mTextureLoc = Constant::kGlInValidLocation;
    mMVPMatrixLoc = Constant::kGlInValidLocation;
    mTexCoordLoc = Constant::kGlInValidLocation;
    mVertexPosLoc = Constant::kGlInValidLocation;

    GLProgram::clean();
}

int VertexProgram::init() {
    clean();
    return GLProgram::init(g_vertex_shader, g_fragment_shader);
}

void VertexProgram::initMatrix(float left, float right,
                               float bottom, float top) {
    float projectMatrix[16];
    Matrix::ortho(projectMatrix, left, right, bottom, top, 0, 6000);
    Matrix::setIdentity(MVMatrix);
    Matrix::setLookAt(MVMatrix, 0, 0, 3000, 0, 0, 0, 0, 1, 0);
    Matrix::setIdentity(MVPMatrix);
    Matrix::multiplyMM(MVPMatrix, projectMatrix, MVMatrix);
}

void VertexProgram::getVarsLocation() {
    mTextureLoc = glGetUniformLocation(mProgramRef, VAR_TEXTURE);
    mMVPMatrixLoc = glGetUniformLocation(mProgramRef, VAR_MVP_MATRIX);
    mTexCoordLoc = glGetAttribLocation(mProgramRef, VAR_TEXTURE_COORD);
    mVertexPosLoc = glGetAttribLocation(mProgramRef, VAR_VERTEX_POS);
}

}
