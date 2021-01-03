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
#include "SimpleVideo.h"

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
    SimpleVideo *video=NULL;
    PlayStatus *playStatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit=false;
    int duration=0;
    pthread_mutex_t  seek_mutex;

    bool isSupportMediaCodec=false;

    //给avPacket添加头信息方便MedieCodec去解析
    const AVBitStreamFilter  *bitStreamFilter=NULL;

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

    void setMute(int mute);

    //这里会生成avCodecContext
    int getCodecContext(AVCodecParameters * parameters,AVCodecContext **avCodecContext);
};


#endif //SIMPLEMUSIC_FFMPEGDECODE_H
