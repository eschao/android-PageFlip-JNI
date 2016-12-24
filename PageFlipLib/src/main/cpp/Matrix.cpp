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

#include "Matrix.h"

namespace eschao {

#define I(_i, _j) ((_j)+((_i)<<2))

void Matrix::ortho(float *m,
                   float left, float right, float bottom, float top,
                   float near, float far) {
    float rWidth = 1.0f / (right - left);
    float rHeight = 1.0f / (top - bottom);
    float rDepth = 1.0f / (far - near);

    float x = 2.0f * rWidth;
    float y = 2.0f * rHeight;
    float z = -2.0f * rDepth;
    float tx = -(right + left) * rWidth;
    float ty = -(top + bottom) * rHeight;
    float tz = -(far + near) * rDepth;
    m[0] = x;
    m[5] = y;
    m[10] = z;
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
    m[15] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[11] = 0.0f;
}

void Matrix::setIdentity(float *m) {
    for (int i = 0; i < 16; ++i) {
        m[i] = 0;
    }

    for (int i = 0; i < 16; i += 5) {
        m[i] = 1.0f;
    }
}

void Matrix::setLookAt(float *m,
                       float eyeX, float eyeY, float eyeZ,
                       float centerX, float centerY, float centerZ,
                       float upX, float upY, float upZ) {
    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;
    normalize(fx, fy, fz);

    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;
    normalize(sx, sy, sz);

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    m[0] = sx;
    m[1] = ux;
    m[2] = -fx;
    m[3] = 0.0f;

    m[4] = sy;
    m[5] = uy;
    m[6] = -fy;
    m[7] = 0.0f;

    m[8] = sz;
    m[9] = uz;
    m[10] = -fz;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;

    translate(m, -eyeX, -eyeY, -eyeZ);
}

void Matrix::translate(float *m, float x, float y, float z) {
    for (int i = 0; i < 4; ++i) {
        m[12 + i] += m[i] * x + m[4 + i] * y + m[8 + i] * z;
    }
}

void Matrix::multiplyMM(float *m, float *lhs, float *rhs) {
    for (int i = 0; i < 4; i++) {
        const float rhs_i0 = rhs[I(i, 0)];
        float ri0 = lhs[I(0, 0)] * rhs_i0;
        float ri1 = lhs[I(0, 1)] * rhs_i0;
        float ri2 = lhs[I(0, 2)] * rhs_i0;
        float ri3 = lhs[I(0, 3)] * rhs_i0;

        for (int j = 1; j < 4; j++) {
            const float rhs_ij = rhs[I(i, j)];
            ri0 += lhs[I(j, 0)] * rhs_ij;
            ri1 += lhs[I(j, 1)] * rhs_ij;
            ri2 += lhs[I(j, 2)] * rhs_ij;
            ri3 += lhs[I(j, 3)] * rhs_ij;
        }

        m[I(i, 0)] = ri0;
        m[I(i, 1)] = ri1;
        m[I(i, 2)] = ri2;
        m[I(i, 3)] = ri3;
    }
}

}
