package com.docwei.mediaplayer;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.docwei.mediaplayer.bean.Mute;
import com.docwei.mediaplayer.listener.OnCompleteListener;
import com.docwei.mediaplayer.listener.OnErrorListener;
import com.docwei.mediaplayer.listener.OnLoadListener;
import com.docwei.mediaplayer.listener.OnPlayStatusListener;
import com.docwei.mediaplayer.listener.OnPreparedListener;
import com.docwei.mediaplayer.listener.OnTimeInfoListener;
import com.docwei.mediaplayer.opengl.OriGlSurfaceView;
import com.docwei.mediaplayer.opengl.OriRender;
import com.docwei.mediaplayer.util.VideoUtil;

import java.nio.ByteBuffer;

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
    private boolean playNext = false;
    private int duration = -1;
    private int volumnPercent = 100;
    private int mute;
    private OriGlSurfaceView mOriGlSurfaceView;



    private MediaFormat mMediaFormat;
    private MediaCodec mMediacodec;
    private Surface mSurface;
    private MediaCodec.BufferInfo info;
    public MusicPlayer() {

    }

    public void setOriGlSurfaceView(OriGlSurfaceView oriGlSurfaceView) {
        mOriGlSurfaceView = oriGlSurfaceView;
        oriGlSurfaceView.getOriRender().setOnSurfaceCreateListener(new OriRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(Surface s) {
                if (mSurface == null) {
                    mSurface = s;
                    Log.e("simpleplayer", "onSurfaceCreate");
                }
            }
        });
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

    public OriGlSurfaceView getOriGlSurfaceView() {
        return mOriGlSurfaceView;
    }

    public void setVolumnPercent(int percent) {
        volumnPercent = percent;
        n_volumn(percent);
    }

    public int getVolumnPercent() {
        return volumnPercent;
    }


    public void prepared() {

        if (TextUtils.isEmpty(source)) {
            Log.e("player", "source  is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                duration = -1;
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
                setVolumnPercent(volumnPercent);
            }
        }).start();
    }

    public native void n_parpared(String source);

    public native void n_start();

    public native void n_resume();

    public native void n_pause();

    public native void n_stop();

    public native void n_seek(int seconds);

    public native int n_duration();

    public native void n_volumn(int percent);

    public native void n_mute(int mute);


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

    public void onCallNext() {
        if (playNext) {
            playNext = false;
            prepared();
        }
    }

    public void onCallRenderYUV(int width,int height,byte[] y,byte[] u,byte[] v){
        if(mOriGlSurfaceView!=null){
            mOriGlSurfaceView.getOriRender().setRenderType(OriRender.RENER_YUV);
            mOriGlSurfaceView.setYUVData(width,height,y,u,v);
        }
    }


    public  boolean onCallIsSupportCodecType(String ffType){
        return VideoUtil.isSupportCodecType(ffType);
    }


    public void initMediaCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1) {
        if (mSurface != null) {
            try {
                mOriGlSurfaceView.getOriRender().setRenderType(OriRender.RENDER_MEDIACODEC);
                String mime = VideoUtil.findVideoCodecName(codecName);
                mMediaFormat = MediaFormat.createVideoFormat(mime, width, height);
                mMediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mMediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mMediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                mMediacodec = MediaCodec.createDecoderByType(mime);

                info = new MediaCodec.BufferInfo();
                mMediacodec.configure(mMediaFormat, mSurface, null, 0);
                mMediacodec.start();

            } catch (Exception e) {
//                e.printStackTrace();
            }
        } else {
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(2001, "surface is null");
            }
        }
    }

    public void decodeAVPacket(int datasize, byte[] data) {
        if (mSurface != null && datasize > 0 && data != null && mMediacodec != null) {
            try {
                int intputBufferIndex = mMediacodec.dequeueInputBuffer(10);
                if (intputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = mMediacodec.getInputBuffers()[intputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(data);
                    mMediacodec.queueInputBuffer(intputBufferIndex, 0, datasize, 0, 0);
                }
                int outputBufferIndex = mMediacodec.dequeueOutputBuffer(info, 10);
                while (outputBufferIndex >= 0) {
                    mMediacodec.releaseOutputBuffer(outputBufferIndex, true);
                    outputBufferIndex = mMediacodec.dequeueOutputBuffer(info, 10);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

    }
    private void releaseMediacodec(){
        if(mMediacodec!=null){
            try{
                mMediacodec.flush();
                mMediacodec.stop();
                mMediacodec.release();

            }catch (Exception e){
                e.printStackTrace();
            }
            mMediacodec=null;
            mMediaFormat=null;
            info=null;
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
        duration=0;
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
                releaseMediacodec();
            }
        }).start();

    }


    public void resume() {
        n_resume();
        if (mOnPlayStatusListener != null) {
            mOnPlayStatusListener.onPause(false);
        }
    }

    public void playNext(String url) {
        source = url;
        playNext = true;
        stop();
    }

    public int getDuration() {
        if (duration < 0) {
            duration = n_duration();
            return duration;
        }
        return duration;
    }

    public void setMute(Mute mute) {
        this.mute = mute.getValue();
        n_mute(mute.getValue());
    }
}
