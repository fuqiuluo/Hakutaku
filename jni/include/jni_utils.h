#ifndef HAK_JNI_UTILS_H
#define HAK_JNI_UTILS_H

#include <jni.h>
#include <string>

namespace hak {

    auto get_string(JNIEnv *env, jstring jstr) -> std::string;

}

#endif //HAK_JNI_UTILS_H
