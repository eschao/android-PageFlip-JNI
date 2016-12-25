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
 * Double pages render
 * <p>
 * Some key points here:
 * <ul>
 *     <li>First page is which page user is clicking on or moving by finger
 *          Sometimes it is left page on screen, sometimes it is right page.
 *          Second page is leftover page against the first page
 *     </li>
 *     <li>mPageNo is always the number of left page instead of first page</li>
 * </ul>
 * </p>
 * <p>
 * Every screen 'Page' contains 3 page contents, so it need 3 textures:
 * <ul>
 *     <li>First texture: first page content of this 'Page'</li>
 *     <li>Back texture: the second page content of this 'Page'</li>
 *     <li>Second texture: the third page content of this 'Page'</li>
 * </ul>
 * </p>
 *
 * @author eschao
 */

public class DoublePagesRender extends PageRender {

    /**
     * Constructor
     * @see {@link #PageRender(Context, PageFlip, Handler, int)}
     */
    public DoublePagesRender(Context context, Handler handler, int pageNo) {
        super(context, handler, pageNo);
    }

    /**
     * Draw page frame
     */
    public void onDrawFrame() {
        // 1. delete unused textures to save memory
        PageFlipLib.recycleTextures();

        // 2. check if the first texture is valid for first page, if not,
        // create it with relative content
        if (!PageFlipLib.isFirstTextureSet(true)) {
            drawPage(PageFlipLib.isLeftPage(true) ? mPageNo : mPageNo + 1);
            PageFlipLib.setFirstTexture(true, mBitmap);
        }

        // 3. check if the first texture is valid for second page
        if (!PageFlipLib.isFirstTextureSet(false)) {
            drawPage(PageFlipLib.isLeftPage(false) ? mPageNo : mPageNo + 1);
            PageFlipLib.setFirstTexture(false, mBitmap);
        }

        // 4. handle drawing command triggered from finger moving and animating
        if (mDrawCommand == DRAW_MOVING_FRAME ||
            mDrawCommand == DRAW_ANIMATING_FRAME) {
            // before drawing, check if back texture of first page is valid
            // Remember: the first page is always the fold page
            if (!PageFlipLib.isBackTextureSet(true)) {
                drawPage(PageFlipLib.isLeftPage(true) ? mPageNo - 1 :
                         mPageNo + 2);
                PageFlipLib.setBackTexture(true, mBitmap);
            }

            // check the second texture of first page is valid.
            if (!PageFlipLib.isSecondTextureSet(true)) {
                drawPage(PageFlipLib.isLeftPage(true) ? mPageNo - 2 :
                         mPageNo + 3);
                PageFlipLib.setSecondTexture(true, mBitmap);
            }

            // draw frame for page flip
            PageFlipLib.drawFlipFrame();
        }
        // draw stationary page without flipping
        else if (mDrawCommand == DRAW_FULL_PAGE){
            PageFlipLib.drawPageFrame();
        }

        // 5. send message to main thread to notify drawing is ended so that
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
        int pageW = PageFlipLib.getPageWidth(true);
        int pageH = PageFlipLib.getPageHeight(true);
        mBitmap = Bitmap.createBitmap(pageW, pageH, Bitmap.Config.ARGB_8888);
        mCanvas.setBitmap(mBitmap);
        LoadBitmapTask.get(mContext).set(pageW, pageH, 2);
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
                // should handle mForward flip to update page number and exchange
                // textures between first and second pages. Don'top have to handle
                // mBackward flip since there is no such state happened in double
                // page mode
                if (PageFlipLib.getFlipState() ==
                    PageFlipLib.END_WITH_FORWARD) {
                    PageFlipLib.swapSecondTexturesWithFirst();

                    // update page number for left page
                    if (PageFlipLib.isLeftPage(true)) {
                        mPageNo -= 2;
                    }
                    else {
                        mPageNo += 2;
                    }
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
        if (width > height) {
            mCanvas.rotate(90);
            mCanvas.drawBitmap(background, null, rect, p);
            mCanvas.rotate(-90);
        }
        else {
            mCanvas.drawBitmap(background, null, rect, p);
        }

        background.recycle();

        // 2. draw page number
        int fontSize = (int)(80 * mContext.getResources().getDisplayMetrics()
                                          .scaledDensity);
        p.setColor(Color.WHITE);
        p.setStrokeWidth(1);
        p.setAntiAlias(true);
        p.setShadowLayer(5.0f, 8.0f, 8.0f, Color.BLACK);
        p.setTextSize(fontSize);

        String text = String.valueOf(number);
        if (number < 1) {
            text = "Preface";
        }
        else if (number > MAX_PAGES) {
            text = "End";
        }
        float textWidth = p.measureText(text);
        float y = height - p.getTextSize() - 20;
        mCanvas.drawText(text, (width - textWidth) / 2, y, p);

        if (number == 1) {
            String firstPage = "The First Page";
            p.setTextSize(calcFontSize(16));
            float w = p.measureText(firstPage);
            float h = p.getTextSize();
            mCanvas.drawText(firstPage, (width - w) / 2, y + 5 + h, p);
        }
        else if (number == MAX_PAGES) {
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
        // current page is left page
        if (PageFlipLib.isLeftPage(true)) {
            return (mPageNo > 1);
        }

        // current page is right page
        return (mPageNo + 2 <= MAX_PAGES);
    }

    /**
     * Don'top need to handle mBackward flip
     *
     * @return always false
     */
    public boolean canFlipBackward() {
        return false;
    }
}
