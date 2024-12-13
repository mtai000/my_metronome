package com.mtai.metronome;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class TunerFragment extends AbstractFragment {
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle state){
        return inflater.inflate(R.layout.tuner,container,false);
    }

    @Override
    public void onViewCreated(View view, Bundle state){
        TextView tv = view.findViewById(R.id.textView);
        String str = AudioEngine.stringFromJNI();
        tv.setText(str);
    }
}
