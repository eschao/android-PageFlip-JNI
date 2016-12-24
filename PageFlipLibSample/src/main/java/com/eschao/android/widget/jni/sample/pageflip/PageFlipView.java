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
package com.eschao.android.widget.jni.sample.pageflip;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.util.Log;

import com.eschao.android.widget.jni.pageflip.PageFlipLib;

import java.util.concurrent.locks.ReentrantLock;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Page flip view
 *
 * @author eschao
 */

public class PageFlipView extends GLSurfaceView implements Renderer {

    private final static String TAG = "PageFlipView";

    int mPageNo;
    int mDuration;
    Handler mHandler;
    PageRender mPageRender;
    ReentrantLock mDrawLock;

    public PageFlipView(Context context) {
        super(context);

        // create handler to tackle message
        newHandler();

        // load preferences
        SharedPreferences pref = PreferenceManager
                                    .getDefaultSharedPreferences(context);
        mDuration = pref.getInt(Constants.PREF_DURATION, 1000);
        int pixelsOfMesh = pref.getInt(Constants.PREF_MESH_PIXELS, 10);
        boolean isAuto = pref.getBoolean(Constants.PREF_PAGE_MODE, true);

        // create PageFlip
        PageFlipLib.init();
        Log.d(TAG, "PageFlipLib init...");
        PageFlipLib.setSemiPerimeterRatio(0.8f);
        PageFlipLib.setShadowWidthOfFoldEdges(5, 60, 0.3f);
        PageFlipLib.setShadowWidthOfFoldBase(5, 80, 0.4f);
        PageFlipLib.setPixelsOfMesh(pixelsOfMesh);
        PageFlipLib.enableAutoPage(isAuto);
        setEGLContextClientVersion(2);

        // init others
        mPageNo = 1;
        mDrawLock = new ReentrantLock();
        mPageRender = new SinglePageRender(context, mHandler, mPageNo);
        // configure render
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    /**
     * Is auto page mode enabled?
     *
     * @return true if auto page mode enabled
     */
    public boolean isAutoPageEnabled() {
        return PageFlipLib.isAutoPageEnabled();
    }

    /**
     * Enable/Disable auto page mode
     *
     * @param enable true is enable
     */
    public void enableAutoPage(boolean enable) {
        if (PageFlipLib.enableAutoPage(enable) > 0) {
            try {
                mDrawLock.lock();
                final boolean hasSecondPage = PageFlipLib.hasSecondPage();
                if (hasSecondPage &&
                    mPageRender instanceof SinglePageRender) {
                    mPageRender = new DoublePagesRender(getContext(),
                                                        mHandler,
                                                        mPageNo);
                    mPageRender.onSurfaceChanged(PageFlipLib.getSurfaceWidth(),
                                                 PageFlipLib.getSurfaceHeight());
                }
                else if (!hasSecondPage &&
                         mPageRender instanceof DoublePagesRender) {
                    mPageRender = new SinglePageRender(getContext(),
                                                       mHandler,
                                                       mPageNo);
                    mPageRender.onSurfaceChanged(PageFlipLib.getSurfaceWidth(),
                                                 PageFlipLib.getSurfaceHeight());
                }
                requestRender();
            }
            finally {
                mDrawLock.unlock();
            }
        }
    }

    /**
     * Get duration of animating
     *
     * @return duration of animating
     */
    public int getAnimateDuration() {
        return mDuration;
    }

    /**
     * Set animate duration
     *
     * @param duration duration of animating
     */
    public void setAnimateDuration(int duration) {
        mDuration = duration;
    }

    /**
     * Get pixels of mesh
     *
     * @return pixels of mesh
     */
    public int getPixelsOfMesh() {
        return PageFlipLib.getPixelsOfMesh();
    }

    public void onDestroy() {
        PageFlipLib.release();
    }

    /**
     * Handle finger down event
     *
     * @param x finger x coordinate
     * @param y finger y coordinate
     */
    public void onFingerDown(float x, float y) {
        // if the animation is going, we should ignore this event to avoid
        // mess drawing on screen
        if (!PageFlipLib.isAnimating() &&
            PageFlipLib.hasFirstPage()) {
            PageFlipLib.onFingerDown(x, y);
        }
    }

    /**
     * Handle finger moving event
     *
     * @param x finger x coordinate
     * @param y finger y coordinate
     */
    public void onFingerMove(float x, float y) {
        if (PageFlipLib.isAnimating()) {
            // nothing to do during animating
        }
        else if (PageFlipLib.canAnimate(x, y)) {
            // if the point is out of current page, try to start animating
            onFingerUp(x, y);
        }
        // move page by finger
        else if (PageFlipLib.onFingerMove(x, y)) {
            try {
                mDrawLock.lock();
                if (mPageRender != null &&
                    mPageRender.onFingerMove(x, y)) {
                    requestRender();
                }
            }
            finally {
                mDrawLock.unlock();
            }
        }
    }

    /**
     * Handle finger up event and start animating if need
     *
     * @param x finger x coordinate
     * @param y finger y coordinate
     */
    public void onFingerUp(float x, float y) {
        if (!PageFlipLib.isAnimating()) {
            PageFlipLib.onFingerUp(x, y, mDuration);
            try {
                mDrawLock.lock();
                if (mPageRender != null &&
                    mPageRender.onFingerUp(x, y)) {
                    requestRender();
                }
            }
            finally {
                mDrawLock.unlock();
            }
        }
    }

    /**
     * Draw frame
     *
     * @param gl OpenGL handle
     */
    @Override
    public void onDrawFrame(GL10 gl) {
        try {
            mDrawLock.lock();
            if (mPageRender != null) {
                mPageRender.onDrawFrame();
            }
        }
        finally {
            mDrawLock.unlock();
        }
    }

    /**
     * Handle surface is changed
     *
     * @param gl OpenGL handle
     * @param width new width of surface
     * @param height new height of surface
     */
    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
            int ret = PageFlipLib.onSurfaceChanged(width, height);

            // if there is the second page, create double page render when need
            int pageNo = mPageRender.getPageNo();
            if (PageFlipLib.hasSecondPage() && width > height) {
                if (!(mPageRender instanceof DoublePagesRender)) {
                    mPageRender.release();
                    mPageRender = new DoublePagesRender(getContext(),
                                                        mHandler,
                                                        pageNo);
                }
            }
            // if there is only one page, create single page render when need
            else if(!(mPageRender instanceof SinglePageRender)) {
                mPageRender.release();
                mPageRender = new SinglePageRender(getContext(),
                                                   mHandler,
                                                   pageNo);
            }

            // let page render handle surface change
            mPageRender.onSurfaceChanged(width, height);
    }

    /**
     * Handle surface is created
     *
     * @param gl OpenGL handle
     * @param config EGLConfig object
     */
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.d("PageFlipView", "Create Surface....");
        if (PageFlipLib.onSurfaceCreated() > -1) {
            Bitmap b = PageFlipLib.createGradientBitmap();
            PageFlipLib.setGradientLightTexture(b);
            b.recycle();
        }
    }

    /**
     * Create message handler to cope with messages from page render,
     * Page render will send message in GL thread, but we want to handle those
     * messages in main thread that why we need handler here
     */
    private void newHandler() {
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case PageRender.MSG_ENDED_DRAWING_FRAME:
                        try {
                            mDrawLock.lock();
                            // notify page render to handle ended drawing
                            // message
                            if (mPageRender != null &&
                                mPageRender.onEndedDrawing(msg.arg1)) {
                                requestRender();
                            }
                        }
                        finally {
                            mDrawLock.unlock();
                        }
                        break;

                    default:
                        break;
                }
            }
        };
    }
}
