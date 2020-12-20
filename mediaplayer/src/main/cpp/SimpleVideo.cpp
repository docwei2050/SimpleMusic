//
// Created by docwei on 2020/11/25.
//

#include "SimpleVideo.h"

SimpleVideo::SimpleVideo(PlayStatus *playStatus, CallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new SimpleQueue(playStatus);
}

SimpleVideo::~SimpleVideo() {

}

void *playVideo(void *data) {
    SimpleVideo *video = static_cast<SimpleVideo *>(data);
    while (video->playStatus != NULL && !video->playStatus->exit) {
        if (video->playStatus->seek) {
            av_usleep(100 * 1000);
            continue;
        }
        if (video->queue->getQueueSize() == 0) {
            if (!video->playStatus->load) {
                video->playStatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);
                av_usleep(100 * 1000);
                continue;
            }
        }else{
            if (video->playStatus->load) {
                video->playStatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvPakcet(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->avCodecContext, avFrame) != 0) {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

         //渲染视频 从avFrame里面去获取yuv420p的数据
         if(avFrame->format==AV_PIX_FMT_YUV420P){
             LOGE("当前视频是YUV420P")
             //渲染调用的代码 宽度使用video->avCodecContext->width会出现花屏
             video->callJava->onCallRenderYUV( avFrame->linesize[0],video->avCodecContext->height,
                     avFrame->data[0],avFrame->data[1],avFrame->data[2]);
         }else{
             LOGE("当前视频不是YUV420P需要转换成YUV420P")
             AVFrame *pFrameYUV420P=av_frame_alloc();
             int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,video->avCodecContext->width,video->avCodecContext->height,1);
             uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
             av_image_fill_arrays(pFrameYUV420P->data,
                     pFrameYUV420P->linesize,
                     buffer,
                     AV_PIX_FMT_YUV420P,
                     video->avCodecContext->width,
                     video->avCodecContext->height,1);
             SwsContext *swsContext=sws_getContext(avFrame->linesize[0],video->avCodecContext->height,video->avCodecContext->pix_fmt,video->avCodecContext->width,video->avCodecContext->height,
                     AV_PIX_FMT_YUV420P,SWS_BICUBIC,NULL,NULL,NULL);
             if(!swsContext){
                 av_frame_free(&pFrameYUV420P);
                 av_free(pFrameYUV420P);
                 av_free(buffer);
                 continue;
             }
             sws_scale(swsContext,reinterpret_cast<const uint8_t *const* >(avFrame->data), avFrame->linesize,0,avFrame->height,pFrameYUV420P->data,pFrameYUV420P->linesize);
             //渲染调用的代码 宽度使用video->avCodecContext->width会出现花屏
             video->callJava->onCallRenderYUV(avFrame->linesize[0],video->avCodecContext->height,
                                              pFrameYUV420P->data[0],pFrameYUV420P->data[1],pFrameYUV420P->data[2]);

             av_frame_free(&pFrameYUV420P);
             av_free(pFrameYUV420P);
             av_free(buffer);
             sws_freeContext(swsContext);

         }




        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }

    pthread_exit(&video->thread_play);
}

void SimpleVideo::play() {
    pthread_create(&thread_play, NULL, playVideo, this);
}

void SimpleVideo::release() {
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
}
