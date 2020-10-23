//
// Created by docwei on 2020/10/22.
//

#ifndef SIMPLEMUSIC_SIMPLEQUEUE_H
#define SIMPLEMUSIC_SIMPLEQUEUE_H

#include <jni.h>
#include <stddef.h>
#include "queue"
#include "pthread.h"
#include "AndroidLog.h"
#include "PlayStatus.h"
extern "C"{
#include "libavcodec/avcodec.h"
}
using namespace std;
class SimpleQueue {
public:
  queue<AVPacket *> queuePacket;
  pthread_mutex_t mutexPacket;
  pthread_cond_t condPacket;
  PlayStatus * playStatus=NULL;
public:
    SimpleQueue(PlayStatus * playStatus);
    ~SimpleQueue();

    int putAvPacket(AVPacket *packet);
    int getAvPakcet(AVPacket *packet);
    int getQueueSize();
};


#endif //SIMPLEMUSIC_SIMPLEQUEUE_H
