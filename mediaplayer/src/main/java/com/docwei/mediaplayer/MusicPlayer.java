package com.docwei.mediaplayer;

import android.text.TextUtils;
import android.util.Log;

import com.docwei.mediaplayer.listener.OnCompleteListener;
import com.docwei.mediaplayer.listener.OnErrorListener;
import com.docwei.mediaplayer.listener.OnLoadListener;
import com.docwei.mediaplayer.listener.OnPlayStatusListener;
import com.docwei.mediaplayer.listener.OnPreparedListener;
import com.docwei.mediaplayer.listener.OnTimeInfoListener;

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
    private OnLoadListener mOnLoadListener;
    private OnPlayStatusListener mOnPlayStatusListener;
    private OnTimeInfoListener mOnTimeInfoListener;
    private OnErrorListener mOnErrorListener;
    private OnCompleteListener mOnCompleteListener;
    private  boolean playNext=false;


    public MusicPlayer() {

    }

    public void setSource(String source) {
        this.source = source;
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        mOnPreparedListener = onPreparedListener;
    }

    public void setOnLoadListener(OnLoadListener onLoadListener) {
        mOnLoadListener = onLoadListener;
    }

    public void setOnPlayStatusListener(OnPlayStatusListener onPlayStatusListener) {
        mOnPlayStatusListener = onPlayStatusListener;
    }

    public void setOnTimeInfoListener(OnTimeInfoListener onTimeInfoListener) {
        mOnTimeInfoListener = onTimeInfoListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        mOnErrorListener = onErrorListener;
    }

    public void setOnCompleteListener(OnCompleteListener onCompleteListener) {
        mOnCompleteListener = onCompleteListener;
    }

    public void prepared() {

        if (TextUtils.isEmpty(source)) {
            Log.e("player", "source  is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();
    }

    public void start() {
        if (TextUtils.isEmpty(source)) {
            Log.e("player", "source is empty");
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

    public native void n_resume();

    public native void n_pause();

    public native void n_stop();

    public native void n_seek(int seconds);

    //C++在子线程中回调这个方法
    public void onCallPrepared() {
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if (mOnLoadListener != null) {
            mOnLoadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int curr, int total) {
        if (mOnTimeInfoListener != null) {
            mOnTimeInfoListener.onTime(curr, total);
            Log.e("player", "onTimeInfo" + total + "---" + curr);
        }
    }

    public void onCallError(int code, String message) {
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(code, message);
        }
    }

    public void onCallComplete() {
        if (mOnCompleteListener != null) {
            mOnCompleteListener.onComplete();
        }
    }
    public void onCallNext(){
        if(playNext){
            playNext=false;
            prepared();
        }
    }


    public void seek(int seconds) {
        n_seek(seconds);
    }


    public void pause() {
        n_pause();
        if (mOnPlayStatusListener != null) {
            mOnPlayStatusListener.onPause(true);
        }
    }

    public void stop() {
        n_stop();
    }


    public void resume() {
        n_resume();
        if (mOnPlayStatusListener != null) {
            mOnPlayStatusListener.onPause(false);
        }
    }

    public void playNext(String url) {
         source=url;
         playNext=true;
         stop();
    }
}
