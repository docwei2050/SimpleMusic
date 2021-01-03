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
    return 0;
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
        } else if (pFortmatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video == NULL) {
                video = new SimpleVideo(playStatus, callJava);
                video->streamIndex = i;
                video->codecParameters = pFortmatCtx->streams[i]->codecpar;
                video->time_base = pFortmatCtx->streams[i]->time_base;

                //获取帧数
                int num = pFortmatCtx->streams[i]->avg_frame_rate.num;
                int den = pFortmatCtx->streams[i]->avg_frame_rate.den;
                if (num != 0 && den != 0) {
                    int fps = num / den;
                    video->defaultDelayTime = 1.0 / fps;
                }

            }
        }
    }

    if (audio != NULL) {
        getCodecContext(audio->codecParameters, &audio->avCodecContext);
    }
    if (video != NULL) {
        getCodecContext(video->codecParameters, &video->avCodecContext);
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
    video->audio = audio;
    const char *codecName = ((const AVCodec *) video->avCodecContext->codec)->name;
    if (callJava->onCallIsSupportCodecType(codecName)) {
        isSupportMediaCodec = true;
        if (strcasecmp(codecName, "h264") == 0) {
            bitStreamFilter = av_bsf_get_by_name("h264_mp4toannexb");
        } else if (strcasecmp(codecName, "h265") == 0) {
            bitStreamFilter = av_bsf_get_by_name("hevc_mp4toannexb");
        }
        if (bitStreamFilter == NULL) {
            isSupportMediaCodec = false;
            goto end;
        }
        if (av_bsf_alloc(bitStreamFilter, &video->abs_ctx) != 0) {
            isSupportMediaCodec = false;
            goto end;
        }
        if (avcodec_parameters_copy(video->abs_ctx->par_in, video->codecParameters) < 0) {
            isSupportMediaCodec = false;
            av_bsf_free(&video->abs_ctx);
            video->abs_ctx = NULL;
            goto end;
        }
        if (av_bsf_init(video->abs_ctx) != 0) {
            isSupportMediaCodec = false;
            av_bsf_free(&video->abs_ctx);
            video->abs_ctx = NULL;
            goto end;
        }
        video->abs_ctx->time_base_in = video->time_base;
    }
    end:
    if(isSupportMediaCodec){
        video->codecType = CODEC_MEDIACODEC;
        video->callJava->onCallInitMediacodec(
                codecName,
                video->avCodecContext->width,
                video->avCodecContext->height,
                video->avCodecContext->extradata_size,
                video->avCodecContext->extradata_size,
                video->avCodecContext->extradata,
                video->avCodecContext->extradata
                );
    }
    audio->play();
    video->play();
    int count = 0;
    while (playStatus != NULL && !playStatus->exit) {

        if (playStatus->seek) {
            av_usleep(100 * 1000);
            continue;
        }
        if (audio->queue->getQueueSize() > 40) {
            av_usleep(100 * 1000);
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
            } else if (avPacket->stream_index == video->streamIndex) {
                video->queue->putAvPacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playStatus != NULL && !playStatus->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    av_usleep(100 * 1000);
                    continue;
                } else {
                    if (!playStatus->seek) {
                        //seek时会清空buffer
                        playStatus->exit = true;
                    }
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
    if(playStatus!=NULL){
        playStatus->pause=true;
    }
    if (audio != NULL) {
        audio->pause();
    }
}

void FFmpegDecode::resume() {
    if(playStatus!=NULL){
        playStatus->pause=false;
    }
    if (audio != NULL) {
        audio->resume();
    }
}

void FFmpegDecode::release() {
    LOGE("开始释放ffmpeg")
    playStatus->exit = true;
    pthread_join(decodeThread,NULL);
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
    LOGE("开始释放video")
    if (video != NULL) {
        video->release();
        delete (video);
        video = NULL;
    }
    LOGE("开始释放封装格式上下文")
    if (pFortmatCtx != NULL) {
        avformat_close_input(&pFortmatCtx);
        avformat_free_context(pFortmatCtx);
        pFortmatCtx = NULL;
    }
    LOGE("开始释放callJava")
    if (callJava != NULL) {
        callJava = NULL;
    }
    LOGE("开始释放playStatus")
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    LOGE("开始释放init_mutex")
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
        playStatus->seek = true;
        //读取avpacket也用到了avFormatContext所以加锁
        pthread_mutex_lock(&seek_mutex);
        int64_t rel = seconds * AV_TIME_BASE;
        avformat_seek_file(pFortmatCtx, -1, INT64_MIN, rel, INT_FAST64_MAX, 0);
        if (audio != NULL) {

            audio->queue->clearAvPacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&audio->codecMutex);
            avcodec_flush_buffers(audio->avCodecContext);
            pthread_mutex_unlock(&audio->codecMutex);

        }
        if(video!=NULL){
            video->queue->clearAvPacket();
            video->clock=0;
            pthread_mutex_lock(&video->codecMutex);
            avcodec_flush_buffers(video->avCodecContext);
            pthread_mutex_unlock(&video->codecMutex);
        }
        pthread_mutex_unlock(&seek_mutex);
        playStatus->seek = false;
    }


}

void FFmpegDecode::setVolumn(int percent) {
    if (audio != NULL) {
        audio->setVolumn(percent);
    }
}

void FFmpegDecode::setMute(int mute) {
    if (audio != NULL) {
        audio->setMute(mute);
    }
}

int FFmpegDecode::getCodecContext(AVCodecParameters *parameters, AVCodecContext **avCodecContext) {
    AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
    if (!codec) {
        LOGE("can not find decoder")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        return -1;
    }

    *avCodecContext = avcodec_alloc_context3(codec);
    if (!audio->avCodecContext) {
        LOGE("can not alloc new decoderCtx")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decoderCtx");
        return -1;
    }

    if (avcodec_parameters_to_context(*avCodecContext, parameters) < 0) {
        LOGE("can not alloc new decoderCtx")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1005, "can not alloc new decoderCtx");
        return -1;
    }
    if (avcodec_open2(*avCodecContext, codec, 0) != 0) {
        LOGE("can not open audio streams")
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        callJava->onCallError(CHILD_THREAD, 1006, "can not open audio streams");
        return -1;
    }
    return 0;
}
