//
// Created by docwei on 2020/11/25.
//

#include "SimpleVideo.h"

SimpleVideo::SimpleVideo(PlayStatus *playStatus, CallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new SimpleQueue(playStatus);
    pthread_mutex_init(&codecMutex, NULL);
}

SimpleVideo::~SimpleVideo() {
    pthread_mutex_destroy(&codecMutex);
}

void *playVideo(void *data) {
    SimpleVideo *video = static_cast<SimpleVideo *>(data);
    while (video->playStatus != NULL && !video->playStatus->exit) {
        if (video->playStatus->seek) {
            av_usleep(100 * 1000);
            continue;
        }
        //添加暂停功能
        if (video->playStatus->pause) {
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
        } else {
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
        if (video->codecType == CODEC_MEDIACODEC) {
            LOGE("开始硬解码")
            if (av_bsf_send_packet(video->abs_ctx, avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            while (av_bsf_receive_packet(video->abs_ctx, avPacket)==0) {
                double diff = video->getFrameDiffTime(NULL, avPacket);
                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallDecodeAVPacket(avPacket->size, avPacket->data);
                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = NULL;

        } else if (video->codecType == CODEC_YUV) {
            //此处用到avCodec解码就加一个线程锁
            pthread_mutex_lock(&video->codecMutex);
            if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
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
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            //渲染视频 从avFrame里面去获取yuv420p的数据
            if (avFrame->format == AV_PIX_FMT_YUV420P) {
                LOGE("当前视频是YUV420P")

                double diff = video->getFrameDiffTime(avFrame, NULL);
                LOGE("需要休眠的时间--》%f", video->getDelayTime(diff))
                av_usleep(video->getDelayTime(diff) * 1000000);
                //渲染调用的代码 宽度使用video->avCodecContext->width会出现花屏
                if (video->callJava != NULL) {
                    video->callJava->onCallRenderYUV(avFrame->linesize[0],
                                                     video->avCodecContext->height,
                                                     avFrame->data[0], avFrame->data[1],
                                                     avFrame->data[2]);
                }
            } else {
                LOGE("当前视频不是YUV420P需要转换成YUV420P")
                AVFrame *pFrameYUV420P = av_frame_alloc();
                int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video->avCodecContext->width,
                                                   video->avCodecContext->height, 1);
                uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
                av_image_fill_arrays(pFrameYUV420P->data,
                                     pFrameYUV420P->linesize,
                                     buffer,
                                     AV_PIX_FMT_YUV420P,
                                     video->avCodecContext->width,
                                     video->avCodecContext->height, 1);
                SwsContext *swsContext = sws_getContext(avFrame->linesize[0],
                                                        video->avCodecContext->height,
                                                        video->avCodecContext->pix_fmt,
                                                        video->avCodecContext->width,
                                                        video->avCodecContext->height,
                                                        AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL,
                                                        NULL);
                if (!swsContext) {
                    av_frame_free(&pFrameYUV420P);
                    av_free(pFrameYUV420P);
                    av_free(buffer);
                    pthread_mutex_unlock(&video->codecMutex);
                    continue;
                }
                sws_scale(swsContext, reinterpret_cast<const uint8_t *const * >(avFrame->data),
                          avFrame->linesize, 0, avFrame->height, pFrameYUV420P->data,
                          pFrameYUV420P->linesize);


                double diff = video->getFrameDiffTime(pFrameYUV420P, NULL);
                LOGE("需要休眠的时间--》%f", video->getDelayTime(diff))
                av_usleep(video->getDelayTime(diff) * 1000000);
                //渲染调用的代码 宽度使用video->avCodecContext->width会出现花屏
                video->callJava->onCallRenderYUV(pFrameYUV420P->linesize[0],
                                                 video->avCodecContext->height,
                                                 pFrameYUV420P->data[0], pFrameYUV420P->data[1],
                                                 pFrameYUV420P->data[2]);

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
            pthread_mutex_unlock(&video->codecMutex);
        }
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
    if (abs_ctx != NULL) {
        av_bsf_free(&abs_ctx);
        abs_ctx = NULL;
    }
    if (avCodecContext != NULL) {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        pthread_mutex_unlock(&codecMutex);
    }
}

double SimpleVideo::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {
    double pts = 0;
    if (avFrame != NULL) {
        pts = av_frame_get_best_effort_timestamp(avFrame);
    }
    if (avPacket != NULL) {
        pts = avPacket->pts;
    }
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(time_base);
    if (pts > 0) {
        clock = pts;
    }
    LOGE("audio->click %f", audio->clock)
    LOGE("clock %f", clock)

    double diff = audio->clock - clock;
    return diff;
}

double SimpleVideo::getDelayTime(double diff) {
    LOGE("clockdiffdiff  %f", diff)
    //将误差控制在3ms以内 延迟正常不超过2帧间距时长
    //可能音频不存在，不存在，延迟1帧的时间
    if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff == 0.003) {

    }
    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }
    if (fabs(diff) > 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}
