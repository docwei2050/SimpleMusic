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
public :
    CallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj);

    ~CallJava();

    void onCallPrepared(int type);
};



#endif //SIMPLEMUSIC_CALLJAVA_H
