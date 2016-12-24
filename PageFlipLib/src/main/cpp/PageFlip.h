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

#ifndef ANDROID_PAGEFLIP_PAGE_FLIP_H
#define ANDROID_PAGEFLIP_PAGE_FLIP_H

#include <math.h>
#include "Page.h"
#include "pointf.h"
#include "GLPoint.h"
#include "GLViewRect.h"
#include "Scroller.h"
#include "ShadowWidth.h"
#include "Vertexes.h"
#include "ShadowVertexes.h"
#include "BackOfFoldVertexes.h"
#include "VertexProgram.h"
#include "ShadowVertexProgram.h"
#include "BackOfFoldVertexProgram.h"

namespace eschao {

// default pixels of mesh vertex
static const int kMeshVertexPixels = 10;
static const int kMeshCountThreshold = 20;

// The min page curl angle (5 degree)
static const int kMinPageCurlAngle = 5;
// The max page curl angle (5 degree)
static const int kMaxPageCurlAngle = 65;
static const int kPageCurlAngleDiff = (kMaxPageCurlAngle - kMinPageCurlAngle);
static const float kMinPageCurlRadian = (float) (M_PI * kMinPageCurlAngle /
                                                 180);
static const float kMaxPageCurlRadian = (float) (M_PI * kMaxPageCurlAngle /
                                                 180);
static const float kMinPageCurlTanOfAngle = (float) tan(kMinPageCurlRadian);
static const float kMaxPageCurlTanOfAngle = (float) tan(kMaxPageCurlRadian);
static const float kMaxPageCurlAngleRatio = kMaxPageCurlAngle / 90;
static const float kMaxTanOfForwardFlip = (float) tan(M_PI / 6);
static const float kMaxTanOfBackwardFlip = (float) tan(M_PI / 20);

// width m_ratio of clicking to flip
static const float kWidthRatioOfClickToFlip = 0.5f;

// width m_ratio of triggering restore flip
static const float kWidthOfRatioOfRestoreFlip = 0.4f;

// folder page shadow color buffer size
static const int kFoldTopEdgeShadowVexCount = 22;

// fold edge shadow color
static const float kFoldEdgeShadowStartColor = 0.1f;
static const float kFoldEdgeShadowStartAlpha = 0.25f;
static const float kFoldEdgeShadowEndColor = 0.3f;
static const float kFoldEdgeShadowEndAlpha = 0;

// fold base shadow color
static const float kFoldBaseShadowStartColor = 0.05f;
static const float kFoldBaseShadowStartAlpha = 0.4f;
static const float kFoldBaseShadowEndColor = 0.3f;
static const float kFoldBaseShadowEndAlpha = 0;

enum PageNo {
    FIRST_PAGE = 0,
    SECOND_PAGE,
    PAGES_SIZE
};

enum PageMode {
    SINGLE_PAGE_MODE = 0,
    AUTO_PAGE_MODE,
};

enum PageFlipState {
    BEGIN_FLIP = 0,
    FORWARD_FLIP,
    BACKWARD_FLIP,
    RESTORE_FLIP,
    END_FLIP,
    END_WITH_FORWARD,
    END_WITH_BACKWARD,
    END_WITH_RESTORE,
};

class PageFlip {

public:
    PageFlip();
    ~PageFlip();

    bool enableAutoPage(bool isAuto);
    int onSurfaceCreated();
    void onSurfaceChanged(int width, int height);
    bool onFingerDown(float x, float y);
    bool onFingerMove(float x, float y, bool canForward, bool canBackward);
    bool onFingerUp(float x, float y, int duration,
                    bool canForward, bool canBackward);
    bool canAnimate(float x, float y);
    bool animating();
    void abortAnimating();
    void drawFlipFrame();
    void drawPageFrame();
    int setGradientLightTexture(AndroidBitmapInfo& info, GLvoid* data);

    inline Page* getPage(bool isFirst) {
        return mPages[isFirst ? FIRST_PAGE : SECOND_PAGE];
    }

    inline bool isAutoPageEnabled() {
        return mPageMode == AUTO_PAGE_MODE;
    }

    inline void enableClickToFlip(bool isEnable) {
        mIsClickToFlip = isEnable;
    }

    inline int setWidthRatioOfClickToFlip(float ratio) {
        if (ratio <= 0 || ratio > 0.5f) {
            return gError.set(Error::ERR_INVALID_PARAMETER);
        }

        mWidthRatioOfClickToFlip = ratio;
        return Error::OK;
    }

    inline void setPixelsOfMesh(int pixels) {
        mPixelsOfMesh = pixels > 0 ? mPixelsOfMesh : kMeshVertexPixels;
    }

    inline int setSemiPerimeterRatio(float ratio) {
        if (ratio <= 0 || ratio > 1) {
            return gError.set(Error::ERR_INVALID_PARAMETER);
        }

        mSemiPerimeterRatio = ratio;
        return Error::OK;
    }

    inline int setMaskAlphaOfFold(int alpha) {
        return mBackOfFoldVertexes.setMaskAlpha(alpha);
    }

    inline int setShadowColorOfFoldEdges(float startColor,
                                         float startAlpha,
                                         float endColor,
                                         float endAlpha) {
        return mFoldEdgeShadowVertexes.color.set(startColor, startAlpha,
                                                     endColor, endAlpha);
    }

    inline int setShadowColorOfFoldBase(float startColor,
                                        float startAlpha,
                                        float endColor,
                                        float endAlpha) {
        return mFoldBaseShadowVertexes.color.set(startColor, startAlpha,
                                                     endColor, endAlpha);
    }

    inline int setShadowWidthOfFoldEdges(float min,
                                         float max,
                                         float ratio) {
        return mFoldEdgeShadowWidth.set(min, max, ratio);
    }

    inline int setShadowWidthOfFoldBase(float min,
                                        float max,
                                        float ratio) {
        return mFoldBaseShadowWidth.set(min, max, ratio);
    }

    inline int surfaceWidth() {
        return (int) mViewRect.surfaceWidth;
    }

    inline int surfaceHeight() {
        return (int) mViewRect.surfaceHeight;
    }

    inline PageFlipState flipState() {
        return mFlipState;
    }

    inline int pixelsOfMesh() {
        return mPixelsOfMesh;
    }

    inline bool hasSecondPage() {
        return mPages[SECOND_PAGE] != NULL;
    }

    inline bool hasFirstPage() {
        return mPages[FIRST_PAGE] != NULL;
    }

    inline int setSecondTexture(bool isFirstPage, AndroidBitmapInfo &info,
                                GLvoid *data) {
        return mPages[isFirstPage ? FIRST_PAGE : SECOND_PAGE]->textures
                .setSecondTexture(info, data);
    }

    inline void recycleTextures() {
        if (mPages[FIRST_PAGE]) {
            mPages[FIRST_PAGE]->textures.recycle();
        }

        if (mPages[SECOND_PAGE]) {
            mPages[SECOND_PAGE]->textures.recycle();
        }
    }

    inline bool isAnimating() {
        return !mScroller.isFinished();
    }

    inline bool isStartedFlip() {
        return mFlipState == BACKWARD_FLIP ||
               mFlipState == FORWARD_FLIP ||
               mFlipState == RESTORE_FLIP;
    }

    inline bool isEndedFlip() {
        return mFlipState == END_FLIP ||
               mFlipState == END_WITH_RESTORE ||
               mFlipState == END_WITH_BACKWARD ||
               mFlipState == END_WITH_FORWARD;
    }

private:
    void createPages();
    void computeScrollPointsForClickingFlip(float x,
                                            bool canForward,
                                            bool canBackward,
                                            PointF &start,
                                            PointF &end);
    void computeMaxMeshCount();
    void computeVertexesBuildPage();
    void computeKeyVertexesWhenVertical();
    void computeVertexesWhenVertical();
    void computeKeyVertexesWhenSlope();
    void computeVertexesWhenSlope();
    void computeBackVertex(bool isX, float x0, float y0, float sx0,
                           float sy0, float xfx, float sinA, float cosA,
                           float texX, float texY, float oX, float oY);
    void computeBackVertex(float x0, float y0, float xfx,
                           float sinA, float cosA, float texX, float texY,
                           float oX, float oY);
    void computeFrontVertex(bool isX, float x0, float y0, float xfx,
                            float sinA, float cosA, float baseWCosA,
                            float baseWSinA, float texX, float texY,
                            float oX, float oY);
    void computeFrontVertex(float x0, float y0, float xfx,
                            float sinA, float cosA, float texX, float texY,
                            float oX, float oY);
    void computeBaseShadowLastVertex(float x0, float y0, float xfs,
                                     float sinA, float cosA,
                                     float baseWCosA, float baseWSinA,
                                     float oX, float oY, float dY);
    void computeVertexesOfFoldTopEdgeShadow(float x0, float y0,
                                            float sinA, float cosA,
                                            float sx, float sy);
    void computeMeshCount();
    float computeTanOfCurlAngle(float dy);
    void printInfo();

private:
    // view size
    GLViewRect mViewRect;

    // the pixel size for each mesh
    int mPixelsOfMesh;

    // gradient shadow texture id
    GLuint mGradientLightTexId;

    // touch point and last touch point
    PointF mTouchP;
    // the last touch point (could be deleted?)
    PointF mLastTouchP;
    // the first touch point when finger down on the screen
    PointF mStartTouchP;
    // the middle point between touch point and origin point
    PointF mMiddleP;

    // from 2D perspective, the line will intersect Y axis and X axis that being
    // through middle point and perpendicular to the line which is from touch
    // point to origin point, The point on Y axis is mYFoldP, the mXFoldP is on
    // X axis. The mY{X}FoldP1 is up mY{X}FoldP, The mY{X}FoldP0 is under
    // mY{X}FoldP
    //
    //        <----- Flip
    //                          ^ Y
    //                          |
    //                          + mYFoldP1
    //                        / |
    //                       /  |
    //                      /   |
    //                     /    |
    //                    /     |
    //                   /      |
    //                  /       + mYFoldP
    //    mTouchP      /      / |
    //       .        /      /  |
    //               /      /   |
    //              /      /    |
    //             /      /     |
    //            /   .  /      + mYFoldP0
    //           /      /      /|
    //          /      /      / |
    //         /      /      /  |
    //X <-----+------+------+---+ originP
    //   mXFoldP1 mXFoldP mXFoldP0
    //
    PointF mYFoldP;
    PointF mYFoldP0;
    PointF mYFoldP1;
    PointF mXFoldP;
    PointF mXFoldP0;
    PointF mXFoldP1;

    //            ^ Y
    //   mTouchP  |
    //        +   |
    //         \  |
    //          \ |
    //       A ( \|
    // X <--------+ originP
    //
    // A is angle between X axis and line from mTouchP to originP
    // the max curling angle between line from touchP to originP and X axis
    float mMaxT2OTanA;
    // another max curling angle when finger moving causes the originP change
    // from (x, y) to (x, -y) which means mirror based on Y axis.
    float mMaxT2DTanA;
    // the tan value of current curling angle
    // mKValue = (touchP.y - originP.y) / (touchP.x - originP.x)
    float mKValue;
    // the length of line from mTouchP to originP
    float mLenOfT2O;
    // the cylinder radius
    float mRadius;
    // the perimeter m_ratio of semi-cylinder based on mLenOfTouchOrigin;
    float mSemiPerimeterRatio;
    // Mesh count
    int mMeshCount;

    // edges shadow width of back of fold page
    ShadowWidth mFoldEdgeShadowWidth;
    // base shadow width of front of fold page
    ShadowWidth mFoldBaseShadowWidth;

    // fold page and shadow mVertexes
    Vertexes mFoldFrontVertexes;
    BackOfFoldVertexes mBackOfFoldVertexes;
    ShadowVertexes mFoldEdgeShadowVertexes;
    ShadowVertexes mFoldBaseShadowVertexes;

    // Shader program for openGL drawing
    VertexProgram mVertexProg;
    BackOfFoldVertexProgram mBackOfFoldVertexProg;
    ShadowVertexProgram mShadowVertexProg;

    // is vertical page flip
    bool mIsVertical;
    PageFlipState mFlipState;

    // use for flip animation
    Scroller mScroller;

    // pages and page mode
    // in single page mode, there is only one page in the index 0
    // in double pages mode, there are two pages, the first one is always active
    // page which is receiving finger events, for example: finger down/move/up
    Page* mPages[PAGES_SIZE];
    PageMode mPageMode;

    // is clicking to flip page
    bool mIsClickToFlip;
    // width ration of clicking to flip
    float mWidthRatioOfClickToFlip;
};

}
#endif //ANDROID_PAGEFLIP_PAGE_FLIP_H
