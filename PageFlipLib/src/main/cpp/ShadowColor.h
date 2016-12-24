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

#ifndef ANDROID_PAGEFLIP_SHADOWCOLOR_H
#define ANDROID_PAGEFLIP_SHADOWCOLOR_H

#include "Error.h"

namespace eschao {

class ShadowColor {

public:

    ShadowColor() : startColor(0), startAlpha(0), endColor(0),
                    endAlpha(0) { }

    inline int set(float startColor, float startAlpha,
                   float endColor, float endAlpha) {
        if (startColor < 0 || startColor > 1 ||
            startAlpha < 0 || startAlpha > 1 ||
            endColor < 0 || endColor > 1 ||
            endAlpha < 0 || endAlpha > 1) {
            return gError.set(Error::ERR_INVALID_PARAMETER);
        }

        this->startColor = startColor;
        this->startAlpha = startAlpha;
        this->endColor = endColor;
        this->endAlpha = endAlpha;
        return Error::OK;
    }

public:
    float startColor;
    float startAlpha;
    float endColor;
    float endAlpha;
};

}
#endif //ANDROID_PAGEFLIP_SHADOWCOLOR_H
