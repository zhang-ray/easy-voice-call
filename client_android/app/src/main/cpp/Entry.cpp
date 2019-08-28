#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_getVersion(JNIEnv* env, jobject) {
    std::string version = "v0.0.1";
    return env->NewStringUTF(version.c_str());
}
