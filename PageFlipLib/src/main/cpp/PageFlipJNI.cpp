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

#include <android/log.h>
#include <android/bitmap.h>
#include "PageFlip.h"
#include "PageFlipJNI.h"

using namespace eschao;

static const char* TAG          = "PageFlipJNI";
static PageFlip *gPageFlip      = NULL;
static const char *gClassName   =
        "com/eschao/android/widget/jni/pageflip/PageFlipLib";

static JNINativeMethod gMethodsTable[] = {
        { "getError", "()I", (void *)JNI_GetError },
        { "init", "()Z", (void *)JNI_InitLib },
        { "release", "()Z", (void *)JNI_ReleaseLib },
        { "enableAutoPage", "(Z)I", (void *)JNI_EnableAutoPage },
        { "isAutoPageEnabled", "()Z", (void *)JNI_IsAutoPageEnabled },
        { "enableClickToFlip", "(Z)I", (void *)JNI_EnableClickToFlip },
        { "setWidthRatioOfClickToFlip", "(F)I",
          (void *)JNI_SetWidthRatioOfClickToFlip },
        { "setPixelsOfMesh", "(I)I", (void *)JNI_SetPixelsOfMesh },
        { "setSemiPerimeterRatio", "(F)I",
          (void *)JNI_SetSemiPerimeterRatio },
        { "setMaskAlphaOfFold", "(I)I", (void *)JNI_SetMaskAlphaOfFold },
        { "setShadowColorOfFoldEdges", "(FFFF)I",
          (void *)JNI_SetShadowColorOfFoldEdges },
        { "setShadowColorOfFoldBase", "(FFFF)I",
          (void *)JNI_SetShadowColorOfFoldBase },
        { "setShadowWidthOfFoldEdges", "(FFF)I",
          (void *)JNI_SetShadowWidthOfFoldEdges },
        { "setShadowWidthOfFoldBase", "(FFF)I",
          (void *)JNI_SetShadowWidthOfFoldBase },
        { "getPixelsOfMesh", "()I", (void *)JNI_GetPixelsOfMesh },
        { "getSurfaceWidth", "()I", (void *)JNI_GetSurfaceWidth },
        { "getSurfaceHeight", "()I", (void *)JNI_GetSurfaceHeight },
        { "onSurfaceCreated", "()I", (void *)JNI_OnSurfaceCreated },
        { "onSurfaceChanged", "(II)I", (void *)JNI_OnSurfaceChanged },
        { "onFingerDown", "(FF)Z", (void *)JNI_OnFingerDown },
        { "animating", "()Z", (void *)JNI_Animating },
        { "canAnimate", "(FF)Z", (void *)JNI_CanAnimate },
        { "isAnimating", "()Z", (void *)JNI_IsAnimating },
        { "abortAnimating", "()I", (void *)JNI_AbortAnimating },
        { "drawFlipFrame", "()I", (void *)JNI_DrawFlipFrame },
        { "drawPageFrame", "()I", (void *)JNI_DrawPageFrame },
        { "getFlipState", "()I", (void *)JNI_GetFlipState },
        { "hasFirstPage", "()Z", (void *)JNI_HasFirstPage },
        { "hasSecondPage", "()Z", (void *)JNI_HasSecondPage },
        { "isFirstTextureSet", "(Z)Z", (void *)JNI_IsFirstTextureSet },
        { "isSecondTextureSet", "(Z)Z", (void *)JNI_IsSecondTextureSet },
        { "isBackTextureSet", "(Z)Z", (void *)JNI_IsBackTextureSet },
        { "setFirstTexture", "(ZLandroid/graphics/Bitmap;)I",
          (void *)JNI_SetFirstTexture },
        { "setSecondTexture", "(ZLandroid/graphics/Bitmap;)I",
          (void *)JNI_SetSecondTexture },
        { "setBackTexture", "(ZLandroid/graphics/Bitmap;)I",
          (void *)JNI_SetBackTexture },
        { "setGradientLightTexture", "(Landroid/graphics/Bitmap;)I",
          (void *)JNI_SetGradientLightTexture },
        { "setFirstTextureWithSecond", "()I",
           (void *)JNI_SetFirstTextureWithSecond },
        { "setSecondTextureWithFirst", "()I",
           (void *)JNI_SetSecondTextureWithFirst },
        { "swapSecondTexturesWithFirst", "()I",
           (void *)JNI_SwapSecondTexturesWithFirst},
        { "recycleTextures", "()I", (void *)JNI_RecycleTextures},
        { "onFingerMove", "(FFZZ)Z", (void *)JNI_OnFingerMove },
        { "onFingerUp", "(FFIZZ)Z", (void *)JNI_OnFingerUp },
        { "getPageWidth", "(Z)I", (void *)JNI_GetPageWidth },
        { "getPageHeight", "(Z)I", (void *)JNI_GetPageHeight },
        { "isLeftPage", "(Z)Z", (void *)JNI_IsLeftPage },
        { "isRightPage", "(Z)Z", (void *)JNI_IsRightPage },
};

static bool registerNatives(JNIEnv* env) {
    jclass cls = env->FindClass(gClassName);
    if (cls == NULL) {
        return JNI_FALSE;
    }

    int size = sizeof(gMethodsTable) / sizeof(JNINativeMethod);
    jint ret = env->RegisterNatives(cls, gMethodsTable, size);
    if (ret < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void *reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    if (!registerNatives(env)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT jint JNICALL JNI_GetError(JNIEnv* env, jobject obj) {
    return (jint)gError.code();
}

JNIEXPORT void JNICALL JNI_InitLib(JNIEnv* env, jobject obj) {
   if (gPageFlip == NULL) {
       LOGD(TAG, "Init PageFlip Object...");
       gPageFlip = new PageFlip();
   }
}

JNIEXPORT void JNICALL JNI_ReleaseLib(JNIEnv* env, jobject obj) {
    if (gPageFlip) {
        LOGD(TAG, "Release PageFlip Object...");
        delete gPageFlip;
        gPageFlip = NULL;
    }
}

JNIEXPORT jint JNICALL JNI_EnableAutoPage(JNIEnv* env,
                                          jobject obj,
                                          jboolean is_auto) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->enableAutoPage(is_auto);
        return JNI_OK;
    }
    else {
        LOGE("JNI_EnableAutoPage",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jboolean JNICALL JNI_IsAutoPageEnabled(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean) gPageFlip->isAutoPageEnabled();
    }
    else {
        LOGE("JNI_IsAutoPageEnabled",
             "PageFlip object is null, please call init() first!");
    }

    gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    return JNI_FALSE;
}

JNIEXPORT jint JNICALL JNI_EnableClickToFlip(JNIEnv* env,
                                             jobject obj,
                                             jboolean enable) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->enableClickToFlip(enable);
        return Error::OK;
    }
    else {
        LOGE("JNI_EnableClickToFlip",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetWidthRatioOfClickToFlip(JNIEnv* env,
                                                      jobject obj,
                                                      jfloat ratio) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setWidthRatioOfClickToFlip(ratio);
    }
    else {
        LOGE("JNI_SetWidthRatioOfClickToFlip",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetPixelsOfMesh(JNIEnv* env,
                                           jobject obj,
                                           jint pixels) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->setPixelsOfMesh(pixels);
        return Error::OK;
    }
    else {
        LOGE("JNI_SetPixelsOfMesh",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetSemiPerimeterRatio(JNIEnv* env,
                                                 jobject obj,
                                                 jfloat ratio) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setSemiPerimeterRatio(ratio);
    }
    else {
        LOGE("JNI_SetSemiPerimeterRatio",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetMaskAlphaOfFold(JNIEnv* env,
                                              jobject obj,
                                              jint alpha) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setMaskAlphaOfFold(alpha);
    }
    else {
        LOGE("JNI_SetMaskAlphaOfFold",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetShadowColorOfFoldEdges(JNIEnv* env,
                                                     jobject obj,
                                                     jfloat s_color,
                                                     jfloat s_alpha,
                                                     jfloat e_color,
                                                     jfloat e_alpha) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setShadowColorOfFoldEdges(s_color, s_alpha,
                                                     e_color, e_alpha);
    }
    else {
        LOGE("JNI_SetShadowColorOfFoldEdges",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetShadowColorOfFoldBase(JNIEnv* env,
                                                    jobject obj,
                                                    jfloat s_color,
                                                    jfloat s_alpha,
                                                    jfloat e_color,
                                                    jfloat e_alpha) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setShadowColorOfFoldBase(s_color, s_alpha,
                                                    e_color, e_alpha);
    }
    else {
        LOGE("JNI_SetShadowColorOfFoldBase",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetShadowWidthOfFoldEdges(JNIEnv* env,
                                                     jobject obj,
                                                     jfloat min,
                                                     jfloat max,
                                                     jfloat ratio) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setShadowWidthOfFoldEdges(min, max, ratio);
    }
    else {
        LOGE("JNI_SetShadowWidthOfFoldEdges",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_SetShadowWidthOfFoldBase(JNIEnv* env,
                                                    jobject obj,
                                                    jfloat min,
                                                    jfloat max,
                                                    jfloat ratio) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->setShadowWidthOfFoldBase(min, max, ratio);
    }
    else {
        LOGE("JNI_SetShadowWidthOfFoldBase",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_GetPixelsOfMesh(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jint) gPageFlip->pixelsOfMesh();
    }
    else {
        LOGE("JNI_GetPixelsOfMesh",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_GetSurfaceWidth(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->surfaceWidth();
    }
    else {
        LOGE("JNI_GetSurfaceWidth",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_GetSurfaceHeight(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jint)gPageFlip->surfaceHeight();
    }
    else {
        LOGE("JNI_GetSurfaceHeight",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_GetPageWidth(JNIEnv* env,
                                        jobject obj,
                                        jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        return (jint)page->width();
    }
    else {
        LOGE("JNI_GetPageWidth",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}


JNIEXPORT jint JNICALL JNI_GetPageHeight(JNIEnv* env,
                                         jobject obj,
                                         jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        return (jint)page->height();
    }
    else {
        LOGE("jni_get_height_of_page1",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jboolean JNICALL JNI_IsLeftPage(JNIEnv* env,
                                          jobject obj,
                                          jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            gError.set(Error::ERR_NULL_PAGE);
            return JNI_FALSE;
        }

        return (jboolean)page->isLeftPage();
    }
    else {
        LOGE("JNI_IsLeftPage",
             "PageFlip object is null, please call init() first!");
    }

    gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_IsRightPage(JNIEnv* env,
                                           jobject obj,
                                           jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            gError.set(Error::ERR_NULL_PAGE);
            return JNI_FALSE;
        }

        return (jboolean)page->isRightPage();
    }
    else {
        LOGE("JNI_IsRightPage",
             "PageFlip object is null, please call init() first!");
    }

    gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    return JNI_FALSE;
} 

JNIEXPORT jint JNICALL JNI_OnSurfaceCreated(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->onSurfaceCreated();
        return Error::OK;
    }
    else {
        LOGE("JNI_OnSurfaceCreated",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_OnSurfaceChanged(JNIEnv* env,
                                            jobject obj,
                                            jint width,
                                            jint height) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->onSurfaceChanged(width, height);
        return Error::OK;
    }
    else {
        LOGE("JNI_OnSurfaceChanged",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jboolean JNICALL JNI_OnFingerDown(JNIEnv* env,
                                            jobject obj,
                                            jfloat x,
                                            jfloat y) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean) gPageFlip->onFingerDown(x, y);
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_OnFingerDown",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_OnFingerMove(JNIEnv* env,
                                            jobject obj,
                                            jfloat x,
                                            jfloat y,
                                            jboolean can_forward,
                                            jboolean can_backward) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->onFingerMove(x, y,
                                                   can_forward, can_backward);
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_OnFingerMove",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_OnFingerUp(JNIEnv* env,
                                          jobject obj,
                                          jfloat x,
                                          jfloat y,
                                          jint duration,
                                          jboolean can_forward,
                                          jboolean can_backward) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->onFingerUp(x, y, duration,
                                               can_forward, can_backward);
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_OnFingerUp",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_Animating(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->animating();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_Animating",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_CanAnimate(JNIEnv* env,
                                          jobject obj,
                                          jfloat x,
                                          jfloat y) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->canAnimate(x, y);
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_CanAnimate",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_IsAnimating(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->isAnimating();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_IsAnimating",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jint JNICALL JNI_AbortAnimating(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->abortAnimating();
        return Error::OK;
    }
    else {
        LOGE("JNI_AbortAnimating",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_DrawFlipFrame(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->drawFlipFrame();
        return Error::OK;
    }
    else {
        LOGE("JNI_DrawFlipFrame",
             "PageFlip object is null, please call init() first!");
    }
    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_DrawPageFrame(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->drawPageFrame();
        return Error::OK;
    }
    else {
        LOGE("JNI_DrawPageFrame",
             "PageFlip object is null, please call init() first!");
    }
    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jint JNICALL JNI_GetFlipState(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return gPageFlip->flipState();
    }
    else {
        LOGE("JNI_GetFlipState",
             "PageFlip object is null, please call init() first!");
    }

    return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
}

JNIEXPORT jboolean JNICALL JNI_HasFirstPage(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->hasFirstPage();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_HasFirstPage",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_HasSecondPage(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        return (jboolean)gPageFlip->hasSecondPage();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_HasSecondPage",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_IsFirstTextureSet(JNIEnv* env,
                                                 jobject obj,
                                                 jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            gError.set(Error::ERR_NULL_PAGE);
            return JNI_FALSE;
        }

        return (jboolean) page->textures.isFirstTextureSet();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_IsFirstTextureSet",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_IsSecondTextureSet(JNIEnv* env,
                                                  jobject obj,
                                                  jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            gError.set(Error::ERR_NULL_PAGE);
            return JNI_FALSE;
        }

        return (jboolean)page->textures.isSecondTextureSet();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_IsSecondTextureSet",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL JNI_IsBackTextureSet(JNIEnv* env,
                                                jobject obj,
                                                jboolean is_first_page) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            gError.set(Error::ERR_NULL_PAGE);
            return JNI_FALSE;
        }

        return (jboolean)page->textures.isBackTextureSet();
    }
    else {
        gError.set(Error::ERR_PAGE_FLIP_UNINIT);
        LOGE("JNI_IsBackTextureSet",
             "PageFlip object is null, please call init() first!");
    }

    return JNI_FALSE;
}

JNIEXPORT jint JNICALL JNI_SetFirstTexture(JNIEnv* env,
                                           jobject obj,
                                           jboolean is_first_page,
                                           jobject bitmap) {
    gError.reset();
    if (bitmap == NULL) {
        LOGE("JNI_SetFirstTexture",
             "Can't set first texture with null Bitmap object!");
        return gError.set(Error::ERR_NULL_PARAMETER);
    }
    else if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        int ret;
        AndroidBitmapInfo info;
        GLvoid *data;
        if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
            return Error::ERR_GET_BITMAP_INFO;
        }

        if ((ret = AndroidBitmap_lockPixels(env, bitmap, &data)) < 0) {
            return Error::ERR_GET_BITMAP_DATA;
        }

        ret = page->textures.setFirstTexture(info, data);
        AndroidBitmap_unlockPixels(env, bitmap);
        return ret;
    }
    else {
        LOGE("JNI_SetFirstTexture",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_SetSecondTexture(JNIEnv* env,
                                            jobject obj,
                                            jboolean is_first_page,
                                            jobject bitmap) {
    gError.reset();
    if (bitmap == NULL) {
        LOGE("JNI_SetSecondTexture",
             "Can't set second texture with null Bitmap object!");
        return gError.set(Error::ERR_NULL_PARAMETER);
    }
    else if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        int ret;
        AndroidBitmapInfo info;
        GLvoid *data;
        if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
            return Error::ERR_GET_BITMAP_INFO;
        }

        if ((ret = AndroidBitmap_lockPixels(env, bitmap, &data)) < 0) {
            return Error::ERR_GET_BITMAP_DATA;
        }

        ret = page->textures.setSecondTexture(info, data);
        AndroidBitmap_unlockPixels(env, bitmap);
        return ret;
    }
    else {
        LOGE("JNI_SetSecondTexture",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_SetBackTexture(JNIEnv* env,
                                          jobject obj,
                                          jboolean is_first_page,
                                          jobject bitmap) {
    gError.reset();
    AndroidBitmapInfo info;
    if (bitmap == NULL) {
        return gPageFlip->setSecondTexture(is_first_page, info, NULL);
    }
    else if (gPageFlip) {
        Page* page = gPageFlip->getPage(is_first_page);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        int ret;
        GLvoid *data;
        if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
            return Error::ERR_GET_BITMAP_INFO;
        }

        if ((ret = AndroidBitmap_lockPixels(env, bitmap, &data)) < 0) {
            return Error::ERR_GET_BITMAP_DATA;
        }

        ret = page->textures.setBackTexture(info, data);
        AndroidBitmap_unlockPixels(env, bitmap);
        return ret;
    }
    else {
        LOGE("JNI_SetBackTexture",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_SetGradientLightTexture(JNIEnv* env,
                                                   jobject obj,
                                                   jobject bitmap) {
    gError.reset();
    AndroidBitmapInfo info;
    if (bitmap == NULL) {
        LOGE("JNI_SetGradientLightTexture",
             "Can't set gradient light texture with null Bitmap object!");
        return gError.set(Error::ERR_NULL_PARAMETER);
    }
    else if (gPageFlip) {
        int ret;
        GLvoid *data;
        if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
            return Error::ERR_GET_BITMAP_INFO;
        }

        if ((ret = AndroidBitmap_lockPixels(env, bitmap, &data)) < 0) {
            return Error::ERR_GET_BITMAP_DATA;
        }

        ret = gPageFlip->setGradientLightTexture(info, data);
        AndroidBitmap_unlockPixels(env, bitmap);
        return ret;
    }
    else {
        LOGE("JNI_SetGradientLightTexture",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_SetFirstTextureWithSecond(JNIEnv* env,
                                                     jobject obj) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(true);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        page->textures.setFirstTextureWithSecond();
        return Error::OK;
    }
    else {
        LOGE("JNI_SetFirstTextureWithSecond",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_SetSecondTextureWithFirst(JNIEnv* env,
                                                     jobject obj) {
    gError.reset();
    if (gPageFlip) {
        Page* page = gPageFlip->getPage(true);
        if (page == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        page->textures.setSecondTextureWithFirst();
        return Error::OK;
    }
    else {
        LOGE("JNI_SetSecondTextureWithFirst",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_SwapSecondTexturesWithFirst(JNIEnv* env,
                                                       jobject obj) {
    gError.reset();
    if (gPageFlip) {
        Page* first = gPageFlip->getPage(true);
        Page* second = gPageFlip->getPage(false);
        if (first == NULL || second == NULL) {
            return gError.set(Error::ERR_NULL_PAGE);
        }

        second->textures.swapTexturesWith(first->textures);
        return Error::OK;
    }
    else {
        LOGE("JNI_SwapSecondTexturesWithFirst",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}

JNIEXPORT jint JNICALL JNI_RecycleTextures(JNIEnv* env, jobject obj) {
    gError.reset();
    if (gPageFlip) {
        gPageFlip->recycleTextures();
        return Error::OK;
    }
    else {
        LOGE("JNI_RecycleTextures",
             "PageFlip object is null, please call init() first!");
        return gError.set(Error::ERR_PAGE_FLIP_UNINIT);
    }
}
