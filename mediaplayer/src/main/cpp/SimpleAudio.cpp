//
// Created by docwei on 2020/9/10.
//


#include "SimpleAudio.h"


SimpleAudio::SimpleAudio(PlayStatus *playStatus, int sample_rate, CallJava *callJava) {
    this->calljava = callJava;
    this->sample_rate = sample_rate;
    this->playStatus = playStatus;
    queue = new SimpleQueue(playStatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);
}

void *decodePlay(void *data) {
    SimpleAudio *audio = (SimpleAudio *) data;
    audio->initOpenSLES();
    pthread_exit(&audio->thread_play);
}

void SimpleAudio::play() {
    pthread_create(&thread_play, NULL, decodePlay, this);
}


int SimpleAudio::resampleAudio() {
    //不置于0就会一直回调
    data_size = 0;
    while (playStatus != NULL && !playStatus->exit) {
        if (queue->getQueueSize() == 0) {
            //加载中
            if (!playStatus->load) {
                playStatus->load = true;
                calljava->onCallLoad(CHILD_THREAD, true);
            }
            continue;
        } else {
            if (playStatus->load) {
                playStatus->load = false;
                calljava->onCallLoad(CHILD_THREAD, false);
            }
        }


        avPacket = av_packet_alloc();
        if (queue->getAvPakcet(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0) {
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swrContext;
            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL, NULL);
            if (!swrContext || swr_init(swrContext) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swrContext);
                continue;
            }
            //重采样操作
            int nb = swr_convert(swrContext, &buffer, avFrame->nb_samples,
                                 (const uint8_t **) avFrame->data, avFrame->nb_samples);
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            //LOGE("data_size is %d", data_size);

            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            break;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }

    }

    return data_size;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    SimpleAudio *audio = (SimpleAudio *) context;
    if (audio != NULL) {
        //  重采样将目标音频搞成Pcm数据给OPenSLES队列
        int bufferSize = audio->resampleAudio();
        if (bufferSize > 0) {
            audio->clock += bufferSize / (double) (audio->sample_rate * 2 * 2);
            //防止频繁回调  在线直播时audio->duration是拿不到的
            if (audio->clock - audio->last_time > 0.1 && audio->duration > 0) {
                audio->last_time = audio->clock;
                audio->calljava->onCallTimeInfo(CHILD_THREAD, audio->clock, audio->duration);
            }
            (*audio->pcmBufferQueue)->Enqueue(audio->pcmBufferQueue, (char *) audio->buffer,
                                              bufferSize);
        }
    }
}


unsigned int SimpleAudio::getCurrentSampleRateForOpenSles(int sample_rate) {
    unsigned int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
            break;

    }
    return rate;
}


void SimpleAudio::initOpenSLES() {
    SLresult result;
    //第一步------------------------------------------
    // 创建引擎对象
    slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);


    //第二步-------------------------------------------
    // 创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    // 第三步--------------------------------------------
    // 创建播放器
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRateForOpenSles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource slDataSource = {&android_queue, &pcm};
    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource,
                                                &audioSnk, 3, ids, req);
    // 初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmPlayerVolume);

    //第四步---------------------------------------
    // 创建缓冲区和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);



    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    //获取音量接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmPlayerVolume);

    setVolumn(volumnPercent);
    //第五步----------------------------------------
    // 设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);


    //第六步----------------------------------------
    // 主动调用回调函数开始工作
    pcmBufferCallBack(pcmBufferQueue, this);

}

void SimpleAudio::pause() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void SimpleAudio::resume() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void SimpleAudio::stop() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void SimpleAudio::release() {
    stop();
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (engineEngine != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (calljava != NULL) {
        calljava = NULL;
    }
    last_time = 0;
    clock = 0;
}

SimpleAudio::~SimpleAudio() {

}

void SimpleAudio::setVolumn(int percent) {
    volumnPercent=percent;
    if (pcmPlayerPlay != NULL) {
        {
            if (percent > 30) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -20);
            } else if (percent > 25) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -22);
            } else if (percent > 20) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -25);
            } else if (percent > 15) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -28);
            } else if (percent > 10) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -30);
            } else if (percent > 5) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -34);
            } else if (percent > 3) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -37);
            } else if (percent > 0) {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -40);
            } else {
                (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -100);
            }
        }
    }
}


