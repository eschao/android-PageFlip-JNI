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

#ifndef ANDROID_PAGEFLIP_PAGEFLIP_JNI_H
#define ANDROID_PAGEFLIP_PAGEFLIP_JNI_H

#include <jni.h>

extern "C" {
JNIEXPORT jint JNICALL JNI_GetError(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL JNI_InitLib(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL JNI_ReleaseLib(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_EnableAutoPage(JNIEnv* env,
                                          jobject obj,
                                          jboolean is_auto);
JNIEXPORT jboolean JNICALL JNI_IsAutoPageEnabled(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_EnableClickToFlip(JNIEnv* env,
                                             jobject obj,
                                             jboolean enable);
JNIEXPORT jint JNICALL JNI_SetWidthRatioOfClickToFlip(JNIEnv* env,
                                                      jobject obj,
                                                      jfloat ratio);
JNIEXPORT jint JNICALL JNI_SetPixelsOfMesh(JNIEnv* env,
                                           jobject obj,
                                           jint pixels);
JNIEXPORT jint JNICALL JNI_SetSemiPerimeterRatio(JNIEnv* env,
                                                 jobject obj,
                                                 jfloat ratio);
JNIEXPORT jint JNICALL JNI_SetMaskAlphaOfFold(JNIEnv* env,
                                              jobject obj,
                                              jint alpha);
JNIEXPORT jint JNICALL JNI_SetShadowColorOfFoldEdges(JNIEnv* env,
                                                     jobject obj,
                                                     jfloat s_color,
                                                     jfloat s_alpha,
                                                     jfloat e_color,
                                                     jfloat e_alpha);
JNIEXPORT jint JNICALL JNI_SetShadowColorOfFoldBase(JNIEnv* env,
                                                    jobject obj,
                                                    jfloat s_color,
                                                    jfloat s_alpha,
                                                    jfloat e_color,
                                                    jfloat e_alpha);
JNIEXPORT jint JNICALL JNI_SetShadowWidthOfFoldEdges(JNIEnv* env,
                                                     jobject obj,
                                                     jfloat min,
                                                     jfloat max,
                                                     jfloat ratio);
JNIEXPORT jint JNICALL JNI_SetShadowWidthOfFoldBase(JNIEnv* env,
                                                    jobject obj,
                                                    jfloat min,
                                                    jfloat max,
                                                    jfloat ratio);
JNIEXPORT jint JNICALL JNI_GetPageWidth(JNIEnv* env,
                                        jobject obj,
                                        jboolean is_first_page);
JNIEXPORT jint JNICALL JNI_GetPageHeight(JNIEnv* env,
                                         jobject obj,
                                         jboolean is_first_page);
JNIEXPORT jint JNICALL JNI_GetPixelsOfMesh(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_GetSurfaceWidth(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_GetSurfaceHeight(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_OnSurfaceCreated(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_OnSurfaceChanged(JNIEnv* env,
                                            jobject obj,
                                            jint width,
                                            jint height);
JNIEXPORT jboolean JNICALL JNI_OnFingerDown(JNIEnv* env,
                                            jobject obj,
                                            jfloat x,
                                            jfloat y);
JNIEXPORT jboolean JNICALL JNI_OnFingerMove(JNIEnv* env,
                                            jobject obj,
                                            jfloat x,
                                            jfloat y,
                                            jboolean can_forward,
                                            jboolean can_backward);
JNIEXPORT jboolean JNICALL JNI_OnFingerUp(JNIEnv* env,
                                          jobject obj,
                                          jfloat x,
                                          jfloat y,
                                          jint duration,
                                          jboolean can_forward,
                                          jboolean can_backward);
JNIEXPORT jboolean JNICALL JNI_Animating(JNIEnv* env, jobject obj);
JNIEXPORT jboolean JNICALL JNI_CanAnimate(JNIEnv* env,
                                          jobject obj,
                                          jfloat x,
                                          jfloat y);
JNIEXPORT jboolean JNICALL JNI_IsAnimating(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_AbortAnimating(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_DrawFlipFrame(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_DrawPageFrame(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL JNI_GetFlipState(JNIEnv* env, jobject obj);
JNIEXPORT jboolean JNICALL JNI_HasFirstPage(JNIEnv* env, jobject obj);
JNIEXPORT jboolean JNICALL JNI_HasSecondPage(JNIEnv* env, jobject obj);
JNIEXPORT jboolean JNICALL JNI_IsLeftPage(JNIEnv* env,
                                          jobject obj,
                                          jboolean is_first_page);
JNIEXPORT jboolean JNICALL JNI_IsRightPage(JNIEnv* env,
                                           jobject obj,
                                           jboolean is_first_page);
JNIEXPORT jboolean JNICALL JNI_IsFirstTextureSet(JNIEnv* env,
                                                 jobject obj,
                                                 jboolean is_first_page);
JNIEXPORT jboolean JNICALL JNI_IsSecondTextureSet(JNIEnv* env,
                                                  jobject obj,
                                                  jboolean is_first_page);
JNIEXPORT jboolean JNICALL JNI_IsBackTextureSet(JNIEnv* env,
                                                jobject obj,
                                                jboolean is_first_page);
JNIEXPORT jint JNICALL JNI_SetFirstTexture(JNIEnv* env,
                                           jobject obj,
                                           jboolean is_first_page,
                                           jobject jobj);
JNIEXPORT jint JNICALL JNI_SetSecondTexture(JNIEnv* env,
                                            jobject obj,
                                            jboolean is_first_page,
                                            jobject jobj);
JNIEXPORT jint JNICALL JNI_SetBackTexture(JNIEnv* env,
                                          jobject obj,
                                          jboolean is_first_page,
                                          jobject jobj);
JNIEXPORT jint JNICALL JNI_SetGradientLightTexture(JNIEnv* env,
                                                   jobject obj,
                                                   jobject jobj);
JNIEXPORT jint JNICALL JNI_SetFirstTextureWithSecond(JNIEnv* env,
                                                     jobject obj);

JNIEXPORT jint JNICALL JNI_SetSecondTextureWithFirst(JNIEnv* env,
                                                     jobject obj);
JNIEXPORT jint JNICALL JNI_SwapSecondTexturesWithFirst(JNIEnv* env,
                                                       jobject obj);
JNIEXPORT jint JNICALL JNI_RecycleTextures(JNIEnv* env, jobject obj);
}

#endif //ANDROID_PAGEFLIP_PAGEFLIP_JNI_H
