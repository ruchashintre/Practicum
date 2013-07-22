#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include "JNIFoo.h"

JNIEXPORT jint JNICALL Java_JNIFoo_nativeFoo (JNIEnv *env, jobject obj)
{
	int i;
	jint ret = 0;
	for (i=0;i<1000000000;i++){
		ret = ret ^ 1;
  	}
	
  	return ret;
}
