//
// Created by docwei on 2020/9/9.
//

#include "CallJava.h"

CallJava::CallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVm = javaVM;
    this->env = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jlz = env->GetObjectClass(jobj);
    if (!jlz) {
        LOGE("get jclass wrong");
        return;
    }
    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_onLoad = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
    jmid_timeInfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
    jmid_error = env->GetMethodID(jlz, "onCallError", "(ILjava/lang/String;)V");
    jmid_complete = env->GetMethodID(jlz, "onCallComplete", "()V");

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

void CallJava::onCallLoad(int type, bool load) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(jobj, jmid_onLoad, load);


    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_onLoad, load);
        javaVm->DetachCurrentThread();

    }
}

void CallJava::onCallTimeInfo(int type, int curr, int total) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(jobj, jmid_timeInfo, curr, total);


    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeInfo, curr, total);
        javaVm->DetachCurrentThread();

    }
}

void CallJava::onCallError(int type, int code, char *msg) {
    if (type == MAIN_THREAD) {
        jstring message = env->NewStringUTF(msg);
        env->CallVoidMethod(jobj, jmid_error, code, message);
        env->DeleteLocalRef(message);

    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jstring message = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, message);
        jniEnv->DeleteLocalRef(message);
        javaVm->DetachCurrentThread();

    }

}

void CallJava::onCallComplete(int type) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(jobj, jmid_complete);


    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_complete);
        javaVm->DetachCurrentThread();

    }

}
