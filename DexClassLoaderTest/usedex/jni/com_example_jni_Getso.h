/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_example_jni_Getso */

#ifndef _Included_com_example_jni_Getso
#define _Included_com_example_jni_Getso
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_example_jni_Getso
 * Method:    getVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_jni_Getso_getVersion
  (JNIEnv *, jobject);

/*
 * Class:     com_example_jni_Getso
 * Method:    getBuf
 * Signature: ([B[B[B)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_jni_Getso_getBuf
  (JNIEnv *, jobject, jbyteArray, jbyteArray, jbyteArray);

#ifdef __cplusplus
}
#endif
#endif
