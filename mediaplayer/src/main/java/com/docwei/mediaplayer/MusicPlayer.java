package com.docwei.mediaplayer;

import android.text.TextUtils;
import android.util.Log;

import com.docwei.mediaplayer.listener.OnPreparedListener;

/**
 * Created by liwk on 2020/9/9.
 */
public class MusicPlayer {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avdevice");
        System.loadLibrary("avutil");
        System.loadLibrary("avresample");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }

    private String source;
    private OnPreparedListener mOnPreparedListener;

    public MusicPlayer() {

    }
    public void setSource(String source) {
        this.source = source;
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        mOnPreparedListener = onPreparedListener;
    }

    public void prepared(){
        if(TextUtils.isEmpty(source)){
            Log.e("player","source  is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();
    }
    public native void n_parpared(String source);

    public void onCallPrepared(){
        if(mOnPreparedListener!=null){
            mOnPreparedListener.onPrepared();
        }
    }
}
