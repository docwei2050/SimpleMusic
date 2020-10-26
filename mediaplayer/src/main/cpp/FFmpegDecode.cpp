//
// Created by docwei on 2020/9/10.
//



#include "FFmpegDecode.h"


FFmpegDecode::FFmpegDecode(PlayStatus *playStatus, CallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->url = url;
    exit = false;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}

void *decodeFF(void *data) {
    FFmpegDecode *fFmpegDecode = (FFmpegDecode *) data;
    fFmpegDecode->decodeFFmpegThread();
    pthread_exit(&fFmpegDecode->decodeThread);
}

void FFmpegDecode::prepared() {
    pthread_create(&decodeThread, NULL, decodeFF, this);
}

int avformat_callback(void *ctx) {
    FFmpegDecode *fFmpegDecode = (FFmpegDecode *) ctx;
    if (fFmpegDecode->playStatus->exit) {
        return AVERROR_EOF;
    }
    return 0;
}

void FFmpegDecode::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);

    av_register_all();
    avformat_network_init();
    pFortmatCtx = avformat_alloc_context();

    pFortmatCtx->interrupt_callback.callback = avformat_callback;
    pFortmatCtx->interrupt_callback.opaque = this;


    if (avformat_open_input(&pFortmatCtx, url, NULL, NULL) != 0) {
        LOGE("can not open url:%s", url)
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");
        return;
    }

    if (avformat_find_stream_info(pFortmatCtx, NULL) < 0) {
        LOGE("can not find stream from  url:%s", url)
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1002, "can not find stream from  url");
        return;
    }

    for (int i = 0; i < pFortmatCtx->nb_streams; i++) {
        if (pFortmatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new SimpleAudio(playStatus, pFortmatCtx->streams[i]->codecpar->sample_rate,
                                        callJava);

                audio->streamIndex = i;
                audio->codecParameters = pFortmatCtx->streams[i]->codecpar;

                audio->duration = pFortmatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFortmatCtx->streams[i]->time_base;
                duration = audio->duration;

            }
        }
    }
    AVCodec *codec = avcodec_find_decoder(audio->codecParameters->codec_id);
    if (!codec) {
        LOGE("can not find decoder")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        return;
    }

    audio->avCodecContext = avcodec_alloc_context3(codec);
    if (!audio->avCodecContext) {
        LOGE("can not alloc new decoderCtx")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decoderCtx");
        return;
    }

    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecParameters) < 0) {
        LOGE("can not alloc new decoderCtx")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1005, "can not alloc new decoderCtx");
        return;
    }
    if (avcodec_open2(audio->avCodecContext, codec, 0) != 0) {
        LOGE("can not open audio streams")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1006, "can not open audio streams");
        return;
    }
    if (callJava != NULL) {

        if (playStatus != NULL && !playStatus->exit) {
            callJava->onCallPrepared(CHILD_THREAD);
        } else {
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);

}

void FFmpegDecode::start() {
    if (audio == NULL) {
        LOGE("audio is NULL")
        callJava->onCallError(CHILD_THREAD, 1007, "audio is NULL");
        return;
    }
    audio->play();
    int count = 0;
    while (playStatus != NULL && !playStatus->exit) {

        if (playStatus->seek) {
            continue;
        }
        if (audio->queue->getQueueSize() > 40) {
            continue;
        }


        AVPacket *avPacket = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(pFortmatCtx, avPacket);
        pthread_mutex_unlock(&seek_mutex);

        if (ret == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
                //解码操作
                count++;
                //LOGE("解码第%d帧", count);
                audio->queue->putAvPacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playStatus != NULL && !playStatus->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
        }
    }
    if (callJava != NULL) {
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;
    LOGE("解码完成")
}

void FFmpegDecode::pause() {
    if (audio != NULL) {
        audio->pause();
    }
}

void FFmpegDecode::resume() {
    if (audio != NULL) {
        audio->resume();
    }
}

void FFmpegDecode::release() {
    LOGE("开始释放ffmpeg")
    playStatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000) {
            exit = true;
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }
    LOGE("开始释放audio")
    if (audio != NULL) {
        audio->release();
        delete (audio);
        audio = NULL;
    }
    LOGE("开始释放封装格式上下文")
    if (pFortmatCtx != NULL) {
        avformat_close_input(&pFortmatCtx);
        avformat_free_context(pFortmatCtx);
        pFortmatCtx = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

FFmpegDecode::~FFmpegDecode() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);

}

void FFmpegDecode::seek(int64_t seconds) {
    if (duration < 0) {
        return;
    }
    if (seconds > 0 && seconds <= duration) {
        if (audio != NULL) {
            playStatus->seek = true;
            audio->queue->clearAvPacket();
            audio->clock = 0;
            audio->last_time = 0;
            //读取avpacket也用到了avFormatContext所以加锁
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = seconds * AV_TIME_BASE;
            avformat_seek_file(pFortmatCtx, -1, INT64_MIN, rel, INT_FAST64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playStatus->seek = false;
        }
    }

}

void FFmpegDecode::setVolumn(int percent) {
    if(audio!=NULL){
        audio->setVolumn(percent);
    }
}
