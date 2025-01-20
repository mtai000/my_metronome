package com.mtai.metronome;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.core.app.ActivityCompat;

public class TunerFragment extends AbstractFragment {
    private PitchView pv;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle state) {
        return inflater.inflate(R.layout.tuner, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle state) {
        TextView tv = view.findViewById(R.id.textView);
        String str = AudioEngine.stringFromJNI();
        tv.setText(str);

        FrameLayout pitchViewContainer = view.findViewById(R.id.pitchViewContainer);
        pv = new PitchView(requireContext(),this);


        pitchViewContainer.addView(pv);
    }


    public void getMicPression(){
        if (ActivityCompat.checkSelfPermission(this.getContext(), android.Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this.getActivity(),
                    new String[]{android.Manifest.permission.RECORD_AUDIO},
                    1001
            );
        }
    }

}
