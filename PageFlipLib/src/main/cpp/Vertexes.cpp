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

#include "Vertexes.h"
#include "Error.h"
#include "Utility.h"

namespace eschao {

Vertexes::Vertexes()
        : mNext(0),
          mCapacity(0),
          mSizeOfPerVex(0),
          mVertexes(NULL),
          mTexCoords(NULL) {
}

Vertexes::Vertexes(int capacity, int sizeOfPerVex, bool hasTexture) {
    set(capacity, sizeOfPerVex, hasTexture);
}

Vertexes::~Vertexes() {
    release();
}

void Vertexes::release() {
    if (mVertexes) {
        delete[] mVertexes;
    }

    if (mTexCoords) {
        delete[] mTexCoords;
    }

    mNext = 0;
    mCapacity = 0;
    mSizeOfPerVex = 0;
}

int Vertexes::set(int capacity, int sizeOfPerVex, bool hasTexture) {
    if (sizeOfPerVex < 2) {
        return gError.set(Error::ERR_INVALID_PARAMETER);
    }

    release();
    this->mCapacity = capacity;
    this->mSizeOfPerVex = sizeOfPerVex;
    mVertexes = new float[capacity * sizeOfPerVex];

    if (hasTexture) {
        mTexCoords = new float[capacity << 1];
    }

    return Error::OK;
}

Vertexes& Vertexes::addVertex(float x, float y, float z) {
    mVertexes[mNext++] = x;
    mVertexes[mNext++] = y;
    mVertexes[mNext++] = z;
    return *this;
}

Vertexes& Vertexes::addVertex(float x, float y, float z,
                              float tx, float ty) {
    int j = mNext / mSizeOfPerVex * 2;
    mVertexes[mNext++] = x;
    mVertexes[mNext++] = y;
    mVertexes[mNext++] = z;

    mTexCoords[j++] = tx;
    mTexCoords[j] = ty;
    return *this;
}

Vertexes& Vertexes::addVertex(float x, float y, float z, float w) {
    mVertexes[mNext++] = x;
    mVertexes[mNext++] = y;
    mVertexes[mNext++] = z;
    mVertexes[mNext++] = w;
    return *this;
}

Vertexes& Vertexes::addVertex(float x, float y, float z, float w,
                              float tx, float ty) {
    int j = mNext / mSizeOfPerVex * 2;
    mVertexes[mNext++] = x;
    mVertexes[mNext++] = y;
    mVertexes[mNext++] = z;
    mVertexes[mNext++] = w;

    mTexCoords[j++] = tx;
    mTexCoords[j] = ty;
    return *this;
}

Vertexes& Vertexes::addVertex(GLPoint &p) {
    int j = mNext / mSizeOfPerVex * 2;
    mVertexes[mNext++] = p.x;
    mVertexes[mNext++] = p.y;
    mVertexes[mNext++] = p.z;

    mTexCoords[j++] = p.texX;
    mTexCoords[j] = p.texY;
    return *this;
}

void Vertexes::drawWith(GLenum type, GLint vertexPosLoc,
                        GLint texCoordLoc) {
    glVertexAttribPointer(vertexPosLoc, mSizeOfPerVex, GL_FLOAT,
                          GL_FALSE, 0, mVertexes);
    glEnableVertexAttribArray(vertexPosLoc);

    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE,
                          0, mTexCoords);
    glEnableVertexAttribArray(texCoordLoc);

    glDrawArrays(type, 0, count());
}

void Vertexes::drawWith(GLenum type, GLint vertexPosLoc, GLint texCoordLoc,
                        int offset, int length) {
    glVertexAttribPointer(vertexPosLoc, mSizeOfPerVex, GL_FLOAT,
                          GL_FALSE, 0, mVertexes);
    glEnableVertexAttribArray(vertexPosLoc);

    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE,
                          0, mTexCoords);
    glEnableVertexAttribArray(texCoordLoc);

    glDrawArrays(type, offset, length);
}

void Vertexes::printVertexes() {
    const auto TAG = "Vertexes";
    LOGV(TAG, "SizeOfPerVex: %d, Count: %d", mSizeOfPerVex, mNext);

    std::string s;
    char buf[256];
    for (int i = 0; i < mNext; i += mSizeOfPerVex) {
        s.clear();
        sprintf(buf, "[%d-%d]: ", i, i + mSizeOfPerVex);
        s.append(buf);

        for (int j = 0; j < mSizeOfPerVex; ++j) {
            sprintf(buf, "%f, ", mVertexes[i+j]);
            s.append(buf);
        }

        LOGV(TAG, "%s", s.c_str());
    }
}

}
