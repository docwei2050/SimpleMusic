//
// Created by docwei on 2020/9/10.
//

#ifndef SIMPLEMUSIC_ANDROIDLOG_H
#define SIMPLEMUSIC_ANDROIDLOG_H

#include "android/log.h"


#define LOG_DEBUG true

#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,"simpleMusic",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"simpleMusic",FORMAT,##__VA_ARGS__);


#endif //SIMPLEMUSIC_ANDROIDLOG_H



