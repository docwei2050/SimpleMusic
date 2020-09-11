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

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

class FFmpegDecode {
public:
    CallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFortmatCtx = NULL;
    SimpleAudio *audio = NULL;

public:
    FFmpegDecode(CallJava *callJava, const char *url);

    ~FFmpegDecode();

    void prepared();

    void decodeFFmpegThread();
};


#endif //SIMPLEMUSIC_FFMPEGDECODE_H
