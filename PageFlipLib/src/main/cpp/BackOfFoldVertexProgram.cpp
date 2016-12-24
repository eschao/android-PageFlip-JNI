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

#include "BackOfFoldVertexProgram.h"
#include "constant.h"

namespace eschao {

static const auto g_vertex_shader =
        "precision mediump float;\n"
        "uniform mat4 u_MVPMatrix;\n"
        "uniform float u_texXOffset;\n"
        "attribute vec4 a_vexPosition;\n"
        "attribute vec2 a_texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "varying float v_shadowX;\n"
        "\n"
        "void main() {\n"
        "    v_texCoord = vec2(abs(a_texCoord.x - u_texXOffset), a_texCoord.y);\n"
        "    v_shadowX = clamp(abs(a_vexPosition.w), 0.01, 1.0);\n"
        "    vec4 vertex = vec4(a_vexPosition.xyz, 1);\n"
        "    gl_Position = u_MVPMatrix * vertex;\n"
        "}";

static const auto g_fragment_shader =
        "precision mediump float;\n"
        "uniform sampler2D u_texture;\n"
        "uniform sampler2D u_shadow;\n"
        "uniform vec4 u_maskColor;\n"
        "varying vec2 v_texCoord;\n"
        "varying float v_shadowX;\n"
        "\n"
        "void main() {\n"
        "    vec4 texture = texture2D(u_texture, v_texCoord);\n"
        "    vec2 shadowCoord = vec2(v_shadowX, 0);\n"
        "    vec4 shadow = texture2D(u_shadow, shadowCoord);\n"
        "    vec4 maskedTexture = vec4(mix(texture.rgb, u_maskColor.rgb, u_maskColor.a), 1.0);\n"
        "    gl_FragColor = vec4(maskedTexture.rgb * (1.0 - shadow.a) + shadow.rgb, maskedTexture.a);\n"
        "}";

static const char *VAR_TEXTURE_OFFSET = "u_texXOffset";
static const char *VAR_MASK_COLOR = "u_maskColor";
static const char *VAR_SHADOW_TEXTURE = "u_shadow";

BackOfFoldVertexProgram::BackOfFoldVertexProgram()
        : mShadowLoc(Constant::kGlInValidLocation),
          mMaskColorLoc(Constant::kGlInValidLocation),
          mTexXOffsetLoc(Constant::kGlInValidLocation) {
}

BackOfFoldVertexProgram::~BackOfFoldVertexProgram() {
    clean();
}

void BackOfFoldVertexProgram::clean() {
    mShadowLoc = Constant::kGlInValidLocation;
    mMaskColorLoc = Constant::kGlInValidLocation;
    mTexXOffsetLoc = Constant::kGlInValidLocation;

    GLProgram::clean();
}

int BackOfFoldVertexProgram::init() {
    clean();
    return GLProgram::init(g_vertex_shader, g_fragment_shader);
}

void BackOfFoldVertexProgram::getVarsLocation() {
    VertexProgram::getVarsLocation();

    mShadowLoc = glGetUniformLocation(mProgramRef, VAR_SHADOW_TEXTURE);
    mMaskColorLoc = glGetUniformLocation(mProgramRef, VAR_MASK_COLOR);
    mTexXOffsetLoc = glGetUniformLocation(mProgramRef, VAR_TEXTURE_OFFSET);
}

}
