package com.docwei.mediaplayer.listener;

/**
 * Created by liwk on 2020/11/7.
 */
public interface OnPcmInfoListener {
    void onPcmInfo(byte[] buffer,int buffersize);

    void onPcmRate(int sampleRate, int channel, int bit);
}
