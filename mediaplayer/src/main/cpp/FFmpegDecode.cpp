//
// Created by docwei on 2020/9/10.
//


#include "FFmpegDecode.h"


FFmpegDecode::FFmpegDecode(PlayStatus * playStatus,CallJava *callJava, const char *url) {
    this->playStatus=playStatus;
    this->callJava = callJava;
    this->url = url;
}

void *decodeFF(void *data) {
    FFmpegDecode *fFmpegDecode = (FFmpegDecode *) data;
    fFmpegDecode->decodeFFmpegThread();
    pthread_exit(&fFmpegDecode->decodeThread);
}

void FFmpegDecode::prepared() {
    pthread_create(&decodeThread, NULL, decodeFF, this);
}

void FFmpegDecode::decodeFFmpegThread() {
    av_register_all();
    avformat_network_init();
    pFortmatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFortmatCtx, url, NULL, NULL) != 0) {
        LOGE("can not open url:%s", url)
        return;
    }

    if (avformat_find_stream_info(pFortmatCtx, NULL) < 0) {
        LOGE("can not find stream from  url:%s", url)
        return;
    }

    for (int i = 0; i < pFortmatCtx->nb_streams; i++) {
        if (pFortmatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new SimpleAudio(playStatus,pFortmatCtx->streams[i]->codecpar->sample_rate);
                audio->streamIndex = i;
                audio->codecParameters = pFortmatCtx->streams[i]->codecpar;
            }
        }
    }
    AVCodec *codec = avcodec_find_decoder(audio->codecParameters->codec_id);
    if (!codec) {
        LOGE("can not find decoder")
        return;
    }

    audio->avCodecContext = avcodec_alloc_context3(codec);
    if (!audio->avCodecContext) {
        LOGE("can not alloc new decoderCtx")
        return;
    }

    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecParameters) < 0) {
        LOGE("can not alloc new decoderCtx")

        return;
    }
    if (avcodec_open2(audio->avCodecContext, codec, 0) != 0) {
        LOGE("can not open audio streams")
        return;
    }

    callJava->onCallPrepared(CHILD_THREAD);

}

void FFmpegDecode::start() {
    if (audio == NULL) {
        LOGE("audio is NULL")
        return;
    }
    audio->play();
    int count = 0;
    while (playStatus != NULL && !playStatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFortmatCtx, avPacket) == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
                //解码操作
                count++;
                LOGE("解码第%d帧", count);
                audio->queue->putAvPacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            LOGE("decode finished");
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
    LOGE("解码完成")

}
