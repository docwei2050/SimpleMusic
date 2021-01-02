//
// Created by docwei on 2020/11/25.
//

#ifndef SIMPLEMUSIC_SIMPLEVIDEO_H
#define SIMPLEMUSIC_SIMPLEVIDEO_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#include "PlayStatus.h"
#include "queue"
#include "SimpleQueue.h"
#include "CallJava.h"
#include "SimpleAudio.h"

class SimpleVideo {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecParameters = NULL;
    SimpleQueue *queue = NULL;
    PlayStatus *playStatus = NULL;
    CallJava *callJava = NULL;
    AVRational time_base;
    pthread_t thread_play;
    SimpleAudio  *audio;
    //因为有时候pts是0，所以要记录这个clock
    double clock;
    double delayTime;
    //默认的延迟事件是基于fps来获取的，比如1s 25帧，那么就是0.04
    double defaultDelayTime=0.04;

    pthread_mutex_t  codecMutex;
public :
    SimpleVideo(PlayStatus *playStatus, CallJava *callJava);

    ~SimpleVideo();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame);

    //针对diff时间去调整延迟时间
    double getDelayTime(double diff);
};


#endif //SIMPLEMUSIC_SIMPLEVIDEO_H
