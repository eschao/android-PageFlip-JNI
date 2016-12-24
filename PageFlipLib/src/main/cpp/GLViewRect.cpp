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

#include "GLViewRect.h"

namespace eschao {

GLViewRect::GLViewRect()
        : left(0), right(0), top(0), bottom(0), width(0), height(0),
          halfWidth(0), halfHeight(0), marginLeft(0), marginRight(0),
          surfaceWidth(0), surfaceHeight(0) {
}

GLViewRect::GLViewRect(float surfaceWidth, float surfaceHeight,
                       float marginLeft, float marginRight) {
    set(surfaceWidth, surfaceHeight, marginLeft, marginRight);
}

void GLViewRect::set(float surfaceWidth, float surfaceHeight,
                     float marginLeft, float marginRight) {
    this->surfaceWidth = surfaceWidth;
    this->surfaceHeight = surfaceHeight;
    this->marginLeft = marginLeft;
    this->marginRight = marginRight;

    width = surfaceWidth - marginLeft - marginRight;
    height = surfaceHeight;
    halfWidth = width * 0.5f;
    halfHeight = height * 0.5f;
    left = -halfWidth + marginLeft;
    right = halfWidth - marginRight;
    top = halfHeight;
    bottom = -halfHeight;
}

}
