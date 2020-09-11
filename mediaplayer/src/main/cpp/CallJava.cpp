//
// Created by docwei on 2020/9/9.
//

#include "CallJava.h"

CallJava::CallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVm = javaVM;
    this->env = env;
    this->jobj= *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jlz = env->GetObjectClass(jobj);
    if (!jlz) {
        LOGE("get jclass wrong");
        return;
    }
    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");

}

CallJava::~CallJava() {

}

void CallJava::onCallPrepared(int type) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(jobj, jmid_prepared);


    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVm->DetachCurrentThread();

    }

}