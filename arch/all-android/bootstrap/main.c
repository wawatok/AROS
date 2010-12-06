#include <stdio.h>
#include <unistd.h>

#include "android.h"
#include "bootstrap.h"
#include "ui.h"

JNIEnv *jni;
jclass cl;
jobject obj;
jmethodID DisplayAlert_mid;
jmethodID DisplayError_mid;

int Java_org_aros_bootstrap_AROSBootstrap_Start(JNIEnv* env, jobject this, jstring basedir)
{
    jboolean is_copy;
    const char *arospath = (*env)->GetStringUTFChars(env, basedir, &is_copy);
    int res = chdir(arospath);

    (*env)->ReleaseStringUTFChars(env, basedir, arospath);

    jni = env;
    obj = this;
    cl = (*env)->GetObjectClass(env, this);
    DisplayAlert_mid = (*jni)->GetMethodID(jni, cl, "DisplayAlert", "(Ljava/lang/String;)V");
    DisplayError_mid = (*jni)->GetMethodID(jni, cl, "DisplayError", "(Ljava/lang/String;)V");

    /* We can't pass any arguments (yet) */
    if (res)
	DisplayError("Failed to locate AROS root directory");
    else
	res = bootstrap(0, NULL);

    return res;
}
