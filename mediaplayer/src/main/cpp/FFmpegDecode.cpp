//
// Created by yangw on 2018-2-28.
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

void *decodeFFmpeg(void *data) {
    FFmpegDecode *wlFFmpeg = (FFmpegDecode *) data;
    wlFFmpeg->decodeFFmpegThread();
    pthread_exit(&wlFFmpeg->decodeThread);
}

void FFmpegDecode::prepared() {

    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);

}

int avformat_callback(void *ctx) {
    FFmpegDecode *fFmpeg = (FFmpegDecode *) ctx;
    if (fFmpeg->playStatus->exit) {
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
        if (LOG_DEBUG) {
            LOGE("can not open url :%s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avformat_find_stream_info(pFortmatCtx, NULL) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not find streams from %s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1002, "can not find streams from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    for (int i = 0; i < pFortmatCtx->nb_streams; i++) {
        if (pFortmatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//得到音频流
        {
            if (audio == NULL) {
                audio = new SimpleAudio(playStatus, pFortmatCtx->streams[i]->codecpar->sample_rate,
                                        callJava);
                audio->streamIndex = i;
                audio->codecParameters = pFortmatCtx->streams[i]->codecpar;
                audio->duration = pFortmatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFortmatCtx->streams[i]->time_base;
                duration = audio->duration;
                callJava->onCallPcmRate(CHILD_THREAD,audio->sample_rate);
            }
        }
    }

    AVCodec *dec = avcodec_find_decoder(audio->codecParameters->codec_id);
    if (!dec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    audio->avCodecContext = avcodec_alloc_context3(dec);
    if (!audio->avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decodecctx");
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecParameters) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decodecctx");
        }
        callJava->onCallError(CHILD_THREAD, 1005, "ccan not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avcodec_open2(audio->avCodecContext, dec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        callJava->onCallError(CHILD_THREAD, 1006, "cant not open audio strames");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
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
        return;
    }
    audio->play();
    while (playStatus != NULL && !playStatus->exit) {
        if (playStatus->seek) {
            av_usleep(1000*100);
            continue;
        }

        if (audio->queue->getQueueSize() > 8) {
            av_usleep(1000*100);
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFortmatCtx, avPacket) == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
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
                    av_usleep(1000*100);
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
            break;
        }
    }
    if (callJava != NULL) {
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;

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

    if (LOG_DEBUG) {
        LOGE("开始释放Ffmpe");
    }
    playStatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000) {
            exit = true;
        }
        if (LOG_DEBUG) {
            LOGE("wait ffmpeg  exit %d", sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }

    if (LOG_DEBUG) {
        LOGE("释放 Audio");
    }

    if (audio != NULL) {
        audio->release();
        delete (audio);
        audio = NULL;
    }

    if (LOG_DEBUG) {
        LOGE("释放 封装格式上下文");
    }
    if (pFortmatCtx != NULL) {
        avformat_close_input(&pFortmatCtx);
        avformat_free_context(pFortmatCtx);
        pFortmatCtx = NULL;
    }
    if (LOG_DEBUG) {
        LOGE("释放 callJava");
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (LOG_DEBUG) {
        LOGE("释放 playStatus");
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

void FFmpegDecode::seek(int64_t secds) {

    if (duration <= 0) {
        return;
    }
    if (secds >= 0 && secds <= duration) {
        if (audio != NULL) {
            playStatus->seek = true;
            audio->queue->clearAvPacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = secds * AV_TIME_BASE;
            avcodec_flush_buffers(audio->avCodecContext);
            avformat_seek_file(pFortmatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playStatus->seek = false;
        }
    }
}

void FFmpegDecode::setVolumn(int percent) {
    if (audio != NULL) {
        audio->setVolume(percent);
    }
}

void FFmpegDecode::setMute(int mute) {
    if (audio != NULL) {
        audio->setMute(mute);
    }
}

void FFmpegDecode::setTune(float pitch) {

    if (audio != NULL) {
        audio->setTune(pitch);
    }

}

void FFmpegDecode::setSpeed(float speed) {

    if (audio != NULL) {
        audio->setSpeed(speed);
    }

}

int FFmpegDecode::getSampleRate() {
    if (audio != NULL) {
        return audio->avCodecContext->sample_rate;
    }
    return 0;
}

void FFmpegDecode::startStopRecord(bool start) {
    if (audio != NULL) {
        audio->startStopRecord(start);
    }

}

bool FFmpegDecode::cutAudioPlay(int startTime, int endTime, bool showPcm) {

    if(startTime>=0&&endTime<=duration&&startTime<endTime){
        audio->isCut=true;
        audio->endTime=endTime;
        audio->showPcm=showPcm;
        seek(startTime);
        return true;
    }
    return false;
}
