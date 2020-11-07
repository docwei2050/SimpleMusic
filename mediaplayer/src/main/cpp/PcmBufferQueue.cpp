//
// Created by ywl on 2017-12-3.
//

#include "PcmBufferQueue.h"
#include "AndroidLog.h"

PcmBufferQueue::PcmBufferQueue(PlayStatus *playStatus) {
    wlPlayStatus = playStatus;
    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);
}

PcmBufferQueue::~PcmBufferQueue() {
    wlPlayStatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    if(LOG_DEBUG)
    {
        LOGE("PcmBufferQueue 释放完了");
    }
}

void PcmBufferQueue::release() {

    if(LOG_DEBUG)
    {
        LOGE("PcmBufferQueue::release");
    }
    noticeThread();
    clearBuffer();

    if(LOG_DEBUG)
    {
        LOGE("PcmBufferQueue::release success");
    }
}

int PcmBufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    PcmBean *pcmBean = new PcmBean(buffer, size);
    queueBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int PcmBufferQueue::getBuffer(PcmBean **pcmBean) {

    pthread_mutex_lock(&mutexBuffer);

    while(wlPlayStatus != NULL && !wlPlayStatus->exit)
    {
        if(queueBuffer.size() > 0)
        {
            *pcmBean = queueBuffer.front();
            queueBuffer.pop_front();
            break;
        } else{
            if(!wlPlayStatus->exit)
            {
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int PcmBufferQueue::clearBuffer() {

    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!queueBuffer.empty())
    {
        PcmBean *pcmBean = queueBuffer.front();
        queueBuffer.pop_front();
        delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int PcmBufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;
}


int PcmBufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}

