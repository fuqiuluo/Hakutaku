//
// Created by 13723 on 24-4-13.
//


#include <cstdint>
#include <cstring>
#include <string>
#include "jni.h"
#include "android/log.h"
#include <cstdint>

#define TAG "UE4-Hook"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

using namespace std;

using pointer = uint64_t;

const pointer GName = 0xB171CC0;
const pointer GUObjectArray = 0xB1B5F98;
const pointer GWorld = 0xB32D8A8;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGI("jni onload called");
    JNIEnv* env = nullptr;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) { //从JavaVM获取JNIEnv，一般使用1.4的版本
        return -1;
    }



    LOGI("jni onload called end...");
    return JNI_VERSION_1_4; //这里很重要，必须返回版本，否则加载会失败。
}