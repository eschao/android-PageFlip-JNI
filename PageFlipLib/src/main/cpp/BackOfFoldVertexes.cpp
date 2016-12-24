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

#include "Page.h"
#include "BackOfFoldVertexes.h"
#include "BackOfFoldVertexProgram.h"

namespace eschao {

void BackOfFoldVertexes::draw(BackOfFoldVertexProgram &program,
                              Page &page,
                              bool hasSecondPage,
                              GLuint gradientLightId) {
    glUniformMatrix4fv(program.mvpMatrixLoc(), 1, GL_FALSE,
                       VertexProgram::MVPMatrix);

    glBindTexture(GL_TEXTURE_2D, page.textures.backTextureId());
    glUniform1i(program.textureLoc(), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gradientLightId);
    glUniform1i(program.shadowLoc(), 1);

    glUniform1f(program.texXOffsetLoc(), hasSecondPage ? 1.0f : 0);

    const float *maskColor = page.textures.getMaskColorOfFirstTexture();
    glUniform4f(program.maskColorLoc(),
                maskColor[0], maskColor[1], maskColor[2],
                hasSecondPage ? 0 : mMaskAlpha);

    drawWith(GL_TRIANGLE_STRIP,
             program.vertexPosLoc(),
             program.texCoordLoc());
}

}
