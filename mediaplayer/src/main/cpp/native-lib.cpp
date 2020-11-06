#include <jni.h>
#include <string>
#include <stddef.h>
#include "FFmpegDecode.h"

extern "C" {
#include <libavformat/avformat.h>
}
JavaVM *javaVm = NULL;
CallJava *callJava = NULL;
FFmpegDecode *fFmpegDecode = NULL;
PlayStatus *playStatus = NULL;
bool nexit = true;
pthread_t start_thread;
extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1parpared(JNIEnv *env, jobject thiz, jstring source) {
    const char *url = env->GetStringUTFChars(source, 0);
    if (fFmpegDecode == NULL) {
        if (callJava == NULL) {
            callJava = new CallJava(javaVm, env, &thiz);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playStatus = new PlayStatus();
        fFmpegDecode = new FFmpegDecode(playStatus, callJava, url);
        fFmpegDecode->prepared();
    }
    // env->ReleaseStringUTFChars(source, url);
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVm = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }
    return JNI_VERSION_1_6;
}

void *start(void *ctx) {
    FFmpegDecode *fFmpegDecode = (FFmpegDecode *) ctx;
    fFmpegDecode->start();
    pthread_exit(&start_thread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1start(JNIEnv *env, jobject thiz) {
    if (fFmpegDecode != NULL) {
        pthread_create(&start_thread, NULL, start, fFmpegDecode);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1resume(JNIEnv *env, jobject thiz) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->resume();
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1pause(JNIEnv *env, jobject thiz) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->pause();
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1stop(JNIEnv *env, jobject thiz) {
    if (!nexit) {
        return;
    }
    jclass jlz = env->GetObjectClass(thiz);
    jmethodID jmid_playNext = env->GetMethodID(jlz, "onCallNext", "()V");


    nexit = false;
    if (fFmpegDecode != NULL) {
        fFmpegDecode->release();
        delete (fFmpegDecode);
        fFmpegDecode = NULL;
        if (callJava != NULL) {
            delete (callJava);
            callJava = NULL;
        }
        if (playStatus != NULL) {
            delete (playStatus);
            playStatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(thiz, jmid_playNext);
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1seek(JNIEnv *env, jobject thiz, jint seconds) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->seek(seconds);
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1duration(JNIEnv *env, jobject thiz) {
    if (fFmpegDecode != NULL) {
        return fFmpegDecode->duration;
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1volumn(JNIEnv *env, jobject thiz, jint percent) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->setVolumn(percent);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1mute(JNIEnv *env, jobject thiz, jint mute) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->setMute(mute);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1setTune(JNIEnv *env, jobject thiz, jfloat tune) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->setTune(tune);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1setSpeed(JNIEnv *env, jobject thiz, jfloat speed) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->setSpeed(speed);
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1samplerate(JNIEnv *env, jobject thiz) {
    if (fFmpegDecode != NULL) {
       return  fFmpegDecode->getSampleRate();
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1startstoprecord(JNIEnv *env, jobject thiz,
                                                           jboolean start) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->startStopRecord(start);
    }
}