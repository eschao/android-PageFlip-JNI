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

#ifndef ANDROID_PAGEFLIP_VERTEXES_H
#define ANDROID_PAGEFLIP_VERTEXES_H

#include <GLES2/gl2.h>
#include "GLPoint.h"

namespace eschao {

class Vertexes {

public:
    Vertexes();
    Vertexes(int capacity, int sizeOfPerVex, bool hasTexture = false);
    virtual ~Vertexes();

    void release();
    int set(int capacity, int sizeOfPerVex, bool hasTexture = false);
    Vertexes& addVertex(float x, float y, float z);
    Vertexes& addVertex(float x, float y, float z, float w);
    Vertexes& addVertex(float x, float y, float z, float tx, float ty);
    Vertexes& addVertex(float x, float y, float z, float w, float tx, float ty);
    Vertexes& addVertex(GLPoint &p);
    void drawWith(GLenum type, GLint vertexPosLoc, GLint texCoordLoc);
    void drawWith(GLenum type, GLint vertexPosLoc, GLint texCoordLoc,
                  int offset, int length);
    void printVertexes();

    // inline
    inline int capacity() {
        return mCapacity;
    };

    inline int count() {
        return mNext / mSizeOfPerVex;
    }

    inline int sizeOfPerVex() {
        return mSizeOfPerVex;
    }

    inline void reset() {
        mNext = 0;
    }

    inline float floatAt(int index) {
        return (index >= 0 && index < mNext) ? mVertexes[index] : 0;
    }

protected:
    int mSizeOfPerVex;
    int mCapacity;
    int mNext;

    float* mVertexes;
    float* mTexCoords;
};

}
#endif //ANDROID_PAGEFLIP_VERTEXES_H
