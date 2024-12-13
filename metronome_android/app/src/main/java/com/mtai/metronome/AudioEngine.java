package com.mtai.metronome;

import android.util.Log;

public class AudioEngine {

    static long handle = 0;
    static boolean paused = true;

    static {
        try {
            System.loadLibrary("engine");
            Log.i("JNI", "Library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e("JNI", "Failed to load library", e);
        }
    }
    public static native String stringFromJNI();
    public static native int demoStartEngine(int audioApi,int deviceId,int channelCount);
    public static native int demoStopEngine();
    public static native void demoSetBPM(int bpm);
    public static native void demoSetToneOn(boolean isToneOn);
    //public static native void demoSetBufferSizeInBursts(int bufferSizeInBursts);
    //public static native double demoGetCurrentOutputLatencyMills();
    //public static native boolean demoIsLatencyDetectionSupported();
    //public static native boolean demoIsAAudioRecommended();
    //public static native void demoSetDefaultStreamValues(int sampleRate,int framesPerBurst);
}
