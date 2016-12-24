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

#ifndef ANDROID_PAGEFLIP_PAGE_H
#define ANDROID_PAGEFLIP_PAGE_H

#include <GLES2/gl2.h>
#include <string.h>
#include <android/bitmap.h>
#include "GLPoint.h"
#include "VertexProgram.h"
#include "Vertexes.h"
#include "PointF.h"
#include "Error.h"
#include "Utility.h"

#define FIRST_TEXTURE_ID    0
#define SECOND_TEXTURE_ID   1
#define BACK_TEXTURE_ID     2
#define TEXTURE_SIZE        3

namespace eschao {

class PageFlip;

struct Texture_ {
    GLuint texId;
    bool isSet;
    float maskColor[3];

    Texture_() : texId(0), isSet(false), maskColor{0} { }
    Texture_(GLuint tId, bool set) : texId(tId), isSet(set), maskColor{0} { }

    Texture_& operator=(const Texture_& rhs) {
        texId = rhs.texId;
        isSet = rhs.isSet;
        maskColor[0] = rhs.maskColor[0];
        maskColor[1] = rhs.maskColor[1];
        maskColor[2] = rhs.maskColor[2];
        return *this;
    }

    inline void setMaskColor(int color) {
        maskColor[0] = RED(color) / 255.0f;
        maskColor[1] = GREEN(color) / 255.0f;
        maskColor[2] = BLUE(color) / 255.0f;
    }
};

struct TexRecycler_ {
    GLuint texIds[TEXTURE_SIZE << 1];
    int size;

    TexRecycler_() : size(0) { }

    inline void recycle() {
        if (size > 0) {
            glDeleteTextures(size, texIds);
            size = 0;
        }
    }

    inline void add(Texture_& tex) {
        if (tex.isSet) {
            texIds[size++] = tex.texId;
            tex.isSet = false;
        }
    }
};

/**
 * Class Textures
 */
class Textures {
public:

    void setFirstTextureWithSecond();
    void setSecondTextureWithFirst();
    void swapTexturesWith(Textures &rhs);

    inline GLuint backTextureId() {
        return mTextures[BACK_TEXTURE_ID].isSet ?
               mTextures[BACK_TEXTURE_ID].texId :
               mTextures[FIRST_TEXTURE_ID].texId;
    }

    inline bool isFirstTextureSet() {
        return mTextures[FIRST_TEXTURE_ID].isSet;
    }

    inline bool isSecondTextureSet() {
        return mTextures[SECOND_TEXTURE_ID].isSet;
    }

    inline bool isBackTextureSet() {
        return mTextures[BACK_TEXTURE_ID].isSet;
    }

    inline float* getMaskColorOfFirstTexture() {
        return mTextures[FIRST_TEXTURE_ID].maskColor;
    }

    inline void recycle() {
        mRecycler.recycle();
    }

    inline void recycleAll() {
        for (int i = 0; i < TEXTURE_SIZE; ++i) {
            if (mTextures[i].isSet) {
                glDeleteTextures(1, &mTextures[i].texId);
                mTextures[i].isSet = false;
            }
        }
    }

    inline int setFirstTexture(AndroidBitmapInfo &info, GLvoid *data) {
        return setTexture(FIRST_TEXTURE_ID, info, data);
    }

    inline int setSecondTexture(AndroidBitmapInfo &info, GLvoid *data) {
        return setTexture(SECOND_TEXTURE_ID, info, data);
    }

    inline int setBackTexture(AndroidBitmapInfo &info, GLvoid *data) {
        if (data == NULL) {
            // recycle back texture
            mRecycler.add(mTextures[BACK_TEXTURE_ID]);
            return Error::OK;
        }

        return setTexture(BACK_TEXTURE_ID, info, data);
    }

private:
    int setTexture(int index, AndroidBitmapInfo &info, GLvoid *data);

private:
    Texture_ mTextures[TEXTURE_SIZE];
    TexRecycler_ mRecycler;

    friend class Page;
};

/**
 * Class Page
 */
class Page {

public:
    Page();
    Page(float left, float right, float top, float bottom);

    void init(float left, float right, float top, float bottom);
    void setOriginDiagonalPoints(bool hasSecondPage, bool isTopArea);
    void invertYOfOriginP();
    void drawFrontPage(VertexProgram &program, Vertexes &vertexes);
    void buildVertexesOfPageWhenVertical(Vertexes& frontVertexes,
                                         PointF& xFoldP1);
    void buildVertexesOfPageWhenSlope(Vertexes& frontVertexes,
                                      PointF& xFoldP1,
                                      PointF& yFoldP1,
                                      float kValue);
    void buildVertexesOfFullPage();

    inline float width() {
        return mWidth;
    }

    inline float height() {
        return mHeight;
    }

    inline bool isLeftPage() {
        return mRight <= 0;
    }

    inline bool isRightPage() {
        return mLeft >= 0;
    }

    inline bool contains(float x, float y) {
        return mLeft < mRight && mBottom < mTop &&
               mLeft <= x && x < mRight &&
               mBottom <= y && y < mTop;
    }

    inline bool isXInRange(float x, float ratio) {
        const float w = mWidth * ratio;
        return (mOriginP.x < 0) ? x < (mOriginP.x + w) :
               x > (mOriginP.x - w);
    }

    inline bool isXOutsidePage(float x) {
        return (mOriginP.x < 0) ? x > mDiagonalP.x : x < mDiagonalP.x;
    }

    inline float textureX(float x) {
        return (x - mLeft) / mTexWidth;
    }

    inline float textureY(float y) {
        return (mTop - y) / mTexHeight;
    }

    inline void drawFullPage(VertexProgram &program, bool isFirst) {
        isFirst ?
        drawFullPage(program, textures.mTextures[FIRST_TEXTURE_ID].texId) :
        drawFullPage(program, textures.mTextures[SECOND_TEXTURE_ID].texId);
    }

private:
    void computeIndexOfApexOrder();
    void drawFullPage(VertexProgram &program, GLuint textureId);

public:
    Textures textures;

private:

    /**
     * <p>
     * 4 apexes of page has different permutation order according to original
     * point since original point will be changed when user click to curl page
     * from different direction. There are 4 kinds of order:
     * </p><pre>
     *   A           B           C           D
     * 2    1      3    0      0    3      1    2
     * +----+      +----+      +----+      +----+
     * |    |      |    |      |    |      |    |
     * +----+      +----+      +----+      +----+
     * 3    0      2    1      1    2      0    3
     *             From A      From A      From A
     *             0 <-> 1     0 <-> 2     0 <-> 3
     *             3 <-> 2     3 <-> 1     1 <-> 2
     * </pre>
     * <ul>
     *      <li>0 always represents the origin point, accordingly 2 is diagonal
     *      point</li>
     *      <li>Case A is default order: 0 -> 1 -> 2 -> 3</li>
     *      <li>Every apex data is stored in mApexes following the case A order
     *      and never changed</li>
     *      <li>This array is mapping apex order (case A - D) to real apex data
     *      stored in mApexes. For example:
     *      <ul>
     *          <li>Case A has same order with storing sequence of apex data in
     *          mApexes</li>
     *          <li>Case B: the 0 apex is stored in 1 position in mApexes</li>
     *      </ul></li>
     *  </ul>
     */
    static const int mPageApexOrders[][4];

    /**
     * <p>When page is curled, there are 4 kinds of mVertexes orders for drawing
     * first texture and second texture with TRIANGLE_STRIP way</p><pre>
     *     A             B              C              D
     * 2       1     2     X 1      2 X     1      2       1
     * +-------+     +-----.-+      +-.-----+      +-------+
     * |       |     | F  /  |      |/      |      |   F   |
     * |   F   .Y    |   /   |     Y.   S   |     X.-------.Y
     * |      /|     |  /    |      |       |      |   S   |
     * +-----.-+     +-.-----+      +-------+      +-------+
     * 3    X  0     3 Y     0      3       0      3       0
     * </pre>
     * <ul>
     *      <li>All cases are based on the apex order case A(0 -> 1 -> 2 -> 3)
     *      </li>
     *      <li>F means the first texture area, S means the second texture area
     *      </li>
     *      <li>X is xFoldX point, Y is yFoldY point</li>
     *      <li>Case A means: xFoldX and yFoldY are both in page</li>
     *      <li>Case B means: xFoldX is in page, but yFoldY is the intersecting
     *      point with line 1->2 since yFoldY is outside the page</li>
     *      <li>Case C means: xFoldX and yFoldY are both outside the page</li>
     *      <li>Case D means: xFoldX outside page but yFoldY is in the page</li>
     *      <li>Combining {@link #mPageApexOrders} with this array, we can get
     *      the right apex data from mApexes array which will help us quickly
     *      organizing triangle data for openGL drawing</li>
     *      <li>The last array(Case E) in this array means: xFoldX and yFoldY
     *      are both outside the page and the whole page will be draw with
     *      second texture</li>
     * </ul>
     */
    static const int mFoldVexOrders[][5];
    static const int VEX_ORDER_LEN = sizeof(mFoldVexOrders[0]) / sizeof(int);

    // page size
    float mLeft;
    float mRight;
    float mTop;
    float mBottom;
    float mWidth;
    float mHeight;

    // texture size for rendering page, normally they are same with page width
    // and height
    float mTexWidth;
    float mTexHeight;

    /**
     * <p>origin point and diagonal point</p>
     * <pre>
     * 0-----+
     * |     |
     * |     |
     * +-----1
     * </pre>
     * <p>if origin(x, y) is 1, the diagonal(x, y) is 0</p>
     */
    GLPoint mOriginP;
    GLPoint mDiagonalP;

    GLPoint mXFoldP;
    GLPoint mYFoldP;

    // storing 4 apexes data of page
    float mApexes[12];
    // texture coordinates for page apex
    float mApexTexCoords[8];
    // vertex size of front of fold page and unfold page
    int mFrontVertexCount;
    // index of apex order array for current original point
    int mApexOrderIndex;

    friend class PageFlip;
};

}
#endif //ANDROID_PAGEFLIP_PAGE_H
