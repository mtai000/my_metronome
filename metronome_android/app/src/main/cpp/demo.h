//
// Created by macmini on 2024/12/11.
//

#ifndef METRONOME_ANDROID_DEMO_H
#define METRONOME_ANDROID_DEMO_H
#include "oboe/Oboe.h"
#include "IRestartable.h"
#include "DefaultDataCallback.h"
#include "TappableAudioSource.h"
#include "DefaultErrorCallback.h"
#include <Oscillator.h>
#include <trace.h>

constexpr int32_t kBufferSizeAutomatic = 0;
class SoundGen : public TappableAudioSource{
    static constexpr size_t kSharedBufferSize = 1024;
public:
    SoundGen(int32_t sampleRate,int32_t channelCount);
    ~SoundGen() override = default;

    void tap(bool isOn) override;

    void renderAudio(float *audioData,int32_t numFrames) override;
    void setBpm(int b);
private:
    std::unique_ptr<Oscillator[]> mOscillators;
    std::unique_ptr<float[]> mBuffer = std::make_unique<float[]>(kSharedBufferSize);
    int bpm  = 60;
    int samplesPerBeat;
    int isBeatOn = true;
    int samplesElapsed = 0;
};

class LatTuneCallback: public DefaultDataCallback {
public:
    LatTuneCallback():DefaultDataCallback(){
        Trace::initialize();
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* oboeStream, void *audioData, int32_t numFrames) override;
    void setBufferTuneEnabled(bool enabled) { mBufferTuneEnabled = enabled; }
private:
    bool mBufferTuneEnabled = true;

    std::unique_ptr<oboe::LatencyTuner> mLatencyTuner;
    oboe::AudioStream *mStream = nullptr;
};

class DemoEngine:public IRestartable{
public:
    DemoEngine();
    virtual  ~DemoEngine() = default;

    oboe::Result start();
    oboe::Result start(oboe::AudioApi audioApi, int deviceId, int channelCount);

    oboe::Result stop();
    void restart() override;
    void setBufferSizeInBursts(int32_t numBursts);
    void tap(bool isDown);

    double getCurrentOutputLatencyMillis();
    bool isLatencyDetectionSupported();
    bool isAAudioRecommended();

    void setBPM(int bpm);

private:
    oboe::Result  reopenStream();
    oboe::Result  openPlaybackStream();

    std::shared_ptr<oboe::AudioStream> mStream;
    std::shared_ptr<LatTuneCallback> mLatCallback;
    std::shared_ptr<DefaultErrorCallback> mErrorCallback;
    std::shared_ptr<SoundGen> mSoundGen;
    bool mIsLatSupported = false;

    int32_t         mDeviceId       = oboe::Unspecified;
    int32_t         mChannelCount   = oboe::Unspecified;
    oboe::AudioApi  mAudioApi       = oboe::AudioApi::Unspecified;

    std::mutex mLock;
};

#endif //METRONOME_ANDROID_DEMO_H
