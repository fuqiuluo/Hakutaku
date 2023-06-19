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

extern "C"
JNIEXPORT auto JNICALL
Java_moe_fuqiuluo_hak_external_ProcessUtils_getProcessList(JNIEnv *env, jobject thiz) -> jobject {
    auto proc_list = hak::get_process_list();
    jclass arrayList_clz = env->FindClass("java/util/ArrayList");
    jmethodID arraylist_init = env->GetMethodID(arrayList_clz, "<init>", "()V");
    jmethodID arraylist_add = env->GetMethodID(arrayList_clz, "add", "(Ljava/lang/Object;)Z");
    jobject arraylist = env->NewObject(arrayList_clz, arraylist_init);

    jclass proc_stat = env->FindClass("moe/fuqiuluo/hak/external/ProcessUtils$ProcStat");
    jmethodID proc_stat_init = env->GetMethodID(proc_stat, "<init>", "(ILjava/lang/String;CI)V");

    for (const auto &stat : proc_list) {
        jobject object = env->NewObject(proc_stat,
            proc_stat_init,
            stat.pid, hak::string_to_jstring(env, stat.comm), stat.state, stat.ppid
        );
        env->CallBooleanMethod(arraylist, arraylist_add, object);
    }

    return arraylist;
}