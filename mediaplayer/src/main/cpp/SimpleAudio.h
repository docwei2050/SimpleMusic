//
// Created by docwei on 2020/9/10.
//

#ifndef SIMPLEMUSIC_SIMPLEAUDIO_H
#define SIMPLEMUSIC_SIMPLEAUDIO_H


#include "PlayStatus.h"
#include "queue"
#include "SimpleQueue.h"

extern "C" {
#include <libavcodec/avcodec.h>
};

class SimpleAudio {

public :
    AVCodecParameters *codecParameters = NULL;
    AVCodecContext *avCodecContext = NULL;
    int streamIndex = -1;
    PlayStatus *playStatus = NULL;
    SimpleQueue *queue = NULL;

    SimpleAudio(PlayStatus* playStatus);

    ~SimpleAudio();
};


#endif //SIMPLEMUSIC_SIMPLEAUDIO_H
