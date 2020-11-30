//
// Created by docwei on 2020/11/25.
//

#ifndef SIMPLEMUSIC_SIMPLEVIDEO_H
#define SIMPLEMUSIC_SIMPLEVIDEO_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

#include "PlayStatus.h"
#include "queue"
#include "SimpleQueue.h"
#include "CallJava.h"

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
public :
    SimpleVideo(PlayStatus *playStatus, CallJava *callJava);

    ~SimpleVideo();

    void play();

    void release();
};


#endif //SIMPLEMUSIC_SIMPLEVIDEO_H
