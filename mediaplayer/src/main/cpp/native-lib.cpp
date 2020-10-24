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
PlayStatus *playStatus=NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1parpared(JNIEnv *env, jobject thiz, jstring source) {
    const char *url = env->GetStringUTFChars(source, 0);
    if (fFmpegDecode == NULL) {
        if (callJava == NULL) {
            callJava = new CallJava(javaVm, env, &thiz);
        }
        playStatus=new PlayStatus();
        fFmpegDecode = new FFmpegDecode(playStatus,callJava, url);
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
}extern "C"
JNIEXPORT void JNICALL
Java_com_docwei_mediaplayer_MusicPlayer_n_1start(JNIEnv *env, jobject thiz) {
    if (fFmpegDecode != NULL) {
        fFmpegDecode->start();
    }
}