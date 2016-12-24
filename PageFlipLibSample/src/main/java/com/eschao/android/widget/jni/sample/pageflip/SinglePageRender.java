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
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Message;

import com.eschao.android.widget.jni.pageflip.PageFlipLib;

/**
 * Single page render
 * <p>
 * Every page need 2 texture in single page mode:
 * <ul>
 *     <li>First texture: current page content</li>
 *     <li>Back texture: back of front content, it is same with first texture
 *     </li>
 *     <li>Second texture: mNext page content</li>
 * </ul>
 * </p>
 *
 * @author eschao
 */

public class SinglePageRender extends PageRender {

    /**
     * Constructor
     * @see {@link #PageRender(Context, PageFlip, Handler, int)}
     */
    public SinglePageRender(Context context, Handler handler, int pageNo) {
        super(context, handler, pageNo);
    }

    /**
     * Draw frame
     */
    public void onDrawFrame() {
        // 1. delete unused textures
        PageFlipLib.recycleTextures();

        // 2. handle drawing command triggered from finger moving and animating
        if (mDrawCommand == DRAW_MOVING_FRAME ||
            mDrawCommand == DRAW_ANIMATING_FRAME) {
            // is mForward flip
            if (PageFlipLib.getFlipState() == PageFlipLib.FORWARD_FLIP) {
                // check if second texture of first page is valid, if not,
                // create new one
                if (!PageFlipLib.isSecondTextureSet(true)) {
                    drawPage(mPageNo + 1);
                    PageFlipLib.setSecondTexture(true, mBitmap);
                }
            }
            // in mBackward flip, check first texture of first page is valid
            else if (!PageFlipLib.isFirstTextureSet(true)) {
                drawPage(--mPageNo);
                PageFlipLib.setFirstTexture(true, mBitmap);
            }

            // draw frame for page flip
            PageFlipLib.drawFlipFrame();
        }
        // draw stationary page without flipping
        else if (mDrawCommand == DRAW_FULL_PAGE) {
            if (!PageFlipLib.isFirstTextureSet(true)) {
                drawPage(mPageNo);
                PageFlipLib.setFirstTexture(true, mBitmap);
            }

            PageFlipLib.drawPageFrame();
        }

        // 3. send message to main thread to notify drawing is ended so that
        // we can continue to calculate mNext animation frame if need.
        // Remember: the drawing operation is always in GL thread instead of
        // main thread
        Message msg = Message.obtain();
        msg.what = MSG_ENDED_DRAWING_FRAME;
        msg.arg1 = mDrawCommand;
        mHandler.sendMessage(msg);
    }

    /**
     * Handle GL surface is changed
     *
     * @param width surface width
     * @param height surface height
     */
    public void onSurfaceChanged(int width, int height) {
        // recycle bitmap resources if need
        if (mBackgroundBitmap != null) {
            mBackgroundBitmap.recycle();
        }

        if (mBitmap != null) {
            mBitmap.recycle();
        }

        // create bitmap and canvas for page
        mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        mCanvas.setBitmap(mBitmap);
        LoadBitmapTask.get(mContext).set(width, height, 1);
    }

    /**
     * Handle ended drawing event
     * In here, we only tackle the animation drawing event, If we need to
     * continue requesting render, please return true. Remember this function
     * will be called in main thread
     *
     * @param what event type
     * @return ture if need render again
     */
    public boolean onEndedDrawing(int what) {
        if (what == DRAW_ANIMATING_FRAME) {
            boolean isAnimating = PageFlipLib.animating();
            // continue animating
            if (isAnimating) {
                mDrawCommand = DRAW_ANIMATING_FRAME;
                return true;
            }
            // animation is finished
            else {
                final int state = PageFlipLib.getFlipState();
                // update page number for mBackward flip
                if (state == PageFlipLib.END_WITH_BACKWARD) {
                    // don't do anything on page number since mPageNo is always
                    // represents the FIRST_TEXTURE no;
                }
                // update page number and switch textures for mForward flip
                else if (state == PageFlipLib.END_WITH_FORWARD) {
                    PageFlipLib.setFirstTextureWithSecond();
                    mPageNo++;
                }

                mDrawCommand = DRAW_FULL_PAGE;
                return true;
            }
        }
        return false;
    }

    /**
     * Draw page content
     *
     * @param number page number
     */
    private void drawPage(int number) {
        final int width = mCanvas.getWidth();
        final int height = mCanvas.getHeight();
        Paint p = new Paint();
        p.setFilterBitmap(true);

        // 1. draw background bitmap
        Bitmap background = LoadBitmapTask.get(mContext).getBitmap();
        Rect rect = new Rect(0, 0, width, height);
        mCanvas.drawBitmap(background, null, rect, p);
        background.recycle();

        // 2. draw page number
        int fontSize = calcFontSize(80);
        p.setColor(Color.WHITE);
        p.setStrokeWidth(1);
        p.setAntiAlias(true);
        p.setShadowLayer(5.0f, 8.0f, 8.0f, Color.BLACK);
        p.setTextSize(fontSize);
        String text = String.valueOf(number);
        float textWidth = p.measureText(text);
        float y = height - p.getTextSize() - 20;
        mCanvas.drawText(text, (width - textWidth) / 2, y, p);

        if (number <= 1) {
            String firstPage = "The First Page";
            p.setTextSize(calcFontSize(16));
            float w = p.measureText(firstPage);
            float h = p.getTextSize();
            mCanvas.drawText(firstPage, (width - w) / 2, y + 5 + h, p);
        }
        else if (number >= MAX_PAGES) {
            String lastPage = "The Last Page";
            p.setTextSize(calcFontSize(16));
            float w = p.measureText(lastPage);
            float h = p.getTextSize();
            mCanvas.drawText(lastPage, (width - w) / 2, y + 5 + h, p);
        }
    }

    /**
     * If page can flip mForward
     *
     * @return true if it can flip mForward
     */
    public boolean canFlipForward() {
        return (mPageNo < MAX_PAGES);
    }

    /**
     * If page can flip mBackward
     *
     * @return true if it can flip mBackward
     */
    public boolean canFlipBackward() {
        return (mPageNo > 1);
    }
}
