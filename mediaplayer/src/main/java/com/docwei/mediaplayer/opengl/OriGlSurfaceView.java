package com.docwei.mediaplayer.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * Created by liwk on 2020/12/2.
 */
public class OriGlSurfaceView extends GLSurfaceView {
    private OriRender mOriRender;

    public OriGlSurfaceView(Context context) {
        this(context, null);
    }

    public OriGlSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mOriRender = new OriRender(getContext());
        setRenderer(mOriRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        mOriRender.setOnRenderListener(new OriRender.OnRenderListener() {
            @Override
            public void onRender() {
                requestRender();
            }
        });
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v) {
        if (mOriRender != null) {
            mOriRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }

    public OriRender getOriRender() {
        return mOriRender;
    }
}
