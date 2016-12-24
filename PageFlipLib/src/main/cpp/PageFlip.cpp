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

#include <math.h>
#include <GLES2/gl2.h>
#include <algorithm>
#include "Page.h"
#include "PageFlip.h"

namespace eschao {

static auto TAG = "PageFlip";

PageFlip::PageFlip()
        : mFlipState(END_FLIP),
          mIsVertical(false),
          mPixelsOfMesh(kMeshVertexPixels),
          mSemiPerimeterRatio(0.8f),
          mIsClickToFlip(true),
          mWidthRatioOfClickToFlip(kWidthRatioOfClickToFlip),
          mPageMode(SINGLE_PAGE_MODE),
          mFoldEdgeShadowWidth(5, 30, 0.25f),
          mFoldBaseShadowWidth(2, 40, 0.4f),
          mFoldEdgeShadowVertexes(kFoldTopEdgeShadowVexCount,
                                  kFoldEdgeShadowStartColor,
                                  kFoldEdgeShadowStartAlpha,
                                  kFoldEdgeShadowEndColor,
                                  kFoldEdgeShadowEndAlpha),
          mFoldBaseShadowVertexes(0,
                                  kFoldBaseShadowStartColor,
                                  kFoldBaseShadowStartAlpha,
                                  kFoldBaseShadowEndColor,
                                  kFoldBaseShadowEndAlpha) {
    mPages[FIRST_PAGE] = NULL;
    mPages[SECOND_PAGE] = NULL;
}

PageFlip::~PageFlip() {
    if (mPages[FIRST_PAGE]) {
        delete mPages[FIRST_PAGE];
    }

    if (mPages[SECOND_PAGE]) {
        delete mPages[SECOND_PAGE];
    }
}

bool PageFlip::enableAutoPage(bool isAuto) {
    PageMode newMode = isAuto ? AUTO_PAGE_MODE : SINGLE_PAGE_MODE;
    if (mPageMode != newMode) {
        mPageMode = newMode;

        if ((newMode == AUTO_PAGE_MODE &&
             mViewRect.surfaceWidth > mViewRect.surfaceHeight &&
             mPages[SECOND_PAGE] == NULL) ||
            (newMode == SINGLE_PAGE_MODE && mPages[SECOND_PAGE] != NULL)) {
            createPages();
            return true;
        }
    }

    return false;
}

int PageFlip::onSurfaceCreated() {
    glClearColor(0, 0, 0, 1);
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);

    mFlipState = END_FLIP;
    mIsVertical = false;

    if (mVertexProg.init() != Error::OK ||
        mShadowVertexProg.init() != Error::OK ||
        mBackOfFoldVertexProg.init() != Error::OK) {
        mVertexProg.clean();
        mShadowVertexProg.clean();
        mBackOfFoldVertexProg.clean();
        return gError.code();
    }

    return Error::OK;
}

void PageFlip::onSurfaceChanged(int width, int height) {
    mViewRect.set(width, height);
    glViewport(0, 0, width, height);
    mVertexProg.initMatrix(-mViewRect.halfWidth,
                             mViewRect.halfWidth,
                             -mViewRect.halfHeight,
                             mViewRect.halfHeight);
    computeMaxMeshCount();
    createPages();
}

void PageFlip::createPages() {
    if (mPages[FIRST_PAGE]) {
        mPages[FIRST_PAGE]->textures.recycleAll();
        delete mPages[FIRST_PAGE];
        mPages[FIRST_PAGE] = NULL;
    }

    if (mPages[SECOND_PAGE]) {
        mPages[SECOND_PAGE]->textures.recycleAll();
        delete mPages[SECOND_PAGE];
        mPages[SECOND_PAGE] = NULL;
    }

    if (mPageMode == AUTO_PAGE_MODE &&
        mViewRect.surfaceWidth > mViewRect.surfaceHeight) {
        mPages[FIRST_PAGE] = new Page(mViewRect.left, 0, mViewRect.top,
                                       mViewRect.bottom);
        mPages[SECOND_PAGE] = new Page(0, mViewRect.right, mViewRect.top,
                                        mViewRect.bottom);
    }
    else {
        mPages[FIRST_PAGE] = new Page(mViewRect.left, mViewRect.right,
                                       mViewRect.top, mViewRect.bottom);
    }
}

bool PageFlip::onFingerDown(float x, float y) {
    x = mViewRect.toOpenGLX(x);
    y = mViewRect.toOpenGLY(y);

    bool isContained = mPages[FIRST_PAGE]->contains(x, y);
    if (!isContained && mPages[SECOND_PAGE] &&
        mPages[SECOND_PAGE]->contains(x, y)) {
        isContained = true;
        std::swap(mPages[FIRST_PAGE], mPages[SECOND_PAGE]);
    }

    if (isContained) {
        mMaxT2OTanA = 0;
        mMaxT2DTanA = 0;
        mLastTouchP.set(x, y);
        mStartTouchP.set(x, y);
        mTouchP.set(x, y);
        mFlipState = BEGIN_FLIP;
    }

    return isContained;
}

bool PageFlip::onFingerMove(float x, float y, bool canForward, bool canBackward)
{
    x = mViewRect.toOpenGLX(x);
    y = mViewRect.toOpenGLY(y);

    float dy = (y - mStartTouchP.y);
    float dx = (x - mStartTouchP.x);

    Page& page = *mPages[FIRST_PAGE];
    const GLPoint& originP = page.mOriginP;
    const GLPoint& diagonalP = page.mDiagonalP;

    if (mFlipState == BEGIN_FLIP &&
        fabs(dx) > mViewRect.halfWidth * 0.05f) {
        page.setOriginDiagonalPoints(mPages[SECOND_PAGE] != NULL, dy > 0);

        // compute max degree between X axis and line from TouchP to OriginP
        // and max degree between X axis and line from TouchP to
        // (OriginP.x, DiagonalP.Y)
        float y2o = fabs(mStartTouchP.y - originP.y);
        float y2d = fabs(mStartTouchP.y - diagonalP.y);
        mMaxT2OTanA = computeTanOfCurlAngle(y2o);
        mMaxT2DTanA = computeTanOfCurlAngle(y2d);

        // moving at the top and bottom screen have different tan value of
        // angle
        if ((originP.y < 0 && page.mRight > 0) ||
            (originP.y > 0 && page.mRight <= 0)) {
            mMaxT2OTanA = -mMaxT2OTanA;
        }
        else {
            mMaxT2DTanA = -mMaxT2DTanA;
        }

        // determine if it is moving mBackward or mForward
        if (mPages[SECOND_PAGE] == NULL && dx > 0 && canBackward) {
            mStartTouchP.x = originP.x;
            dx = (x - mStartTouchP.x);
            mFlipState = BACKWARD_FLIP;
            mPages[FIRST_PAGE]->textures.setSecondTextureWithFirst();
        }
        else if (canForward &&
                ((dx < 0 && originP.x > 0) || (dx > 0 && originP.x < 0))) {
            mFlipState = FORWARD_FLIP;
        }
    }

    // in moving, compute the TouchXY
    if (mFlipState == FORWARD_FLIP ||
        mFlipState == BACKWARD_FLIP ||
        mFlipState == RESTORE_FLIP) {

        // check if page is flipping vertically
        mIsVertical = fabs(dy) <= 1;

        // multiply a factor to make sure the touch point is always head of
        // finger point
        if (FORWARD_FLIP == mFlipState) {
            dx *= 1.2f;
        }
        else {
            dx *= 1.1f;
        }

        // moving direction is changed:
        // 1. invert max curling angle
        // 2. invert Y of original point and diagonal point
        if ((dy < 0 && originP.y < 0) || (dy > 0 && originP.y > 0)) {
            std::swap(mMaxT2OTanA, mMaxT2DTanA);
            page.invertYOfOriginP();
        }

        // compute new TouchP.y
        float maxY = dx * mMaxT2OTanA;
        if (fabs(dy) > fabs(maxY)) {
            dy = maxY;
        }

        // check if XFoldX1 is outside page width, if yes, recompute new
        // TouchP.y to assure the XFoldX1 is in page width
        float t2oK = dy / dx;
        float xTouchX = dx + dy * t2oK;
        float xRatio = (1 + mSemiPerimeterRatio) * 0.5f;
        float xFoldX1 = xRatio * xTouchX;
        if (fabs(xFoldX1) + 2 >= page.mWidth) {
            float dy2 = ((diagonalP.x - originP.x) / xRatio - dx) * dx;
            // ignore current moving if we can't get a valid dy, for example
            // , in double pages mode, when finger is moving from the one
            // page to another page, the dy2 is negative and should be
            // ignored
            if (dy2 < 0) {
                return false;
            }

            double t = sqrt(dy2);
            if (originP.y > 0) {
                t = -t;
                dy = (int)ceil(t);
            }
            else {
                dy = (int)floor(t);
            }
        }

        // set touchP(x, y) and middleP(x, y)
        mLastTouchP.set(x, y);
        mTouchP.set(dx + originP.x, dy + originP.y);
        mMiddleP.x = (mTouchP.x + originP.x) * 0.5f;
        mMiddleP.y = (mTouchP.y + originP.y) * 0.5f;

        // continue to compute points to drawing flip
        computeVertexesBuildPage();
        return true;
    }

    return false;
}

bool PageFlip::onFingerUp(float x, float y, int duration,
                          bool canForward, bool canBackward) {
    x = mViewRect.toOpenGLX(x);
    y = mViewRect.toOpenGLY(y);

    Page& page = *mPages[FIRST_PAGE];
    const GLPoint& originP = page.mOriginP;
    const GLPoint& diagonalP = page.mDiagonalP;
    PointF start(mTouchP);
    PointF end(0, 0);

    // mForward flipping
    if (mFlipState == FORWARD_FLIP) {
        // can't going mForward, restore current page
        if (page.isXInRange(x, kWidthOfRatioOfRestoreFlip)) {
            end.x = originP.x;
            mFlipState = RESTORE_FLIP;
        }
        else if (mPages[SECOND_PAGE] && originP.x < 0) {
            end.x = diagonalP.x + page.mWidth;
        }
        else {
            end.x = diagonalP.x - page.mWidth;
        }
        end.y = originP.y;
    }
    // mBackward flipping
    else if (mFlipState == BACKWARD_FLIP) {
        // if not over middle x, change from mBackward to mForward to restore
        if (!page.isXInRange(x, 0.5f)) {
            mFlipState = FORWARD_FLIP;
            end.set(diagonalP.x - page.mWidth, originP.y);
        }
        else {
            mMaxT2OTanA = (mTouchP.y - originP.y) / (mTouchP.x - originP.x);
            end.set((int) originP.x, (int) originP.y);
        }
    }
    // ready to flip
    else if (mFlipState == BEGIN_FLIP) {
        mIsVertical = false;
        mFlipState = END_FLIP;
        page.setOriginDiagonalPoints(mPages[SECOND_PAGE] != NULL, -y > 0);

        // if enable clicking to flip, compute scroller points for animation
        if (mIsClickToFlip && fabs(x - mStartTouchP.x) < 2) {
            computeScrollPointsForClickingFlip(x, canForward, canBackward,
                                               start, end);
        }
    }

    // start scroller for animating
    if (mFlipState == FORWARD_FLIP ||
        mFlipState == BACKWARD_FLIP ||
        mFlipState == RESTORE_FLIP) {
        mScroller.startScroll(start.x, start.y, end.x - start.x,
                              end.y - start.y, duration);
        return true;
    }

    return false;
}

bool PageFlip::canAnimate(float x, float y) {
    return (mFlipState == FORWARD_FLIP &&
            !mPages[FIRST_PAGE]->contains(mViewRect.toOpenGLX(x),
                                           mViewRect.toOpenGLY(y)));
}

void PageFlip::computeScrollPointsForClickingFlip(float x,
                                                  bool canForward,
                                                  bool canBackward,
                                                  PointF &start,
                                                  PointF &end) {
    Page& page = *mPages[FIRST_PAGE];
    GLPoint& originP = page.mOriginP;
    GLPoint& diagonalP = page.mDiagonalP;
    bool hasSecondPage = mPages[SECOND_PAGE] != NULL;

    // mForward and mBackward flip have different degree
    float tanOfForwardAngle = kMaxTanOfForwardFlip;
    float tanOfBackwardAngle = kMaxTanOfBackwardFlip;
    if ((originP.y < 0 && originP.x > 0) ||
        (originP.y > 0 && originP.x < 0)) {
        tanOfForwardAngle = -tanOfForwardAngle;
        tanOfBackwardAngle = -tanOfBackwardAngle;
    }

    // mBackward flip
    if (!hasSecondPage &&
        x < diagonalP.x + page.mWidth * mWidthRatioOfClickToFlip &&
        canBackward) {
        mFlipState = BACKWARD_FLIP;
        mKValue = tanOfBackwardAngle;
        page.textures.setSecondTextureWithFirst();
        start.set(diagonalP.x, (originP.y + (start.x - originP.x) * mKValue));
        end.set(originP.x - 5, originP.y);
    }
    // mForward flip
    else if (canForward && page.isXInRange(x, kWidthRatioOfClickToFlip)) {
        mFlipState = FORWARD_FLIP;
        mKValue = tanOfForwardAngle;

        // compute start.x
        if (originP.x < 0) {
            start.x = (int)(originP.x + page.mWidth * 0.25f);
        }
        else {
            start.x = (int)(originP.x - page.mWidth * 0.25f);
        }

        // compute start.y
        start.y = originP.y + (start.x - originP.x) * mKValue;

        // compute end.x
        // left page in double page mode
        if (hasSecondPage && originP.x < 0) {
            end.x = diagonalP.x + page.mWidth;
        }
        // right page in double page mode
        else {
            end.x = diagonalP.x - page.mWidth;
        }
        end.y = originP.y;
    }
}

/**
 * Compute animating and check if it can continue
 *
 * @return true animating is continue or it is stopped
 */
bool PageFlip::animating() {
    Page& page = *mPages[FIRST_PAGE];
    const GLPoint& originP = page.mOriginP;
    const GLPoint& diagonalP = page.mDiagonalP;

    // is to end animating?
    bool isAnimating = !mScroller.isFinished();
    if (isAnimating) {
        // get new (x, y)
        mScroller.computeScrollOffset();
        mTouchP.set(mScroller.currX(), mScroller.currY());

        // for mBackward and restore flip, compute x to check if it can
        // continue to flip
        if (mFlipState == BACKWARD_FLIP ||
            mFlipState == RESTORE_FLIP) {
            mTouchP.y = (mTouchP.x - originP.x) * mKValue + originP.y;
            isAnimating = fabs(mTouchP.x - originP.x) > 10;
        }
        // check if flip is vertical
        else {
            mIsVertical = fabs(mTouchP.y - originP.y) < 1;
        }

        // compute middle point
        mMiddleP.set((mTouchP.x + originP.x) * 0.5f,
                     (mTouchP.y + originP.y) * 0.5f);

        // compute key points
        if (mIsVertical) {
            computeKeyVertexesWhenVertical();
        }
        else {
            computeKeyVertexesWhenSlope();
        }

        // in double page mode
        if (mPages[SECOND_PAGE]) {
            // if the xFoldP1.x is outside page width, need to limit
            // xFoldP1.x is in page.width and recompute new key points so
            // that the page flip is still going mForward
            if (page.isXOutsidePage(mXFoldP1.x)) {
                mXFoldP1.x = diagonalP.x;
                float cosA = (mTouchP.x - originP.x) / mLenOfT2O;
                float ratio = (1 - page.mWidth * fabs(cosA) / mLenOfT2O);
                mRadius = (float)(mLenOfT2O * (1 - 2 * ratio) / M_PI);
                mXFoldP0.x = mLenOfT2O * ratio / cosA + originP.x;

                if (mIsVertical) {
                    mYFoldP0.x = mXFoldP0.x;
                    mYFoldP1.x = mXFoldP1.x;
                }
                else {
                    mYFoldP1.y = originP.y + (mXFoldP1.x - originP.x) / mKValue;
                    mYFoldP0.y = originP.y + (mXFoldP0.x - originP.x) / mKValue;
                }

                // re-compute mesh count
                float len = fabs(mMiddleP.x - mXFoldP0.x);
                if (mMeshCount > len) {
                    mMeshCount = (int)len;
                }
                isAnimating = mMeshCount > 0 &&
                              fabs(mXFoldP0.x - diagonalP.x) >= 2;
            }
        }
        // in single page mode, check if the whole fold page is outside the
        // screen and animating should be stopped
        else if (mFlipState == FORWARD_FLIP) {
            float r = (float)(mLenOfT2O * mSemiPerimeterRatio / M_PI);
            float x = (mYFoldP1.y - diagonalP.y) * mKValue + r;
            isAnimating = x > (diagonalP.x - originP.x);
        }
    }

    // animation is stopped
    if (!isAnimating) {
        abortAnimating();
    }
    // continue animation and compute mVertexes
    else if (mIsVertical) {
        computeVertexesWhenVertical();
    }
    else {
        computeVertexesWhenSlope();
    }

    return isAnimating;
}

/**
 * Abort animating
 */
void PageFlip::abortAnimating() {
    mScroller.abortAnimation();
    if (mFlipState == FORWARD_FLIP) {
        mFlipState = END_WITH_FORWARD;
    }
    else if (mFlipState == BACKWARD_FLIP) {
        mFlipState = END_WITH_BACKWARD;
    }
    else if (mFlipState == RESTORE_FLIP) {
        mFlipState = END_WITH_RESTORE;
    }
}

/**
 * Draw flipping frame
 */
void PageFlip::drawFlipFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. draw back of fold page
    glUseProgram(mBackOfFoldVertexProg.programRef());
    glActiveTexture(GL_TEXTURE0);
    mBackOfFoldVertexes.draw(mBackOfFoldVertexProg,
                                 *mPages[FIRST_PAGE],
                                 mPages[SECOND_PAGE] != NULL,
                                 mGradientLightTexId);

    // 2. draw unfold page and front of fold page
    glUseProgram(mVertexProg.programRef());
    glActiveTexture(GL_TEXTURE0);
    mPages[FIRST_PAGE]->drawFrontPage(mVertexProg, mFoldFrontVertexes);
    if (mPages[SECOND_PAGE]) {
        mPages[SECOND_PAGE]->drawFullPage(mVertexProg, true);
    }

    // 3. draw edge and base shadow of fold parts
    glUseProgram(mShadowVertexProg.programRef());
    mFoldBaseShadowVertexes.draw(mShadowVertexProg);
    mFoldEdgeShadowVertexes.draw(mShadowVertexProg);
}

/**
 * Draw frame with full page
 */
void PageFlip::drawPageFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(mVertexProg.programRef());
    glUniformMatrix4fv(mVertexProg.mvpMatrixLoc(), 1, GL_FALSE,
                       mVertexProg.MVPMatrix);
    glActiveTexture(GL_TEXTURE0);

    // 1. draw first page
    mPages[FIRST_PAGE]->drawFullPage(mVertexProg, true);

    // 2. draw second page if have
    if (mPages[SECOND_PAGE]) {
        mPages[SECOND_PAGE]->drawFullPage(mVertexProg, true);
    }
}

/**
 * Compute max mesh count and allocate mVertexes buffer
 */
void PageFlip::computeMaxMeshCount() {
    // compute max mesh count
    int maxMeshCnt = (int) mViewRect.minOfWidthHeight() / mPixelsOfMesh;

    // make sure the vertex count is even number
    if (maxMeshCnt % 2 != 0) {
        maxMeshCnt++;
    }

    // init mVertexes buffers
    mBackOfFoldVertexes.set(maxMeshCnt + 2);
    mFoldFrontVertexes.set((maxMeshCnt << 1) + 8, 3, true);
    mFoldEdgeShadowVertexes.set(maxMeshCnt + 2);
    mFoldBaseShadowVertexes.set(maxMeshCnt + 2);
}

/**
 * Create gradient shadow texture for lighting effect
 */
int PageFlip::setGradientLightTexture(AndroidBitmapInfo &info, GLvoid *data) {
    GLint format;
    GLenum type;
    if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
    }
    else if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    else {
        return Error::ERR_UNSUPPORT_BITMAP_FORMAT;
    }

    glGenTextures(1, &mGradientLightTexId);
    glActiveTexture(GL_TEXTURE0);

    // gradient shadow texture
    glBindTexture(GL_TEXTURE_2D, mGradientLightTexId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format, info.width, info.height, 0, format,
                 type, data);
    return Error::OK;
}

/**
 * Compute mVertexes of page
 */
void PageFlip::computeVertexesBuildPage() {
    if (mIsVertical) {
        computeKeyVertexesWhenVertical();
        computeVertexesWhenVertical();
    }
    else {
        computeKeyVertexesWhenSlope();
        computeVertexesWhenSlope();
    }
}

/**
 * Compute key mVertexes when page flip is vertical
 */
void PageFlip::computeKeyVertexesWhenVertical() {
    const float oX = mPages[FIRST_PAGE]->mOriginP.x ;
    const float oY = mPages[FIRST_PAGE]->mOriginP.y;
    const float dY = mPages[FIRST_PAGE]->mDiagonalP.y;

    mTouchP.y = oY;
    mMiddleP.y = oY;

    // set key point on X axis
    float r0 = 1 - mSemiPerimeterRatio;
    float r1 = 1 + mSemiPerimeterRatio;
    mXFoldP.set(mMiddleP.x, oY);
    mXFoldP0.set(oX + (mXFoldP.x - oX) * r0, mXFoldP.y);
    mXFoldP1.set(oX + r1 * (mXFoldP.x - oX), mXFoldP.y);

    // set key point on Y axis
    mYFoldP.set(mMiddleP.x, dY);
    mYFoldP0.set(mXFoldP0.x, mYFoldP.y);
    mYFoldP1.set(mXFoldP1.x, mYFoldP.y);

    // line length from mTouchP to originP
    mLenOfT2O = fabs(mTouchP.x - oX);
    mRadius = (float)(mLenOfT2O * mSemiPerimeterRatio / M_PI);

    // compute mesh count
    computeMeshCount();
}

/**
 * Compute all mVertexes when page flip is vertical
 */
void PageFlip::computeVertexesWhenVertical() {
    float x = mMiddleP.x;
    const float stepX = (mMiddleP.x - mXFoldP0.x) / mMeshCount;

    Page& page = *mPages[FIRST_PAGE];
    const float oY = page.mOriginP.y;
    const float dY = page.mDiagonalP.y;
    const float dTexY = page.mDiagonalP.texY;
    const float oTexY = page.mOriginP.texY;
    const float oTexX = page.mOriginP.texX;

    // compute the point on back page half cylinder
    mBackOfFoldVertexes.reset();

    for (int i = 0; i <= mMeshCount; ++i, x -= stepX) {
        // compute rad of x point
        float x2t = x - mXFoldP1.x;
        float rad = x2t / mRadius;
        float sinR = sin(rad);
        float texX = page.textureX(x);
        float fx = mXFoldP1.x + mRadius * sinR;
        float fz = mRadius * (1 - cos(rad));

        // compute vertex when it is curled
        mBackOfFoldVertexes.addVertex(fx, dY, fz, sinR, texX, dTexY)
                           .addVertex(fx, oY, fz, sinR, texX, oTexY);
    }

    float tpX = mTouchP.x;
    mBackOfFoldVertexes.addVertex(tpX, dY, 1, 0, oTexX, dTexY)
                       .addVertex(tpX, oY, 1, 0, oTexX, oTexY);

    // compute shadow width
    float sw = -mFoldEdgeShadowWidth.width(mRadius);
    float bw = mFoldBaseShadowWidth.width(mRadius);
    if (page.mOriginP.x < 0) {
        sw = -sw;
        bw = -bw;
    }

    // fold base shadow
    float bx0 = mBackOfFoldVertexes.floatAt(0);
    mFoldBaseShadowVertexes.setVertexes(0, bx0, oY, bx0 + bw, oY)
                           .setVertexes(8, bx0, dY, bx0 + bw, dY)
                           .setRange(0, 16);

    // fold edge shadow
    mFoldEdgeShadowVertexes.setVertexes(0, tpX, oY, tpX + sw, oY)
                           .setVertexes(8, tpX, dY, tpX + sw, dY)
                           .setRange(0, 16);

    // fold front
    mFoldFrontVertexes.reset();
    page.buildVertexesOfPageWhenVertical(mFoldFrontVertexes, mXFoldP1);
}

/**
 * Compute key mVertexes when page flip is slope
 */
void PageFlip::computeKeyVertexesWhenSlope() {
    const float oX = mPages[FIRST_PAGE]->mOriginP.x;
    const float oY = mPages[FIRST_PAGE]->mOriginP.y;

    const float dx = mMiddleP.x - oX;
    const float dy = mMiddleP.y - oY;

    // compute key points on X axis
    float r0 = 1 - mSemiPerimeterRatio;
    float r1 = 1 + mSemiPerimeterRatio;
    mXFoldP.set(mMiddleP.x + dy * dy / dx, oY);
    mXFoldP0.set(oX + (mXFoldP.x - oX) * r0, mXFoldP.y);
    mXFoldP1.set(oX + r1 * (mXFoldP.x - oX), mXFoldP.y);

    // compute key points on Y axis
    mYFoldP.set(oX, mMiddleP.y + dx * dx / dy);
    mYFoldP0.set(mYFoldP.x, oY + (mYFoldP.y - oY) * r0);
    mYFoldP1.set(mYFoldP.x, oY + r1 * (mYFoldP.y - oY));

    // line length from TouchXY to OriginalXY
    mLenOfT2O = hypot(mTouchP.x - oX, mTouchP.y - oY);

    // cylinder radius
    mRadius = (float)(mLenOfT2O * mSemiPerimeterRatio / M_PI);

    // compute line slope
    mKValue = (mTouchP.y - oY) / (mTouchP.x - oX);

    // compute mesh count
    computeMeshCount();
}

/**
 * Compute back vertex and edge shadow vertex of fold page
 * <p>
 * In 2D coordinate system, for every vertex on fold page, we will follow
 * the below steps to compute its 3D point (x,y,z) on curled page(cylinder):
 * </p>
 * <ul>
 *     <li>deem originP as (0, 0) to simplify the next computing steps</li>
 *     <li>translate point(x, y) to new coordinate system
 *     (originP is (0, 0))</li>
 *     <li>rotate point(x, y) with curling angle A in clockwise</li>
 *     <li>compute 3d point (x, y, z) for 2d point(x, y), at this time, the
 *     cylinder is vertical in new coordinate system which will help us
 *     compute point</li>
 *     <li>rotate 3d point (x, y, z) with -A to restore</li>
 *     <li>translate 3d point (x, y, z) to original coordinate system</li>
 * </ul>
 *
 * <p>For point of edge shadow, the most computing steps are same but:</p>
 * <ul>
 *     <li>shadow point is following the page point except different x
 *     coordinate</li>
 *     <li>shadow point has same z coordinate with the page point</li>
 * </ul>
 *
 * @param isX is vertex for x point on x axis or y point on y axis?
 * @param x0 x of point on axis
 * @param y0 y of point on axis
 * @param sx0 x of edge shadow point
 * @param sy0 y of edge shadow point
 * @param tX x of xFoldP1 point in rotated coordinate system
 * @param sinA sin value of page curling angle
 * @param cosA cos value of page curling angel
 * @param texX x of texture coordinate
 * @param coordY y of texture coordinate
 * @param oX x of originate point
 * @param oY y of originate point
 */
void PageFlip::computeBackVertex(bool isX, float x0, float y0, float sx0,
                                 float sy0, float xfx, float sinA,
                                 float cosA, float texX, float texY,
                                 float oX, float oY) {
    // rotate degree A
    float x = x0 * cosA - y0 * sinA;
    float y = x0 * sinA + y0 * cosA;

    // rotate degree A for mVertexes of fold edge shadow
    float sx = sx0 * cosA - sy0 * sinA;
    float sy = sx0 * sinA + sy0 * cosA;

    // compute mapping point on cylinder
    float rad = (x - xfx) / mRadius;
    double sinR = sin(rad);
    x = (float) (xfx + mRadius * sinR);
    float cz = mRadius * (1 - cos(rad));

    // rotate degree -A, sin(-A) = -sin(A), cos(-A) = cos(A)
    float cx = x * cosA + y * sinA + oX;
    float cy = y * cosA - x * sinA + oY;
    mBackOfFoldVertexes.addVertex(cx, cy, cz, (float)sinR, texX, texY);

    // compute coordinates of fold shadow edge
    float sRad = (sx - xfx) / mRadius;
    sx = xfx + mRadius * sin(sRad);
    mFoldEdgeShadowVertexes.addVertexes(isX, cx, cy,
                                        sx * cosA + sy * sinA + oX,
                                        sy * cosA - sx * sinA + oY);
}

/**
 * Compute back vertex of fold page
 * <p>
 * Almost same with another computeBackVertex function except expunging the
 * shadow point part
 * </p>
 *
 * @param x0 x of point on axis
 * @param y0 y of point on axis
 * @param tX x of xFoldP1 point in rotated coordinate system
 * @param sinA sin value of page curling angle
 * @param cosA cos value of page curling angel
 * @param texX x of texture coordinate
 * @param coordY y of texture coordinate
 * @param oX x of originate point
 * @param oY y of originate point
 */
void PageFlip::computeBackVertex(float x0, float y0, float xfx,
                                 float sinA, float cosA, float texX,
                                 float texY, float oX, float oY) {
    // rotate degree A
    float x = x0 * cosA - y0 * sinA;
    float y = x0 * sinA + y0 * cosA;

    // compute mapping point on cylinder
    float rad = (x - xfx) / mRadius;
    double sinR = sin(rad);
    x = (float) (xfx + mRadius * sinR);
    float cz = mRadius * (1 - cos(rad));

    // rotate degree -A, sin(-A) = -sin(A), cos(-A) = cos(A)
    float cx = x * cosA + y * sinA + oX;
    float cy = y * cosA - x * sinA + oY;
    mBackOfFoldVertexes.addVertex(cx, cy, cz, (float)sinR, texX, texY);
}

/**
 * Compute front vertex and base shadow vertex of fold page
 * <p>The computing principle is almost same with
 * {@link #computeBackVertex(boolean, float, float, float, float, float,
 * float, float, float, float, float, float)}</p>
 *
 * @param isX is vertex for x point on x axis or y point on y axis?
 * @param x0 x of point on axis
 * @param y0 y of point on axis
 * @param tX x of xFoldP1 point in rotated coordinate system
 * @param sinA sin value of page curling angle
 * @param cosA cos value of page curling angel
 * @param baseWcosA base shadow width * cosA
 * @param baseWsinA base shadow width * sinA
 * @param texX x of texture coordinate
 * @param coordY y of texture coordinate
 * @param oX x of originate point
 * @param oY y of originate point
 */
void PageFlip::computeFrontVertex(bool isX, float x0, float y0, float xfx,
                                  float sinA, float cosA,
                                  float baseWCosA, float baseWSinA,
                                  float texX, float texY,
                                  float oX, float oY) {
    // rotate degree A
    float x = x0 * cosA - y0 * sinA;
    float y = x0 * sinA + y0 * cosA;

    // compute mapping point on cylinder
    float rad = (x - xfx)/ mRadius;
    x = xfx + mRadius * sin(rad);
    float cz = mRadius * (1 - cos(rad));

    // rotate degree -A, sin(-A) = -sin(A), cos(-A) = cos(A)
    float cx = x * cosA + y * sinA + oX;
    float cy = y * cosA - x * sinA + oY;
    mFoldFrontVertexes.addVertex(cx, cy, cz, texX, texY);
    mFoldBaseShadowVertexes.addVertexes(isX, cx, cy,
                                        cx + baseWCosA,
                                        cy - baseWSinA);
}

/**
 * Compute front vertex
 * <p>The difference with another
 * {@link #computeFrontVertex(boolean, float, float, float, float, float,
 * float, float, float, float, float, float, float)} is that it won't
 * compute base shadow vertex</p>
 *
 * @param x0 x of point on axis
 * @param y0 y of point on axis
 * @param tX x of xFoldP1 point in rotated coordinate system
 * @param sinA sin value of page curling angle
 * @param cosA cos value of page curling angel
 * @param texX x of texture coordinate
 * @param coordY y of texture coordinate
 * @param oX x of originate point
 * @param oY y of originate point
 */
void PageFlip::computeFrontVertex(float x0, float y0, float xfx,
                                  float sinA, float cosA,
                                  float texX, float texY,
                                  float oX, float oY) {
    // rotate degree A
    float x = x0 * cosA - y0 * sinA;
    float y = x0 * sinA + y0 * cosA;

    // compute mapping point on cylinder
    float rad = (x - xfx)/ mRadius;
    x = xfx + mRadius * sin(rad);
    float cz = mRadius * (1 - cos(rad));

    // rotate degree -A, sin(-A) = -sin(A), cos(-A) = cos(A)
    float cx = x * cosA + y * sinA + oX;
    float cy = y * cosA - x * sinA + oY;
    mFoldFrontVertexes.addVertex(cx, cy, cz, texX, texY);
}

/**
 * Compute last vertex of base shadow(mBackward direction)
 * <p>
 * The mVertexes of base shadow are composed by two part: mForward and
 * mBackward part. Forward mVertexes are computed from XFold points and
 * mBackward mVertexes are computed from YFold points. The reason why we use
 * mForward and mBackward is because how to change float buffer index when we
 * add a new vertex to buffer. Backward means the index is declined from
 * buffer middle position to the head, in contrast, the mForward is
 * increasing index from middle to the tail. This design will help keep
 * float buffer consecutive and to be draw at a time.
 * </p><p>
 * Sometimes, the whole or part of YFold points will be outside page, that
 * means their Y coordinate are greater than page height(diagonal.y). In
 * this case, we have to crop them like cropping line on 2D coordinate
 * system. If delve further, we can conclude that we only need to compute
 * the first start/end mVertexes which is falling on the border line of
 * diagonal.y since other mBackward mVertexes must be outside page and could
 * not be seen, and then combine these mVertexes with mForward mVertexes to
 * render base shadow.
 * </p><p>
 * This function is just used to compute the couple mVertexes.
 * </p>
 *
 * @param x0 x of point on axis
 * @param y0 y of point on axis
 * @param tX x of xFoldP1 point in rotated coordinate system
 * @param sinA sin value of page curling angle
 * @param cosA cos value of page curling angel
 * @param baseWcosA base shadow width * cosA
 * @param baseWsinA base shadow width * sinA
 * @param oX x of originate point
 * @param oY y of originate point
 * @param dY y of diagonal point
 */
void PageFlip::computeBaseShadowLastVertex(float x0, float y0, float xfs,
                                           float sinA, float cosA,
                                           float baseWCosA,
                                           float baseWSinA,
                                           float oX, float oY, float dY) {
    // like computing front vertex, we firstly compute the mapping vertex
    // on fold cylinder for point (x0, y0) which also is last vertex of
    // base shadow(mBackward direction)
    float x = x0 * cosA - y0 * sinA;
    float y = x0 * sinA + y0 * cosA;

    // compute mapping point on cylinder
    float rad = (x - xfs)/ mRadius;
    x = xfs + mRadius * sin(rad);

    float cx1 = x * cosA + y * sinA + oX;
    float cy1 = y * cosA - x * sinA + oY;

    // now, we have start vertex(cx1, cy1), compute end vertex(cx2, cy2)
    // which is translated based on start vertex(cx1, cy1)
    float cx2 = cx1 + baseWCosA;
    float cy2 = cy1 - baseWSinA;

    // as we know, this function is only used to compute last vertex of
    // base shadow(mBackward) when the YFold points are outside page height,
    // that means the (cx1, cy1) and (cx2, cy2) we computed above normally
    // is outside page, so we need to compute their projection points on page
    // border as rendering vertex of base shadow
    float bx1 = cx1 + mKValue * (cy1 - dY);
    float bx2 = cx2 + mKValue * (cy2 - dY);

    // add start/end vertex into base shadow buffer, it will be linked with
    // mForward mVertexes to draw base shadow
    mFoldBaseShadowVertexes.addVertexes(false, bx1, dY, bx2, dY);
}

/**
 * Compute mVertexes when page flip is slope
 */
void PageFlip::computeVertexesWhenSlope() {
    Page& page = *mPages[FIRST_PAGE];
    const float oX = page.mOriginP.x;
    const float oY = page.mOriginP.y;
    const float dY = page.mDiagonalP.y;
    const float oTexX = page.mOriginP.texX;
    const float oTexY = page.mOriginP.texY;
    const float dTexY = page.mDiagonalP.texY;
    const float height = page.mHeight;
    const float d2oY = dY - oY;

    // compute radius and sin/cos of angle
    const float sinA = (mTouchP.y - oY) / mLenOfT2O;
    const float cosA = (oX - mTouchP.x) / mLenOfT2O;

    // need to translate before rotate, and then translate back
    float xFP1 = (mXFoldP1.x - oX) * cosA;
    float edgeW = mFoldEdgeShadowWidth.width(mRadius);
    float baseW = mFoldBaseShadowWidth.width(mRadius);
    float baseWCosA = baseW * cosA;
    float baseWSinA = baseW * sinA;
    float edgeY = oY > 0 ? edgeW : -edgeW;
    float edgeX = oX > 0 ? edgeW : -edgeW;
    float stepSY = edgeY / mMeshCount;
    float stepSX = edgeX / mMeshCount;

    // reset mVertexes buffer counter
    mFoldEdgeShadowVertexes.reset();
    mFoldBaseShadowVertexes.reset();
    mFoldFrontVertexes.reset();
    mBackOfFoldVertexes.reset();

    // add the first 3 float numbers is fold triangle
    mBackOfFoldVertexes.addVertex(mTouchP.x, mTouchP.y, 1, 0, oTexX, oTexY);

    // compute mVertexes for fold back part
    float stepX = (mXFoldP0.x - mXFoldP.x) / mMeshCount;
    float stepY = (mYFoldP0.y - mYFoldP.y) / mMeshCount;
    float x = mXFoldP0.x - oX;
    float y = mYFoldP0.y - oY;
    float sx = edgeX;
    float sy = edgeY;

    // compute point of back of fold page
    // Case 1: y coordinate of point YFP0 -> YFP is < diagonalP.y
    //
    //   <---- Flip
    // +-------------+ diagonalP
    // |             |
    // |             + YFP
    // |            /|
    // |           / |
    // |          /  |
    // |         /   |
    // |        /    + YFP0
    // |       / p  /|
    // +------+--.-+-+ originP
    //      XFP   XFP0
    //
    // 1. XFP -> XFP0 -> originP -> YFP0 ->YFP is back of fold page
    // 2. XFP -> XFP0 -> YFP0 -> YFP is a half of cylinder when page is
    //    curled
    // 3. P point will be computed
    //
    // compute points within the page
    int i = 0;
    for (; i <= mMeshCount && fabs(y) < height;
          ++i, x -= stepX, y -= stepY, sy -= stepSY, sx -= stepSX) {
        computeBackVertex(true, x, 0, x, sy, xFP1, sinA, cosA,
                          page.textureX(x + oX), oTexY, oX, oY);
        computeBackVertex(false, 0, y, sx, y, xFP1, sinA, cosA, oTexX,
                          page.textureY(y + oY), oX, oY);
    }

    // If y coordinate of point on YFP0 -> YFP is > diagonalP
    // There are two cases:
    //                      <---- Flip
    //     Case 2                               Case 3
    //          YFP                               YFP   YFP0
    // +---------+---+ diagonalP          +--------+-----+--+ diagonalP
    // |        /    |                    |       /     /   |
    // |       /     + YFP0               |      /     /    |
    // |      /     /|                    |     /     /     |
    // |     /     / |                    |    /     /      |
    // |    /     /  |                    |   /     /       |
    // |   / p   /   |                    |  / p   /        |
    // +--+--.--+----+ originalP          +-+--.--+---------+ originalP
    //   XFP   XFP0                        XFP   XFP0
    //
    // compute points outside the page
    if (i <= mMeshCount) {
        if (fabs(y) != height) {
            // case 3: compute mapping point of diagonalP
            if (fabs(mYFoldP0.y - oY) > height) {
                float tx = oX + 2 * mKValue * (mYFoldP.y - dY);
                float ty = dY + mKValue * (tx - oX);
                mBackOfFoldVertexes.addVertex(tx, ty, 1, 0, oTexX, dTexY);

                float tsx = tx - sx;
                float tsy = dY + mKValue * (tsx - oX);
                mFoldEdgeShadowVertexes.addVertexes(false, tx, ty, tsx, tsy);
            }
            // case 2: compute mapping point of diagonalP
            else {
                float x1 = mKValue* d2oY;
                computeBackVertex(true, x1, 0, x1, sy, xFP1, sinA, cosA,
                                  page.textureX(x1 + oX), oTexY, oX, oY);
                computeBackVertex(false, 0, d2oY, sx, d2oY, xFP1, sinA,
                                  cosA, oTexX, dTexY, oX, oY);
            }
        }

        // compute the remaining points
        for (; i <= mMeshCount;
               ++i, x -= stepX, y -= stepY, sy -= stepSY, sx -= stepSX) {
            computeBackVertex(true, x, 0, x, sy, xFP1, sinA, cosA,
                              page.textureX(x + oX), oTexY, oX, oY);

            // since the origin Y is beyond page, we need to compute its
            // projection point on page border and then compute mapping
            // point on curled cylinder
            float x1 = mKValue * (y + oY - dY);
            computeBackVertex(x1, d2oY, xFP1, sinA, cosA,
                              page.textureX(x1 + oX), dTexY, oX, oY);
        }
    }

    // Like above computation, the below steps are computing mVertexes of
    // front of fold page
    // Case 1: y coordinate of point YFP -> YFP1 is < diagonalP.y
    //
    //     <---- Flip
    // +----------------+ diagonalP
    // |                |
    // |                + YFP1
    // |               /|
    // |              / |
    // |             /  |
    // |            /   |
    // |           /    + YFP
    // |          /    /|
    // |         /    / |
    // |        /    /  + YFP0
    // |       /    /  /|
    // |      / p  /  / |
    // +-----+--.-+--+--+ originP
    //    XFP1  XFP  XFP0
    //
    // 1. XFP -> YFP -> YFP1 ->XFP1 is front of fold page and a half of
    //    cylinder when page is curled.
    // 2. YFP->XFP is joint line of front and back of fold page
    // 3. P point will be computed
    //
    // compute points within the page
    stepX = (mXFoldP.x - mXFoldP1.x) / mMeshCount;
    stepY = (mYFoldP.y - mYFoldP1.y) / mMeshCount;
    x = mXFoldP.x - oX - stepX;
    y = mYFoldP.y - oY - stepY;
    int j = 0;
    for (; j < mMeshCount && fabs(y) < height; ++j, x -= stepX, y -= stepY) {
        computeFrontVertex(true, x, 0, xFP1, sinA, cosA, baseWCosA, baseWSinA,
                           page.textureX(x + oX), oTexY, oX, oY);
        computeFrontVertex(false, 0, y, xFP1, sinA, cosA, baseWCosA, baseWSinA,
                           oTexX, page.textureY(y + oY), oX, oY);
    }

    // compute points outside the page
    if (j < mMeshCount) {
        // compute mapping point of diagonalP
        if (fabs(y) != height && j > 0) {
            float y1 = (dY - oY);
            float x1 = mKValue * y1;
            computeFrontVertex(true, x1, 0, xFP1, sinA, cosA,
                               baseWCosA, baseWSinA,
                               page.textureX(x1 + oX), oTexY, oX, oY);

            computeFrontVertex(0, y1, xFP1, sinA, cosA, oTexX,
                               page.textureY(y1 + oY), oX, oY);
        }

        // compute last pair of mVertexes of base shadow
        computeBaseShadowLastVertex(0, y, xFP1, sinA, cosA,
                                    baseWCosA, baseWSinA, oX, oY, dY);

        // compute the remaining points
        for (; j < mMeshCount; ++j, x -= stepX, y -= stepY) {
            computeFrontVertex(true, x, 0, xFP1, sinA, cosA,
                               baseWCosA, baseWSinA,
                               page.textureX(x + oX), oTexY, oX, oY);

            float x1 = mKValue * (y + oY - dY);
            computeFrontVertex(x1, d2oY, xFP1, sinA, cosA,
                               page.textureX(x1 + oX), dTexY, oX, oY);
        }

    }

    // set uniform Z value for shadow mVertexes
    mFoldEdgeShadowVertexes.setVertexZ(mFoldFrontVertexes.floatAt(2));
    mFoldBaseShadowVertexes.setVertexZ(-0.5f);

    // add two mVertexes to connect with the unfold front page
    page.buildVertexesOfPageWhenSlope(mFoldFrontVertexes, mXFoldP1,
                                      mYFoldP1, mKValue);

    // compute mVertexes of fold edge shadow
    computeVertexesOfFoldTopEdgeShadow(mTouchP.x, mTouchP.y,
                                       sinA, cosA, -edgeX, edgeY);
}

/**
 * Compute mVertexes of fold top edge shadow
 * <p>Top edge shadow of fold page is a quarter circle</p>
 *
 * @param x0 X of touch point
 * @param y0 Y of touch point
 * @param sinA Sin value of page curling angle
 * @param cosA Cos value of page curling angle
 * @param sx Shadow width on X axis
 * @param sy Shadow width on Y axis
 */
void PageFlip::computeVertexesOfFoldTopEdgeShadow(float x0, float y0,
                                                  float sinA, float cosA,
                                                  float sx, float sy) {
    float sin2A = 2 * sinA * cosA;
    float cos2A = 1 - 2 * pow(sinA, 2);
    float r = 0;
    float dr = (float)(M_PI / (kFoldTopEdgeShadowVexCount - 2));
    int size = kFoldTopEdgeShadowVexCount / 2;
    int j = mFoldEdgeShadowVertexes.maxBackward();

    //                 ^ Y                             __ |
    //      TouchP+    |                             /    |
    //             \   |                            |     |
    //              \  |                             \    |
    //               \ |              X <--------------+--+- OriginP
    //                \|                                 /|
    // X <----------+--+- OriginP                       / |
    //             /   |                               /  |
    //             |   |                              /   |
    //              \__+ Top edge              TouchP+    |
    //                 |                                  v Y
    // 1. compute quarter circle at origin point
    // 2. rotate quarter circle to touch point direction
    // 3. move quarter circle to touch point as top edge shadow
    for (int i = 0; i < size; ++i, r += dr, j += 8) {
        float x = sx * cos(r);
        float y = sy * sin(r);

        // rotate -2A and then translate to touchP
        mFoldEdgeShadowVertexes.setVertexes(j, x0, y0,
                                            x * cos2A + y * sin2A + x0,
                                            y * cos2A - x * sin2A + y0);
    }
}

/**
 * Compute mesh count for page flip
 */
void PageFlip::computeMeshCount() {
    float dx = fabs(mXFoldP0.x - mXFoldP1.x);
    float dy = fabs(mYFoldP0.y - mYFoldP1.y);
    int len = mIsVertical ? (int)dx : (int)std::min(dx, dy);
    mMeshCount = 0;

    // make sure mesh count is greater than threshold, if less than it,
    // the page maybe is drawn unsmoothly
    for (int i = mPixelsOfMesh;
         i >= 1 && mMeshCount < kMeshCountThreshold;
         i >>= 1) {
        mMeshCount = len / i;
    }

    // keep count is even
    if (mMeshCount % 2 != 0) {
        mMeshCount++;
    }

    // half count for fold page
    mMeshCount >>= 1;
}

/**
 * Compute tan value of curling angle
 *
 * @param dy the diff value between touchP.y and originP.y
 * @return tan value of max curl angle
 */
float PageFlip::computeTanOfCurlAngle(float dy) {
    float ratio = dy / mViewRect.halfHeight;
    if (ratio <= 1 - kMaxPageCurlAngleRatio) {
        return kMaxPageCurlTanOfAngle;
    }

    float degree = kMaxPageCurlAngle - kPageCurlAngleDiff * ratio;
    if (degree < kMinPageCurlAngle) {
        return kMinPageCurlTanOfAngle;
    }
    else {
        return (float)tan(M_PI * degree / 180);
    }
}

/**
 * Debug information
 */
void PageFlip::printInfo() {
    GLPoint originP = mPages[FIRST_PAGE]->mOriginP;
    GLPoint diagonalP = mPages[FIRST_PAGE]->mDiagonalP;

    LOGD(TAG, "************************************");
    LOGD(TAG, " Mesh Count:    %d", mMeshCount);
    LOGD(TAG, " Mesh Pixels:   %d", mPixelsOfMesh);
    LOGD(TAG, " Origin:        %f, %f", originP.x, originP.y);
    LOGD(TAG, " Diagonal:      %f, %f", diagonalP.x, diagonalP.y);
    LOGD(TAG, " OriginTouchP:  %f, %f", mStartTouchP.x, + mStartTouchP.y);
    LOGD(TAG, " TouchP:        %f, %f", mTouchP.x, mTouchP.y);
    LOGD(TAG, " MiddleP:       %f, %f", mMiddleP.x , mMiddleP.y);
    LOGD(TAG, " XFoldP:        %f, %f", mXFoldP.x, mXFoldP.y);
    LOGD(TAG, " XFoldP0:       %f, %f", mXFoldP0.x, mXFoldP0.y);
    LOGD(TAG, " XFoldP1:       %f, %f", mXFoldP1.x, mXFoldP1.y);
    LOGD(TAG, " YFoldP:        %f, %f", mYFoldP.x, mYFoldP.y);
    LOGD(TAG, " YFoldP0:       %f, %f", mYFoldP0.x, mYFoldP0.y);
    LOGD(TAG, " YFoldP1:       %f, %f", mYFoldP1.x, mYFoldP1.y);
    LOGD(TAG, " LengthT->O:    %f", mLenOfT2O);
}

}

