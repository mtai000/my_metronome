//
// Created by macmini on 2024/12/11.
//
#include "Demo.h"
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include "logging_macros.h"
#include <oboe/Oboe.h>

extern "C" {
    static DemoEngine sDemoEngine;
    JNIEXPORT jstring JNICALL
    Java_com_mtai_metronome_AudioEngine_stringFromJNI(JNIEnv *env, jclass) {
        return env->NewStringUTF("Hello world from C++");
    }

    JNIEXPORT jint JNICALL
    Java_com_mtai_metronome_AudioEngine_demoStartEngine(JNIEnv *env, jclass clazz,
                                                        jint audio_api,jint device_id, jint channel_count) {
        return static_cast<jint>(sDemoEngine.start((oboe::AudioApi)oboe::AudioApi::AAudio,device_id,channel_count));
    }
    JNIEXPORT jint JNICALL
    Java_com_mtai_metronome_AudioEngine_demoStopEngine(JNIEnv *env, jclass clazz) {
        return static_cast<jint>(sDemoEngine.stop());
    }
    JNIEXPORT void JNICALL
    Java_com_mtai_metronome_AudioEngine_demoSetToneOn(JNIEnv *env, jclass clazz, jboolean is_tone_on) {
        sDemoEngine.tap(is_tone_on);
    }

    JNIEXPORT void JNICALL
    Java_com_mtai_metronome_AudioEngine_demoSetBPM(JNIEnv *env,jclass clazz, jint bpm){
        sDemoEngine.setBPM(bpm);
    }

    //JNIEXPORT void JNICALL
    //Java_com_mtai_metronome_AudioEngine_demoSetBufferSizeInBursts(JNIEnv *env, jclass clazz,jint buffer_size_in_bursts) {
    //    sDemoEngine.setBufferSizeInBursts(buffer_size_in_bursts);
    //}
    //JNIEXPORT jdouble JNICALL
    //Java_com_mtai_metronome_AudioEngine_demoGetCurrentOutputLatencyMills(JNIEnv *env, jclass clazz) {
    //    return static_cast<jdouble>(sDemoEngine.getCurrentOutputLatencyMillis());
    //}
    //JNIEXPORT jboolean JNICALL
    //Java_com_mtai_metronome_AudioEngine_demoIsLatencyDetectionSupported(JNIEnv *env, jclass clazz) {
    //    return (sDemoEngine.isLatencyDetectionSupported() ? JNI_TRUE : JNI_FALSE);
    //}
    //JNIEXPORT jboolean JNICALL
    //Java_com_mtai_metronome_AudioEngine_demoIsAAudioRecommended(JNIEnv *env, jclass clazz) {
    //    return (sDemoEngine.isAAudioRecommended() ? JNI_TRUE : JNI_FALSE);
    //}
    //JNIEXPORT void JNICALL
    //Java_com_mtai_metronome_AudioEngine_demoSetDefaultStreamValues(JNIEnv *env, jclass clazz,jint sample_rate,jint frames_per_burst) {
    //    oboe::DefaultStreamValues::SampleRate = (int32_t) sample_rate;
    //    oboe::DefaultStreamValues::FramesPerBurst = (int32_t) frames_per_burst;
    //}
}

