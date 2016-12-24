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

#include "Scroller.h"

namespace eschao {

const float ViscousFluidInterpolator::VISCOUS_FLUID_SCALE = 8.0f;
const float ViscousFluidInterpolator::VISCOUS_FLUID_NORMALIZE = 1.0f /
                            ViscousFluidInterpolator::viscousFluid(1.0f);
const float ViscousFluidInterpolator::VISCOUS_FLUID_OFFSET = 1.0f -
        VISCOUS_FLUID_NORMALIZE * ViscousFluidInterpolator::viscousFluid(1.0f);

Scroller::Scroller()
        : mFinished(true) {
    mInterpolator = new ViscousFluidInterpolator();
}

Scroller::Scroller(Interpolator *interpolator)
        : mFinished(true), mInterpolator(interpolator) {
}

Scroller::~Scroller() {
    if (mInterpolator) {
        delete mInterpolator;
    }
}

bool Scroller::computeScrollOffset() {
    if (mFinished) {
        return false;
    }

    long time_passed = (long)(getSystemClock() - mStartTime);

    if (time_passed < mDuration) {
        const float x = mInterpolator->interpolate(
                                            time_passed * mDurationReciprocal);
        mCurrX = (float) (mStartX + round(x * mDeltaX));
        mCurrY = (float) (mStartY + round(x * mDeltaY));
    }
    else {
        mCurrX = mFinalX;
        mCurrY = mFinalY;
        mFinished = true;
    }

    return true;
}

void Scroller::startScroll(float startX, float startY,
                           float dx, float dy,
                           int duration) {
    mFinished = false;
    mDuration = duration;
    mStartTime = getSystemClock();
    mDurationReciprocal = 1.0f / (float)duration;

    mStartX = startX;
    mStartY = startY;
    mFinalX = startX + dx;
    mFinalY = startY + dy;
    mDeltaX = dx;
    mDeltaY = dy;
}

}
