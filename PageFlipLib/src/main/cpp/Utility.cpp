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

#include <android/bitmap.h>
#include "Utility.h"

namespace eschao {

int computeAverageColor(AndroidBitmapInfo &info, GLvoid *data, int count)
{
    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 0;
    int width = info.width;
    int height = info.height;
    int maxWidthPixels = width / 3;
    int maxHeightPixels = height / 3;

    if (count > maxWidthPixels) {
        count = maxWidthPixels;
    }

    if (count > maxHeightPixels) {
        count = maxHeightPixels;
    }

    int right = width - count;
    int bottom = height - count;
    int center_left = right / 2;
    int center_top = bottom / 2;

    for (int i = 0; i < count; ++i) {
        uint32_t* row = (uint32_t*)((char*)data + i * info.stride);
        int color = row[i];
        red += RED(color);
        green += GREEN(color);
        blue += BLUE(color);
        alpha += ALPHA(color);

        row = (uint32_t*)((char*)data + (center_top + i) * info.stride);
        color = row[center_left + i];
        red += RED(color);
        green += GREEN(color);
        blue += BLUE(color);
        alpha += ALPHA(color);

        row = (uint32_t*)((char*)data + i * info.stride);
        color = row[right + i];
        red += RED(color);
        green += GREEN(color);
        blue += BLUE(color);
        alpha += ALPHA(color);

        row = (uint32_t*)((char*)data + (bottom + i) * info.stride);
        color = row[i];
        red += RED(color);
        green += GREEN(color);
        blue += BLUE(color);
        alpha += ALPHA(color);

        row = (uint32_t*)((char*)data + (bottom + i) * info.stride);
        color = row[right + i];
        red += RED(color);
        green += GREEN(color);
        blue += BLUE(color);
        alpha += ALPHA(color);
    }

    count *= 5;
    red /= count;
    green /= count;
    blue /= count;
    alpha /= count;
    return ARGB(alpha, red, green, blue);
}

}

