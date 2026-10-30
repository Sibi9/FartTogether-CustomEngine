#include <jni.h>
#include <android/log.h>
#include "../core/EngineCore.hpp"

#define LOG_TAG "CustomEngineNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static std::unique_ptr<CustomEngine::EngineCore> g_engine = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_customengine_NativeBridge_initEngine(JNIEnv *env, jobject thiz) {
    if (!g_engine) {
        g_engine = std::make_unique<CustomEngine::EngineCore>();
        g_engine->Initialize();
        LOGI("CustomEngine Native Core initialized successfully on Android.");
    }
}

JNIEXPORT void JNICALL
Java_com_customengine_NativeBridge_setFileSpeedLimit(JNIEnv *env, jobject thiz, jint fileId, jlong bytesPerSec) {
    if (g_engine) {
        g_engine->SetFileSpeedLimit(static_cast<uint32_t>(fileId), static_cast<int64_t>(bytesPerSec));
        LOGI("Speed limit updated for file %d to %lld B/s", fileId, bytesPerSec);
    }
}

JNIEXPORT jfloat JNICALL
Java_com_customengine_NativeBridge_getFileProgress(JNIEnv *env, jobject thiz, jint fileId) {
    if (g_engine) {
        return g_engine->GetFileProgress(static_cast<uint32_t>(fileId));
    }
    return 0.0f;
}

JNIEXPORT jboolean JNICALL
Java_com_customengine_NativeBridge_isFileComplete(JNIEnv *env, jobject thiz, jint fileId) {
    if (g_engine) {
        return g_engine->IsFileComplete(static_cast<uint32_t>(fileId)) ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_customengine_NativeBridge_sendTouchInput(JNIEnv *env, jobject thiz, jfloat moveX, jfloat moveY, jboolean gas, jboolean puff, jint tick) {
    if (g_engine) {
        g_engine->SendPlayerInput(moveX, moveY, gas == JNI_TRUE, puff == JNI_TRUE, static_cast<uint32_t>(tick));
    }
}

JNIEXPORT void JNICALL
Java_com_customengine_NativeBridge_updateTick(JNIEnv *env, jobject thiz) {
    if (g_engine) {
        g_engine->UpdateTick();
    }
}

}
