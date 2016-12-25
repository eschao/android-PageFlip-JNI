package com.eschao.android.widget.jni.pageflip;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;

public class PageFlipLib {

    public static int BEGIN_FLIP        = 0;
    public static int FORWARD_FLIP      = 1;
    public static int BACKWARD_FLIP     = 2;
    public static int RESTORE_FLIP      = 3;
    public static int END_FLIP          = 4;
    public static int END_WITH_FORWARD  = 5;
    public static int END_WITH_BACKWARD = 6;
    public static int END_WITH_RESTORE  = 7;

    static {
        System.loadLibrary("pageflip");
    }

    private static OnPageFlipListener mListener = null;
    public static void setListener(OnPageFlipListener listener) {
        mListener = listener;
    }

    public static boolean onFingerMove(float x, float y) {
        final boolean canForward = (mListener != null &&
                                    mListener.canFlipForward());
        final boolean canBackward = (mListener != null &&
                                     mListener.canFlipBackward());
        return onFingerMove(x, y, canForward, canBackward);
    }

    public static boolean onFingerUp(float x, float y, int duration) {
        final boolean canForward = (mListener != null &&
                                    mListener.canFlipForward());
        final boolean canBackward = (mListener != null &&
                                     mListener.canFlipBackward());
        return onFingerUp(x, y, duration, canForward, canBackward);
    }

    public static native boolean init();
    public static native boolean release();
    public static native int enableAutoPage(boolean isAuto);
    public static native boolean isAutoPageEnabled();
    public static native int enableClickToFlip(boolean enable);
    public static native int setWidthRatioOfClickToFlip(float ratio);
    public static native int setPixelsOfMesh(int pixelsOfMesh);
    public static native int setSemiPerimeterRatio(float ratio);
    public static native int setMaskAlphaOfFold(int alpha);
    public static native int setShadowColorOfFoldEdges(float startColor,
                                                       float startAlpha,
                                                       float endColor,
                                                       float endAlpha);
    public static native int setShadowColorOfFoldBase(float startColor,
                                                      float startAlpha,
                                                      float endColor,
                                                      float endAlpha);
    public static native int setShadowWidthOfFoldEdges(float min,
                                                       float max,
                                                       float ratio);
    public static native int setShadowWidthOfFoldBase(float min,
                                                      float max,
                                                      float ratio);
    public static native int getPixelsOfMesh();
    public static native int getSurfaceWidth();
    public static native int getSurfaceHeight();
    public static native int onSurfaceCreated();
    public static native int onSurfaceChanged(int width, int height);

    public static native boolean animating();
    public static native boolean canAnimate(float x, float y);
    public static native boolean isAnimating();
    public static native int abortAnimating();

    public static native int drawFlipFrame();
    public static native int drawPageFrame();

    public static native boolean hasFirstPage();
    public static native boolean hasSecondPage();
    public static native boolean isFirstTextureSet(boolean isFirstPage);
    public static native boolean isSecondTextureSet(boolean isFirstPage);
    public static native boolean isBackTextureSet(boolean isFirstPage);
    public static native int setFirstTexture(boolean isFirstPage, Bitmap b);
    public static native int setSecondTexture(boolean isFirstPage, Bitmap b);
    public static native int setBackTexture(boolean isFirstPage, Bitmap b);
    public static native int setGradientLightTexture(Bitmap b);
    public static native int setFirstTextureWithSecond();
    public static native int setSecondTextureWithFirst();
    public static native int swapSecondTexturesWithFirst();
    public static native int recycleTextures();
    public static native boolean onFingerDown(float x, float y);

    public static native int getPageWidth(boolean isFirstPage);
    public static native int getPageHeight(boolean isFirstPage);
    public static native boolean isLeftPage(boolean isFirstPage);
    public static native boolean isRightPage(boolean isFirstPage);

    public static native int getFlipState();
    private static native boolean onFingerMove(float x, float y,
                                               boolean canForward,
                                               boolean canBackward);
    private static native boolean onFingerUp(float x, float y, int duration,
                                             boolean canForward,
                                             boolean canBackward);

    /**
     * Create gradient bitmap for drawing lighting effect on back of fold page
     *
     * @return gradient bitmap object
     */
    public static Bitmap createGradientBitmap() {
        Canvas c = new Canvas();
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        Bitmap bitmap = Bitmap.createBitmap(256, 1, Bitmap.Config.ARGB_8888);

        c.setBitmap(bitmap);
        int[] colors = new int[]{0x00FFFFFF,
                                 0x24000000,
                                 0x24101010,
                                 0x48000000};
        float[] positions = new float[]{0.5f, 0.9f, 0.94f, 1.0f};
        LinearGradient shader = new LinearGradient(0, 0, 256, 0, colors,
                                                   positions,
                                                   Shader.TileMode.CLAMP);
        paint.setShader(shader);
        c.drawRect(0, 0, 256, 1, paint);
        return bitmap;
    }

    // Error definition
    public static final int OK                             = 0;
    public static final int ERROR                          = OK - 1;
    public static final int ERR_PAGE_FLIP_UNINIT           = OK - 2;
    public static final int ERR_NULL_PARAMETER             = OK - 3;
    public static final int ERR_INVALID_PARAMETER          = OK - 4;
    public static final int ERR_GL_ERROR                   = OK - 5;
    public static final int ERR_GL_COMPILE_SHADER          = OK - 6;
    public static final int ERR_GL_LINK_PROGRAM            = OK - 7;
    public static final int ERR_GL_CREATE_SHADER_REF       = OK - 8;
    public static final int ERR_GL_CREATE_PROGRAM_REF      = OK - 9;
    public static final int ERR_GL_ATTACH_SHADER           = OK - 10;
    public static final int ERR_GL_ATTACH_FRAGMENT         = OK - 11;
    public static final int ERR_UNSUPPORT_BITMAP_FORMAT    = OK - 12;
    public static final int ERR_GET_BITMAP_INFO            = OK - 13;
    public static final int ERR_GET_BITMAP_DATA            = OK - 14;
    public static final int ERR_NO_TWO_PAGES               = OK - 15;
    public static final int ERR_NULL_PAGE                  = OK - 16;

    public static native int getError();
}
