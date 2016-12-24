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

#ifndef ANDROID_PAGEFLIP_GLVIEWRECT_H
#define ANDROID_PAGEFLIP_GLVIEWRECT_H

namespace eschao {

class GLViewRect {

public:
    GLViewRect();
    GLViewRect(float surfaceWidth, float surfaceHeight,
               float marginLeft, float marginRight);

    void set(float surfaceWidth, float surfaceHeight,
             float marginLeft, float marginRight);

    inline void set(float surfaceWidth, float surfaceHeight) {
        set(surfaceWidth, surfaceHeight, marginLeft, marginRight);
    }

    inline void setMargin(float marginLeft, float marginRight) {
        set(surfaceWidth, surfaceHeight, marginLeft, marginRight);
    }

    inline float minOfWidthHeight() {
        return width > height ? width : height;
    }

    inline float toOpenGLX(float x) {
        return x - halfWidth;
    }

    inline float toOpenGLY(float y) {
        return halfHeight - y;
    }

public:
    float left;
    float right;
    float top;
    float bottom;
    float width;
    float height;
    float halfWidth;
    float halfHeight;

    float marginLeft;
    float marginRight;
    float surfaceWidth;
    float surfaceHeight;
};

}
#endif //ANDROID_PAGEFLIP_GLVIEWRECT_H
