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

#ifndef ANDROID_PAGEFLIP_SCROLLER_H
#define ANDROID_PAGEFLIP_SCROLLER_H

#include <math.h>
#include <time.h>
#include <sys/time.h>

#define DEFAULT_DURATION 250

namespace eschao {

class Interpolator {

public:
    virtual ~Interpolator() { };

    virtual float interpolate(float input) = 0;

protected:
    Interpolator() { }
};

class AccelerateInterpolator : public Interpolator {

public:
    explicit AccelerateInterpolator(float factor = 1.0f)
            : mFactor(factor), mDoubleFactor(factor * 2) { }

    virtual float interpolate(float input) {
        if (mFactor == 1.0f) {
            return input * input;
        }
        else {
            return (float) pow(input, mDoubleFactor);
        }
    }

private:
    const float mFactor;
    const float mDoubleFactor;
};

class LinearInterpolator : public Interpolator {

public:
    virtual float interpolate(float input) {
        return input;
    }
};

class ViscousFluidInterpolator : public Interpolator {

public:
    float interpolate(float input) {
        float interpolated = VISCOUS_FLUID_NORMALIZE * viscousFluid(input);
        if (interpolated > 0) {
            return interpolated + VISCOUS_FLUID_OFFSET;
        }
        return interpolated;
    }

private:
    static const float VISCOUS_FLUID_SCALE;
    static const float VISCOUS_FLUID_NORMALIZE;
    static const float VISCOUS_FLUID_OFFSET;

    static float viscousFluid(float x) {
        x *= VISCOUS_FLUID_SCALE;
        if (x < 1.0f) {
            x -= (1.0f - (float)exp(-x));
        }
        else {
            float start = 0.36787944117f; // 1/e == exp(-1)
            x = 1.0f - (float)exp(1.0f - x);
            x = start + x * (1.0f - start);
        }
        return x;
    }

};

/**
 * Copied from Android Scroller.java &
 * frameworks/base/+/master/libs/hwui/Interpolator.h
 */
class Scroller {

public:
    Scroller();
    Scroller(Interpolator *interpolator);
    ~Scroller();

    bool computeScrollOffset();
    void startScroll(float startX, float startY, float dx, float dy,
                     int duration = DEFAULT_DURATION);

    inline void setInterpolator(Interpolator *interpolator) {
        if (mInterpolator) {
            delete mInterpolator;
        }
        mInterpolator = interpolator;
    }

    inline bool isFinished() {
        return mFinished;
    }

    inline void setFinished(bool finished) {
        mFinished = finished;
    }

    inline int duration() {
        return mDuration;
    }

    inline float currX() {
        return mCurrX;
    }

    inline float currY() {
        return mCurrY;
    }

    inline float startX() {
        return mStartX;
    }

    inline float startY() {
        return mStartY;
    }

    inline float finalX() {
        return mFinalX;
    }

    inline float finalY() {
        return mFinalY;
    }

    inline void abortAnimation() {
        mCurrX = mFinalX;
        mCurrY = mFinalY;
        mFinished = true;
    }

private:
    inline long long getSystemClock()
    {
        gettimeofday(&mTimeVal, NULL);
        return ((mTimeVal.tv_sec * 1000) + (mTimeVal.tv_usec / 1000));
    }

private:
    Interpolator* mInterpolator;
    struct timeval mTimeVal;

    float mStartX;
    float mStartY;
    float mFinalX;
    float mFinalY;
    float mDeltaX;
    float mDeltaY;

    int mDuration;
    float mDurationReciprocal;
    float mCurrX;
    float mCurrY;
    bool mFinished;
    long long mStartTime;
};

}
#endif //ANDROID_PAGEFLIP_SCROLLER_H
