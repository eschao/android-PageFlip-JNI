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

#ifndef ANDROID_PAGEFLIP_UTILITY_H
#define ANDROID_PAGEFLIP_UTILITY_H

#include <GLES2/gl2.h>
#include <android/bitmap.h>
#include <android/log.h>

namespace eschao {

#define RED(clr) (clr & 0xFF)
#define GREEN(clr) ((clr >> 8) & 0xFF)
#define BLUE(clr) ((clr >> 16) & 0xFF)
#define ALPHA(clr) (clr >> 24)
#define ARGB(a, r, g, b) ((a << 24) | (b << 16) | (g << 8) | r)

#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGV(TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)


extern int computeAverageColor(AndroidBitmapInfo &info,
                               GLvoid *data,
                               int count);

}
#endif //ANDROID_PAGEFLIP_UTILITY_H
