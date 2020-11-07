package com.docwei.mediaplayer;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;

import com.docwei.mediaplayer.bean.Mute;
import com.docwei.mediaplayer.listener.OnCompleteListener;
import com.docwei.mediaplayer.listener.OnPcmInfoListener;
import com.docwei.mediaplayer.listener.OnErrorListener;
import com.docwei.mediaplayer.listener.OnLoadListener;
import com.docwei.mediaplayer.listener.OnPlayStatusListener;
import com.docwei.mediaplayer.listener.OnPreparedListener;
import com.docwei.mediaplayer.listener.OnTimeInfoListener;
import com.docwei.mediaplayer.listener.OnTimeRecordListener;
import com.docwei.mediaplayer.listener.OnVolumnDBListener;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by liwk on 2020/9/9.
 */
public class MusicPlayer {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-58");
        System.loadLibrary("avdevice-58");
        System.loadLibrary("avfilter-7");
        System.loadLibrary("avformat-58");
        System.loadLibrary("avutil-56");
        System.loadLibrary("postproc-55");
        System.loadLibrary("swresample-3");
        System.loadLibrary("swscale-5");
    }

    private String source;
    private OnPreparedListener mOnPreparedListener;
    private OnLoadListener mOnLoadListener;
    private OnPlayStatusListener mOnPlayStatusListener;
    private OnTimeInfoListener mOnTimeInfoListener;
    private OnErrorListener mOnErrorListener;
    private OnCompleteListener mOnCompleteListener;
    private OnVolumnDBListener mOnVolumnDBListener;
    private OnTimeRecordListener mOnTimeRecordListener;
    private OnPcmInfoListener mOnPcmInfoListener;

    private boolean playNext = false;
    private int duration = -1;
    private int volumnPercent = 100;
    private int mute;
    private float speed;
    private float tune;
    private double recordTime = 0;
    private double sampleRate = 0;



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

    public void setOnVolumnDBListener(OnVolumnDBListener onVolumnDBListener) {
        mOnVolumnDBListener = onVolumnDBListener;
    }

    public void setOnTimeRecordListener(OnTimeRecordListener onTimeRecordListener) {
        mOnTimeRecordListener = onTimeRecordListener;
    }

    public void setOnPcmInfoListener(OnPcmInfoListener onCutAudioPlayListener) {
        mOnPcmInfoListener = onCutAudioPlayListener;
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

    public native void n_setSpeed(float speed);

    public native void n_setTune(float tune);

    private native void n_startstoprecord(boolean start);

    private native boolean n_cutAudioPlay(int startTime,int endTime,boolean showPcm);


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
        stop();
        if (mOnCompleteListener != null) {
            mOnCompleteListener.onComplete();
        }
    }

    public void onCallDB(int db) {
        if (mOnVolumnDBListener != null) {
            mOnVolumnDBListener.onSuccess(db);
        }
    }

    public void onCallNext() {
        if (playNext) {
            playNext = false;
            prepared();
        }
    }
    public void onCallPcmInfo(byte[] buffer,int buffersize){
        if(mOnPcmInfoListener!=null){
            mOnPcmInfoListener.onPcmInfo(buffer,buffersize);
        }
    }
    public void onCallPcmRate(int sampleRate){
        if(mOnPcmInfoListener!=null){
            mOnPcmInfoListener.onPcmRate(sampleRate,2,2);
        }
    }




    public void cutAudioPlay(int startTime, int endTime, boolean showPcm){
        if(n_cutAudioPlay(startTime,endTime,showPcm)){
            start();
        }else{
            stop();
            onCallError(2001,"cut audio params is error");
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
        duration = -1;
        stopRecord();
        n_stop();
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

    public void setSpeed(float speed) {
        this.speed = speed;
        n_setSpeed(speed);
    }

    public void setTune(float tune) {
        this.tune = tune;
        n_setTune(tune);
    }


    private static boolean initmeiacodec = false;

    private native int n_samplerate();


    public void startRecord(File outfile) {
        if (!initmediacodec) {
            sampleRate = n_samplerate();
            if (sampleRate > 0) {
                initmediacodec = true;
                initMediacodec(n_samplerate(), outfile);
                n_startstoprecord(true);
                Log.e("player", "开始录制");
            }
        }
    }

    public void stopRecord() {
        if (initmediacodec) {
            n_startstoprecord(false);
            releaseMedicacodec();
            Log.e("player", "完成录制");
        }
    }

    public void pauseRecord() {
        n_startstoprecord(false);
        Log.e("player", "暂停录制");
    }

    public void resumeRecord() {
        n_startstoprecord(true);
        Log.e("player", "继续录制");
    }


    private static boolean initmediacodec = false;

    //mediacodec

    private MediaFormat encoderFormat = null;
    private MediaCodec encoder = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo info = null;
    private int perpcmsize = 0;
    private byte[] outByteBuffer = null;
    private int aacsamplerate = 4;

    private void initMediacodec(int samperate, File outfile) {
        try {
            aacsamplerate = getADTSsamplerate(samperate);
            encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, samperate, 2);
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            info = new MediaCodec.BufferInfo();
            if (encoder == null) {
                Log.e("player", "craete encoder wrong");
                return;
            }
            recordTime = 0;
            encoder.configure(encoderFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(outfile);
            encoder.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void onCallEncodecPcmToAAC(int size, byte[] buffer) {
        if (buffer != null && encoder != null) {
            int inputBufferindex = encoder.dequeueInputBuffer(0);
            if (inputBufferindex >= 0) {
                ByteBuffer byteBuffer = encoder.getInputBuffers()[inputBufferindex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferindex, 0, size, 0, 0);
            }

            int index = encoder.dequeueOutputBuffer(info, 0);
            while (index >= 0) {
                try {
                    recordTime += size * 1.0 / (sampleRate * 2 * 16 / 8);
                    if(mOnTimeRecordListener!=null){
                        mOnTimeRecordListener.onTime(recordTime);
                    }
                    perpcmsize = info.size + 7;
                    outByteBuffer = new byte[perpcmsize];

                    ByteBuffer byteBuffer = encoder.getOutputBuffers()[index];
                    byteBuffer.position(info.offset);
                    byteBuffer.limit(info.offset + info.size);

                    addADtsHeader(outByteBuffer, perpcmsize, aacsamplerate);

                    byteBuffer.get(outByteBuffer, 7, info.size);
                    byteBuffer.position(info.offset);
                    outputStream.write(outByteBuffer, 0, perpcmsize);

                    encoder.releaseOutputBuffer(index, false);
                    index = encoder.dequeueOutputBuffer(info, 0);
                    outByteBuffer = null;
                    Log.e("player", "编码...");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate) {
        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSsamplerate(int samplerate) {
        int rate = 4;
        switch (samplerate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    private void releaseMedicacodec() {
        if (encoder == null) {
            return;
        }
        try {
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encoderFormat = null;
            info = null;
            initmediacodec = false;
            recordTime = 0;
            Log.e("player", "录制完成...");
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }


}
