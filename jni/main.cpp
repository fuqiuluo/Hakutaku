#include "process.h"
#include "proc.h"

#include <string>
#include "jni.h"
#include "jni_utils.h"

extern "C"
JNIEXPORT auto JNICALL
Java_moe_fuqiuluo_hak_external_Platform_getPid(JNIEnv *env, jobject thiz, jstring package_name) -> jint {
    auto package = hak::get_string(env, package_name);
    auto pid = hak::find_process(package);
    return pid;
}

extern "C"
JNIEXPORT auto JNICALL
Java_moe_fuqiuluo_hak_external_Platform_getPidList(JNIEnv *env, jobject thiz) -> jobject {
    auto pidList = hak::get_pid_list();
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID arrayListAdd = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    jobject arrayList = env->NewObject(arrayListClass, arrayListConstructor);
    jclass jint = env->FindClass("java/lang/Integer");
    jmethodID jint_cons = env->GetMethodID(jint, "<init>", "(I)V");
    for (pid_t pid : pidList) {
        jobject integerObject = env->NewObject(jint, jint_cons, pid);
        env->CallBooleanMethod(arrayList, arrayListAdd, integerObject);
    }
    return arrayList;
}

