package com.mtai.metronome;

import android.os.Bundle;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.SwitchCompat;

import org.w3c.dom.Text;

public class MetronomeFragment extends AbstractFragment {
    View view;

    private static final String TAG = "metronome";

    private static final int STATUS_START = 100;
    private static final int STATUS_STOP = 101;
    private static final int STATUS_SET_DEVICE_ID = 102;
    private static final int STATUS_SET_AUDIO_API = 103;
    private static final int STATUS_SET_CHANNEL_COUNT = 104;
    private static final int STATUS_SET_BPM = 105;

    private BackgroundRunner mRunner = new MyBackgroundRunner();
    private class MyBackgroundRunner extends BackgroundRunner{
        int audioApi;
        int deviceId;
        int channelCount;
        @Override
        void handleMessageInBackground(Message msg){
            int what = msg.what;
            int arg1 = msg.arg1;
            Log.i(TAG,"got background msg,what = " + what + ", arg1 = " + arg1);

            int result = 0;
            boolean restart = false;

            switch (what){
                case STATUS_START:
                    result = AudioEngine.demoStartEngine(audioApi,deviceId,channelCount);
                    break;
                case STATUS_STOP:
                    result = AudioEngine.demoStopEngine();
                    break;
                case STATUS_SET_BPM:
                    AudioEngine.demoSetBPM(msg.arg1);
                    break;
                case STATUS_SET_AUDIO_API:
                    if(audioApi != arg1){
                        audioApi = arg1;
                        restart = true;
                    }
                    break;
                case STATUS_SET_DEVICE_ID:
                    if(deviceId != arg1){
                        deviceId = arg1;
                        restart = true;
                    }
                    break;
                case STATUS_SET_CHANNEL_COUNT:
                    if(channelCount != arg1){
                        channelCount = arg1;
                        restart = true;
                    }
                    break;
            }

            if(restart){
                int result1 = AudioEngine.demoStopEngine();
                int result2 = AudioEngine.demoStartEngine(audioApi,deviceId,channelCount);
                result = (result2 != 0) ? result2: result1;
            }
            if(result != 0){
                Log.e(TAG,"audio error " + result);
            }
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle state){
        return inflater.inflate(R.layout.metronome,container,false);
    }

    @Override
    public void onViewCreated(@NonNull View view, Bundle state){
        this.view = view;
        initSwitch();
        initBpmBar();
        startAudioAsync();
    }

    private void initBpmBar(){
        SeekBar bar = this.view.findViewById(R.id.bmpBar);
        bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                TextView tv = MetronomeFragment.this.view.findViewById(R.id.bmpTextView);
                tv.setText(String.valueOf(bar.getProgress()));
                updateAudioBpm(bar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        this.view.findViewById(R.id.Add10).setOnClickListener(v -> bar.setProgress(bar.getProgress() + 10));
        this.view.findViewById(R.id.Add5 ).setOnClickListener(v -> bar.setProgress(bar.getProgress() + 5 ));
        this.view.findViewById(R.id.Add1 ).setOnClickListener(v -> bar.setProgress(bar.getProgress() + 1 ));
        this.view.findViewById(R.id.Sub1 ).setOnClickListener(v -> bar.setProgress(bar.getProgress() - 1 ));
        this.view.findViewById(R.id.Sub5 ).setOnClickListener(v -> bar.setProgress(bar.getProgress() - 5 ));
        this.view.findViewById(R.id.Sub10).setOnClickListener(v -> bar.setProgress(bar.getProgress() - 10));

    }

    private void initSwitch(){
        SwitchCompat sc = this.view.findViewById(R.id.Switch);
        sc.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if(isChecked){
                startMetronome();
            }
            else
            {
                stopMetronome();
            }
        });
    }

    private void startMetronome(){
        System.out.println("Start metronome");
        //AudioEngine.playDemo(AudioEngine.handle);
        AudioEngine.demoSetToneOn(true);
    }
    private void stopMetronome(){
        System.out.println("Stop metronome");
        AudioEngine.demoSetToneOn(false);
    }

    void startAudioAsync(){
        System.out.println("start audio engine");
        mRunner.sendMessage(STATUS_START);
    }

    void stopAudioAsync(){
        mRunner.sendMessage(STATUS_STOP);
    }

    void updateAudioBpm(int bpm){
        mRunner.sendMessage(STATUS_SET_BPM,bpm);
    }
}
