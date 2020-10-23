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
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
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
    public void start(){
        if(TextUtils.isEmpty(source)){
            Log.e("player","source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }
    public native void n_parpared(String source);
    public native void n_start();

    //C++在子线程中回调这个方法
    public void onCallPrepared(){
        if(mOnPreparedListener!=null){
            mOnPreparedListener.onPrepared();
        }
    }
}
