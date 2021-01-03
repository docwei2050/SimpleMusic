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
    jmid_renderYUV = env->GetMethodID(jlz, "onCallRenderYUV", "(II[B[B[B)V");
    jmid_isSupportCodecType = env->GetMethodID(jlz, "onCallIsSupportCodecType",
                                               "(Ljava/lang/String;)Z");
    jmid_initmediacodec = env->GetMethodID(jlz, "initMediaCodec", "(Ljava/lang/String;II[B[B)V");
    jmid_decodeavpacket = env->GetMethodID(jlz, "decodeAVPacket", "(I[B)V");

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

void CallJava::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv) {
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        return;
    }
    //yvu420P===其中y:uv=4:1
    jbyteArray y = jniEnv->NewByteArray(width * height);
    jniEnv->SetByteArrayRegion(y, 0, width * height, reinterpret_cast<const jbyte *>(fy));

    jbyteArray u = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(u, 0, width * height / 4, reinterpret_cast<const jbyte *>(fu));

    jbyteArray v = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(v, 0, width * height / 4, reinterpret_cast<const jbyte *>(fv));

    jniEnv->CallVoidMethod(jobj, jmid_renderYUV, width, height, y, u, v);
    jniEnv->DeleteLocalRef(y);
    jniEnv->DeleteLocalRef(u);
    jniEnv->DeleteLocalRef(v);
    javaVm->DetachCurrentThread();

}

bool CallJava::onCallIsSupportCodecType(const char *ffType) {
    bool isSupport = false;
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        return isSupport;
    }
    jstring type = jniEnv->NewStringUTF(ffType);
    isSupport = jniEnv->CallBooleanMethod(jobj, jmid_isSupportCodecType, type);
    jniEnv->DeleteLocalRef(type);
    javaVm->DetachCurrentThread();
    return isSupport;
}

void CallJava::onCallInitMediacodec(const char *mime, int width, int height, int csd0_size,
                                    int csd1_size, uint8_t *csd_0, uint8_t *csd_1) {
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        return;
    }
    jstring type = jniEnv->NewStringUTF(mime);
    jbyteArray csd0 = jniEnv->NewByteArray(csd0_size);
    jniEnv->SetByteArrayRegion(csd0, 0, csd0_size, reinterpret_cast<const jbyte *>(csd_0));

    jbyteArray csd1 = jniEnv->NewByteArray(csd1_size);
    jniEnv->SetByteArrayRegion(csd1, 0, csd1_size, reinterpret_cast<const jbyte *>(csd_1));

    jniEnv->CallVoidMethod(jobj, jmid_initmediacodec, type, width, height, csd0, csd1);
    jniEnv->DeleteLocalRef(csd0);
    jniEnv->DeleteLocalRef(csd1);
    jniEnv->DeleteLocalRef(type);
    javaVm->DetachCurrentThread();
}

void CallJava::onCallDecodeAVPacket(int datasize, uint8_t *packetdata) {
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        return;
    }
    jbyteArray data = jniEnv->NewByteArray(datasize);
    jniEnv->SetByteArrayRegion(data, 0, datasize, reinterpret_cast<const jbyte *>(packetdata));
    jniEnv->CallVoidMethod(jobj, jmid_decodeavpacket, datasize, data);
    jniEnv->DeleteLocalRef(data);
    javaVm->DetachCurrentThread();

}
