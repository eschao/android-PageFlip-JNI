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

#ifndef ANDROID_PAGEFLIP_SHADOWWIDTH_H
#define ANDROID_PAGEFLIP_SHADOWWIDTH_H

#include "Error.h"

namespace eschao {

class ShadowWidth {

public:
    ShadowWidth() : m_min(0), m_max(0), m_ratio(0) { }

    ShadowWidth(float min, float max, float ratio)
    {
        set(min, max, ratio);
    }

    inline int set(float min, float max, float ratio)
    {
        if (min < 0 || max < 0 || min > max ||
            ratio <= 0 || ratio > 1) {
            return gError.set(Error::ERR_INVALID_PARAMETER);
        }

        this->m_min = min;
        this->m_max = max;
        this->m_ratio = ratio;
        return Error::OK;
    }

    inline float width(float r)
    {
        float w = r * m_ratio;
        if (w < m_min) {
            return m_min;
        }
        else if (w > m_max) {
            return m_max;
        }
        else {
            return w;
        }
    }

protected:
    float m_min;
    float m_max;
    float m_ratio;

};

}
#endif //ANDROID_PAGEFLIP_SHADOWWIDTH_H
