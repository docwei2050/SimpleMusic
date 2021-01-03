//
// Created by docwei on 2020/10/22.
//

#include "SimpleQueue.h"

SimpleQueue::SimpleQueue(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    //初始化锁和锁条件
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);

}

int SimpleQueue::putAvPacket(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(packet);
    if (LOG_DEBUG) {
        LOGD("放入一个AVPacket到队列里面，个数为：%d", queuePacket.size());
    }
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int SimpleQueue::getAvPakcet(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    while (playStatus != NULL && !playStatus->exit) {
        if (queuePacket.size() > 0) {
            AVPacket *avPacket = queuePacket.front();
            if (av_packet_ref(packet, avPacket) == 0) {
                queuePacket.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            LOGD("从队列里面取出一个AVPacket,还剩个数为：%d", queuePacket.size());
            break;
        } else {
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int SimpleQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

void SimpleQueue::clearAvPacket() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()) {
        AVPacket *packet = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);

}

SimpleQueue::~SimpleQueue() {
    clearAvPacket();
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
}

void SimpleQueue::noticeQueue() {
   pthread_cond_signal(&condPacket);
}
