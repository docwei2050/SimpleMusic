//
// Created by ywl on 2017-12-3.
//

#ifndef WLPLAYER_BUFFERQUEUE_H
#define WLPLAYER_BUFFERQUEUE_H

#include "deque"
#include "Playstatus.h"
#include "PcmBean.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class PcmBufferQueue {

public:
    std::deque<PcmBean *> queueBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    PlayStatus *wlPlayStatus = NULL;

public:
    PcmBufferQueue(PlayStatus *playStatus);
    ~PcmBufferQueue();
    int putBuffer(SAMPLETYPE *buffer, int size);
    int getBuffer(PcmBean **pcmBean);
    int clearBuffer();

    void release();
    int getBufferSize();

    int noticeThread();
};


#endif //WLPLAYER_BUFFERQUEUE_H
