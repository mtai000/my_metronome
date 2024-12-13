//
// Created by macmini on 2024/12/11.
//

#include "Demo.h"
#include <android/log.h>
#include "logging_macros.h"
DemoEngine::DemoEngine():
        mLatCallback(std::make_shared<LatTuneCallback>()),
        mErrorCallback(std::make_shared<DefaultErrorCallback>(*this)) {
}

oboe::Result DemoEngine::start() {
    std::lock_guard<std::mutex> lock(mLock);
    oboe::Result result = oboe::Result::OK;
    int tryCount = 0;
    do{
        if(tryCount > 0){
            usleep(20 * 1000);
        }
        mIsLatSupported = false;
        result = openPlaybackStream();
        if(result == oboe::Result::OK){
            mSoundGen = std::make_shared<SoundGen>(mStream->getSampleRate(),mStream->getChannelCount());
            mLatCallback->setSource(std::dynamic_pointer_cast<IRenderableAudio>(mSoundGen));

            LOGD("Stream opened: AudioAPI = %d, channelCount = %d, deviceID = %d",
                 mStream->getAudioApi(),
                 mStream->getChannelCount(),
                 mStream->getDeviceId());
            result = mStream->requestStart();
            if(result != oboe::Result::OK){
                LOGE("Error starting playback stream. Error: %s",oboe::convertToText(result));
                mStream->close();
                mStream.reset();
            } else {
               mIsLatSupported = (mStream->getTimestamp(CLOCK_MONOTONIC) != oboe::Result::ErrorUnimplemented);
            }

        } else {
            LOGE("Error creating playback stream. Error: %s", oboe::convertToText(result));
        }

    } while ( result != oboe::Result::OK && tryCount++ < 3);
    return result;
}

oboe::Result DemoEngine::start(oboe::AudioApi audioApi, int deviceId, int channelCount) {
    mAudioApi = audioApi;
    mDeviceId = deviceId;
    mChannelCount = channelCount;
    return start();
}

oboe::Result DemoEngine::stop() {
    oboe::Result result = oboe::Result::OK;
    std::lock_guard<std::mutex> lock(mLock);
    if(mStream){
        result = mStream->stop();
        mStream->close();
        mStream.reset();
    }
    return result;
}

void DemoEngine::restart() {
    mLatCallback.reset();
    start();
}
//
//void DemoEngine::setBufferSizeInBursts(int32_t numBursts) {
//    std::lock_guard<std::mutex> lock(mLock);
//    if(!mStream) return;
//
//    mLatCallback->setBufferTuneEnabled(numBursts == kBufferSizeAutomatic);
//    auto result = mStream->setBufferSizeInFrames(numBursts * mStream->getFramesPerBurst());
//    if(result){
//        LOGD("Buffer size successfully changed to : %d",result.value());
//    } else {
//        LOGW("Buffer size could not be changed, %d", result.error());
//    }
//
//}
//
//double DemoEngine::getCurrentOutputLatencyMillis() {
//    if(!mIsLatSupported) return -1.0;
//    std::lock_guard<std::mutex> lock(mLock);
//    if(!mStream) return -1.0;
//
//    oboe::ResultWithValue<double> latResult = mStream->calculateLatencyMillis();
//    if(latResult){
//        return latResult.value();
//    } else {
//        LOGE("Error calculating latency: %s",oboe::convertToText(latResult.error()));
//    }
//    return -1.0;
//}
//
//bool DemoEngine::isLatencyDetectionSupported() {
//    return mIsLatSupported;
//}
//
//bool DemoEngine::isAAudioRecommended() {
//    return oboe::AudioStreamBuilder::isAAudioRecommended();
//}
//
//oboe::Result DemoEngine::reopenStream() {
//    if(mStream){
//        stop();
//        return start();
//    } else {
//        return oboe::Result::OK;
//    }
//}
//
oboe::Result DemoEngine::openPlaybackStream() {
    oboe::AudioStreamBuilder builder;
    oboe::Result result = builder.setSharingMode(oboe::SharingMode::Exclusive)
                                ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
                                ->setFormat(oboe::AudioFormat::Float)
                                ->setFormatConversionAllowed(true)
                                ->setDataCallback(mLatCallback)
                                ->setErrorCallback(mErrorCallback)
                                ->setAudioApi(mAudioApi)
                                ->setChannelCount(mChannelCount)
                                ->setDeviceId(mDeviceId)
                                ->openStream(mStream);

    if(result == oboe::Result::OK){
        mChannelCount = mStream->getChannelCount();
    }
    return result;
}

void DemoEngine::tap(bool isDown) {
    if(mSoundGen){
        mSoundGen->tap(isDown);
    }
}

void DemoEngine::setBPM(int bpm) {
    std::lock_guard<std::mutex> lock(mLock);
    if(mSoundGen) {
        mSoundGen->setBpm(bpm);
    }
}

oboe::DataCallbackResult
LatTuneCallback::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    if(oboeStream != mStream){
        mStream = oboeStream;
        mLatencyTuner = std::make_unique<oboe::LatencyTuner>(*oboeStream);
    }
    if(mBufferTuneEnabled && mLatencyTuner && oboeStream->getAudioApi() == oboe::AudioApi::AAudio){
        mLatencyTuner->tune();
    }
    auto underrunCountResult = oboeStream->getXRunCount();
    int bufferSize = oboeStream->getBufferSizeInFrames();

    if(Trace::isEnabled()) Trace::beginSection("numFrames %d, Underruns %d, buffer size %d",numFrames,underrunCountResult.value(),bufferSize);
    auto result = DefaultDataCallback::onAudioReady(oboeStream,audioData,numFrames);
    if(Trace::isEnabled()) Trace::endSection();
    return result;
}

SoundGen::SoundGen(int32_t sampleRate, int32_t channelCount) :
        TappableAudioSource(sampleRate,channelCount),
        mOscillators(std::make_unique<Oscillator[]>(channelCount)){
    double frequency = 440.0;
    constexpr double interval = 110.0;
    constexpr float amplitude = 1.0;

    for(int i = 0; i < mChannelCount; ++i){
        mOscillators[i].setFrequency(frequency);
        mOscillators[i].setSampleRate(mSampleRate);
        mOscillators[i].setAmplitude(amplitude);
        frequency += interval;
    }
}

void SoundGen::tap(bool isOn) {
    for(int i = 0; i < mChannelCount; ++i){
        mOscillators[i].setWaveOn(isOn);
    }
}

void SoundGen::renderAudio(float *audioData, int32_t numFrames) {
    samplesPerBeat = (int) (oboe::DefaultStreamValues::SampleRate * 60.0 / bpm);
    std::fill_n(mBuffer.get(),kSharedBufferSize,0);
    for(int i = 0; i < mChannelCount; ++i){
        mOscillators[i].renderAudio(mBuffer.get(),numFrames);
        for(int j = 0; j < numFrames; ++j){
            samplesElapsed++;
            if(samplesElapsed >= samplesPerBeat){
                isBeatOn = !isBeatOn;
                samplesElapsed = 0;
            }
            if(isBeatOn and samplesElapsed < oboe::DefaultStreamValues::SampleRate / 10) {
                audioData[(j * mChannelCount) + i] = mBuffer[j];
            } else {
                audioData[(j * mChannelCount) + i] = 0.0f;
            }
        }
    }
}

void SoundGen::setBpm(int b) {
    bpm = b;
}
