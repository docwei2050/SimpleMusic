//
// Created by docwei on 2020/9/10.
//

#ifndef SIMPLEMUSIC_FFMPEGDECODE_H
#define SIMPLEMUSIC_FFMPEGDECODE_H

#include "CallJava.h"
#include <jni.h>
#include <stddef.h>
#include "pthread.h"
#include "SimpleAudio.h"
#include "PlayStatus.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

class FFmpegDecode {
public:
    CallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFortmatCtx = NULL;
    SimpleAudio *audio = NULL;
    PlayStatus *playStatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit=false;
    int duration=0;
    pthread_mutex_t  seek_mutex;

public:
    FFmpegDecode(PlayStatus *playStatus, CallJava *callJava, const char *url);

    ~FFmpegDecode();

    void prepared();

    void decodeFFmpegThread();

    void start();

    void pause();
    void resume();
    void release();
    void seek(int64_t seconds);


    void setVolumn(int percent);
};


#endif //SIMPLEMUSIC_FFMPEGDECODE_H
