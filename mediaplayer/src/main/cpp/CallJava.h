//
// Created by docwei on 2020/9/9.
//

#ifndef SIMPLEMUSIC_CALLJAVA_H
#define SIMPLEMUSIC_CALLJAVA_H

#include <jni.h>
#include "AndroidLog.h"
#include <stddef.h>
#define MAIN_THREAD 0
#define CHILD_THREAD 1


class CallJava {

public:
    JavaVM *javaVm = NULL;
    JNIEnv *env = NULL;
    jobject jobj;

    jmethodID  jmid_prepared;
    jmethodID jmid_onLoad;
    jmethodID jmid_timeInfo;
    jmethodID jmid_error;
    jmethodID jmid_complete;

public :
    CallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj);

    ~CallJava();

    void onCallPrepared(int type);
    void onCallLoad(int type,bool load);
    void onCallTimeInfo(int type,int curr,int total);
    void onCallError(int type,int code,char * msg);
    void onCallComplete(int type);

};



#endif //SIMPLEMUSIC_CALLJAVA_H
