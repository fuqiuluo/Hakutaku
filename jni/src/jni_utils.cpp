#include "jni_utils.h"

auto hak::get_string(JNIEnv *env, jstring jstr) -> std::string {
    const char* cstr = env->GetStringUTFChars(jstr, nullptr);
    jsize len = env->GetStringUTFLength(jstr);
    std::string str(cstr, len);
    env->ReleaseStringUTFChars(jstr, cstr);
    return std::move(str);
}
