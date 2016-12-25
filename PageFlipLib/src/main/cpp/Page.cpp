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

#include <algorithm>
#include <android/bitmap.h>
#include "Page.h"

using namespace std;

namespace eschao {

void Textures::setFirstTextureWithSecond() {
    mRecycler.add(mTextures[FIRST_TEXTURE_ID]);
    mTextures[FIRST_TEXTURE_ID] = mTextures[SECOND_TEXTURE_ID];
    mTextures[SECOND_TEXTURE_ID].isSet = false;
}

void Textures::setSecondTextureWithFirst() {
    mRecycler.add(mTextures[SECOND_TEXTURE_ID]);
    mTextures[SECOND_TEXTURE_ID] = mTextures[FIRST_TEXTURE_ID];
    mTextures[FIRST_TEXTURE_ID].isSet = false;
}

void Textures::swapTexturesWith(Textures &rhs) {
    mRecycler.add(mTextures[SECOND_TEXTURE_ID]);
    mTextures[SECOND_TEXTURE_ID] = mTextures[FIRST_TEXTURE_ID];

    mRecycler.add(mTextures[BACK_TEXTURE_ID]);
    mTextures[BACK_TEXTURE_ID] = rhs.mTextures[FIRST_TEXTURE_ID];

    mTextures[FIRST_TEXTURE_ID] = rhs.mTextures[BACK_TEXTURE_ID];
    rhs.mTextures[BACK_TEXTURE_ID].isSet = false;

    rhs.mTextures[FIRST_TEXTURE_ID]= rhs.mTextures[SECOND_TEXTURE_ID];
    rhs.mTextures[SECOND_TEXTURE_ID].isSet = false;
}

int Textures::setTexture(int index, AndroidBitmapInfo &info, GLvoid *data) {
    mTextures[index].setMaskColor(computeAverageColor(info, data, 30));

    GLint format;
    GLenum type;
    if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
    }
    else if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    else {
        return Error::ERR_UNSUPPORT_BITMAP_FORMAT;
    }

    GLuint id;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format, info.width, info.height, 0, format,
                 type, data);
    mTextures[index].texId = id;
    mTextures[index].isSet = true;
    return Error::OK;
}

const int Page::mPageApexOrders[][4] = {
        {0, 1, 2, 3}, // for case A
        {1, 0, 3, 2}, // for case B
        {2, 3, 0, 1}, // for case C
        {3, 2, 1, 0}, // for case D
};

const int Page::mFoldVexOrders[][5] = {
        {4, 3, 1, 2, 0}, // Case A
        {3, 3, 2, 0, 1}, // Case B
        {3, 2, 1, 3, 0}, // Case C
        {2, 2, 3, 1, 0}, // Case D
        {1, 0, 1, 3, 2}, // Case E
};

Page::Page() {
    init(0, 0, 0, 0);
}

Page::Page(float left, float right, float top, float bottom) {
    init(left, right, top, bottom);
}

void Page::init(float left, float right, float top, float bottom) {
    mTop = top;
    mLeft = left;
    mRight = right;
    mBottom = bottom;

    mWidth = right - left;
    mHeight = top - bottom;

    mTexWidth = mWidth;
    mTexHeight = mHeight;

    mFrontVertexCount = 0;
    mApexOrderIndex = 0;

    buildVertexesOfFullPage();
}

void Page::computeIndexOfApexOrder() {
    mApexOrderIndex = 0;
    if (mOriginP.x < mRight && mOriginP.y < 0) {
        mApexOrderIndex = 3;
    }
    else {
        if (mOriginP.y > 0) {
            mApexOrderIndex++;
        }
        if (mOriginP.x < mRight) {
            mApexOrderIndex++;
        }
    }
}

void Page::setOriginDiagonalPoints(bool hasSecondPage, bool isTopArea) {
    if (hasSecondPage && mLeft < 0) {
        mOriginP.x = mLeft;
        mDiagonalP.x = mRight;
    }
    else {
        mOriginP.x = mRight;
        mDiagonalP.x = mLeft;
    }

    if (isTopArea) {
        mOriginP.y = mBottom;
        mDiagonalP.y = mTop;
    }
    else {
        mOriginP.y = mTop;
        mDiagonalP.y = mBottom;
    }

    computeIndexOfApexOrder();

    mOriginP.texX = (mOriginP.x - mLeft) / mTexWidth;
    mOriginP.texY = (mTop - mOriginP.y) / mTexHeight;
    mDiagonalP.texX = (mDiagonalP.x - mLeft) / mTexWidth;
    mDiagonalP.texY = (mTop - mDiagonalP.y) / mTexHeight;
}

void Page::invertYOfOriginP() {
    swap(mOriginP.y, mDiagonalP.y);
    swap(mOriginP.texY, mDiagonalP.texY);
    computeIndexOfApexOrder();
}

void Page::drawFrontPage(VertexProgram &program, Vertexes &vertexes) {
    // 1. draw unfold part and curled part with the first texture
    glUniformMatrix4fv(program.mvpMatrixLoc(), 1, GL_FALSE,
                       VertexProgram::MVPMatrix);
    glBindTexture(GL_TEXTURE_2D, textures.mTextures[FIRST_TEXTURE_ID].texId);
    glUniform1i(program.textureLoc(), 0);
    vertexes.drawWith(GL_TRIANGLE_STRIP,
                      program.vertexPosLoc(), program.texCoordLoc(),
                      0, mFrontVertexCount);

    // 2. draw the second texture
    glBindTexture(GL_TEXTURE_2D, textures.mTextures[SECOND_TEXTURE_ID].texId);
    glUniform1i(program.textureLoc(), 0);
    glDrawArrays(GL_TRIANGLE_STRIP, mFrontVertexCount,
                 vertexes.count() - mFrontVertexCount);
}

void Page::drawFullPage(VertexProgram &program, GLuint textureId) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(program.textureLoc(), 0);

    glVertexAttribPointer(program.vertexPosLoc(), 3, GL_FLOAT, GL_FALSE, 0,
                          mApexes);
    glEnableVertexAttribArray(program.vertexPosLoc());

    glVertexAttribPointer(program.texCoordLoc(), 2, GL_FLOAT, GL_FALSE, 0,
                          mApexTexCoords);
    glEnableVertexAttribArray(program.texCoordLoc());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Page::buildVertexesOfPageWhenVertical(Vertexes &frontVertexes,
                                           PointF &xFoldP1) {
    // if xFoldX and yFoldY are both outside the page, use the last vertex
    // order to draw page
    int index = 4;

    // compute xFoldX and yFoldY points
    if (!isXOutsidePage(xFoldP1.x)) {
        // use the case B of vertex order to draw page
        index = 1;
        float cx = textureX(xFoldP1.x);
        mXFoldP.set(xFoldP1.x, mOriginP.y, 0, cx, mOriginP.texY);
        mYFoldP.set(xFoldP1.x, mDiagonalP.y, 0, cx, mDiagonalP.texY);
    }

    // get apex order and fold vertex order
    const int *apex_order = mPageApexOrders[mApexOrderIndex];
    const int *vex_order = mFoldVexOrders[index];

    // need to draw first texture, add xFoldX and yFoldY first. Remember
    // the adding order of vertex in float buffer is X point prior to Y
    // point
    if (vex_order[0] > 1) {
        frontVertexes.addVertex(mXFoldP).addVertex(mYFoldP);
    }

    // add the leftover mVertexes for the first texture
    for (int i = 1; i < vex_order[0]; ++i) {
        int k = apex_order[vex_order[i]];
        int m = k * 3;
        int n = k << 1;
        frontVertexes.addVertex(mApexes[m], mApexes[m + 1], 0,
                                 mApexTexCoords[n],
                                 mApexTexCoords[n + 1]);
    }

    // the vertex size for drawing front of fold page and first texture
    mFrontVertexCount = frontVertexes.count();

    // if xFoldX and yFoldY are in the page, need add them for drawing the
    // second texture
    if (vex_order[0] > 1) {
        mXFoldP.z = mYFoldP.z = -1;
        frontVertexes.addVertex(mXFoldP).addVertex(mYFoldP);
    }

    // add the remaining mVertexes for the second texture
    for (int i = vex_order[0]; i < VEX_ORDER_LEN; ++i) {
        int k = apex_order[vex_order[i]];
        int m = k * 3;
        int n = k << 1;
        frontVertexes.addVertex(mApexes[m], mApexes[m + 1], -1,
                                 mApexTexCoords[n],
                                 mApexTexCoords[n + 1]);
    }
}

void Page::buildVertexesOfPageWhenSlope(Vertexes &frontVertexes,
                                        PointF &xFoldP1,
                                        PointF &yFoldP1,
                                        float kValue) {
    // compute xFoldX point
    float half_h = mHeight * 0.5f;
    int index = 0;
    mXFoldP.set(xFoldP1.x, mOriginP.y, 0, textureX(xFoldP1.x), mOriginP.texY);
    if (isXOutsidePage(xFoldP1.x)) {
        index = 2;
        mXFoldP.x = mDiagonalP.x;
        mXFoldP.y = mOriginP.y + (mXFoldP.x - mDiagonalP.x) / kValue;
        mXFoldP.texX = mDiagonalP.texX;
        mXFoldP.texY = textureY(mXFoldP.y);
    }

    // compute yFoldY point
    mYFoldP.set(mOriginP.x, yFoldP1.y, 0, mOriginP.texX, textureY(yFoldP1.y));
    if (fabs(yFoldP1.y) > half_h) {
        index++;
        mYFoldP.x = mOriginP.x + kValue * (yFoldP1.y - mDiagonalP.y);
        if (isXOutsidePage(mYFoldP.x)) {
            index++;
        }
        else {
            mYFoldP.y = mDiagonalP.y;
            mYFoldP.texX = textureX(mYFoldP.x);
            mYFoldP.texY = mDiagonalP.texY;
        }
    }

    // get apex order and fold vertex order
    const int* const apex_order = mPageApexOrders[mApexOrderIndex];
    const int* const vex_order = mFoldVexOrders[index];

    // need to draw first texture, add xFoldX and yFoldY first. Remember
    // the adding order of vertex in float buffer is X point prior to Y
    // point
    if (vex_order[0] > 1) {
        frontVertexes.addVertex(mXFoldP).addVertex(mYFoldP);
    }

    // add the leftover mVertexes for the first texture
    for (int i = 1; i < vex_order[0]; ++i) {
        int k = apex_order[vex_order[i]];
        int m = k * 3;
        int n = k << 1;
        frontVertexes.addVertex(mApexes[m], mApexes[m + 1], 0,
                                 mApexTexCoords[n],
                                 mApexTexCoords[n + 1]);
    }

    // the vertex size for drawing front of fold page and first texture
    mFrontVertexCount = frontVertexes.count();

    // if xFoldX and yFoldY are in the page, need add them for drawing the
    // second texture
    if (vex_order[0] > 1) {
        mXFoldP.z = mYFoldP.z = -1;
        frontVertexes.addVertex(mXFoldP).addVertex(mYFoldP);
    }

    // add the remaining mVertexes for the second texture
    for (int i = vex_order[0]; i < VEX_ORDER_LEN; ++i) {
        int k = apex_order[vex_order[i]];
        int m = k * 3;
        int n = k << 1;
        frontVertexes.addVertex(mApexes[m], mApexes[m + 1], -1,
                                 mApexTexCoords[n],
                                 mApexTexCoords[n + 1]);
    }
}

void Page::buildVertexesOfFullPage() {
    int i = 0;
    int j = 0;

    mApexes[i++] = mRight;
    mApexes[i++] = mBottom;
    mApexes[i++] = 0;
    mApexTexCoords[j++] = textureX(mRight);
    mApexTexCoords[j++] = textureY(mBottom);

    mApexes[i++] = mRight;
    mApexes[i++] = mTop;
    mApexes[i++] = 0;
    mApexTexCoords[j++] = textureX(mRight);
    mApexTexCoords[j++] = textureY(mTop);

    mApexes[i++] = mLeft;
    mApexes[i++] = mTop;
    mApexes[i++] = 0;
    mApexTexCoords[j++] = textureX(mLeft);
    mApexTexCoords[j++] = textureY(mTop);

    mApexes[i++] = mLeft;
    mApexes[i++] = mBottom;
    mApexes[i] = 0;
    mApexTexCoords[j++] = textureX(mLeft);
    mApexTexCoords[j] = textureY(mBottom);
}

}
