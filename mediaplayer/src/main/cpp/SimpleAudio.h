//
// Created by docwei on 2020/9/10.
//

#ifndef SIMPLEMUSIC_SIMPLEAUDIO_H
#define SIMPLEMUSIC_SIMPLEAUDIO_H


#include "PlayStatus.h"
#include "queue"
#include "SimpleQueue.h"
#include "CallJava.h"
#include "SoundTouch.h"
#include "PcmBufferQueue.h"
#include "PcmBean.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}
using namespace soundtouch;

class SimpleAudio {

public :
    AVCodecParameters *codecParameters = NULL;
    AVCodecContext *avCodecContext = NULL;
    int streamIndex = -1;
    PlayStatus *playStatus = NULL;
    CallJava *callJava = NULL;
    SimpleQueue *queue = NULL;

    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    double clock;
    double now_time;
    double last_time;
    double duration;
    AVRational time_base;
    int volumePercent = 100;
    int mute = 2;


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
    SLVolumeItf pcmVolumePlay = NULL;
    SLMuteSoloItf pcmPlayerMuteSolo = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue;

    //soundTouch相关
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    float speed = 1.0f;
    float tune = 1.0f;
    int nb = 0;
    int num = 0;
    uint8_t *out_buffer = NULL;
    bool finished =true;
    bool isRecordPcm=false;
    bool readFrameFinished=true;


    bool isCut=false;
    int endTime=0;
    bool showPcm=false;

    pthread_t pcmCallBackThread;
    PcmBufferQueue *bufferQueue = NULL;
    int defaultPcmSize = 4096;

    SimpleAudio(PlayStatus *playStatus, int sample_rate, CallJava *callJava);


    ~SimpleAudio();

    void play();

    int resampleAudio(void **pcmbuf);

    void initOpenSLES();

    unsigned int getCurrentSampleRateForOpenSles(int sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);

    void setSpeed(float speed);

    void setTune(float tune);

    //pcm数据是uint_8 而soundTouch至少是uint_16
    int getSoundTouchData();

    int getPCMdB(char* pcmData,size_t pcmsize);

    void startStopRecord(bool start);
};


#endif //SIMPLEMUSIC_SIMPLEAUDIO_H
