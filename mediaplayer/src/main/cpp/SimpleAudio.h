//
// Created by docwei on 2020/9/10.
//

#ifndef SIMPLEMUSIC_SIMPLEAUDIO_H
#define SIMPLEMUSIC_SIMPLEAUDIO_H


#include "PlayStatus.h"
#include "queue"
#include "SimpleQueue.h"
#include "CallJava.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}


class SimpleAudio {

public :
    AVCodecParameters *codecParameters = NULL;
    AVCodecContext *avCodecContext = NULL;
    int streamIndex = -1;
    PlayStatus *playStatus = NULL;
    CallJava * calljava=NULL;
    SimpleQueue *queue = NULL;

    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate=0;

    double clock;
    double now_time;
    double last_time;
    double duration;
    AVRational time_base;
    int volumnPercent=100;

    //引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

   //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
   //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmPlayerVolume = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue;


    SimpleAudio(PlayStatus *playStatus,int sample_rate,CallJava* callJava);


    ~SimpleAudio();

    void play();

    int resampleAudio();

    void initOpenSLES();

    unsigned int getCurrentSampleRateForOpenSles(int sample_rate);

    void pause();
    void resume();

    void stop();
    void release();

    void setVolumn(int percent);
};


#endif //SIMPLEMUSIC_SIMPLEAUDIO_H
