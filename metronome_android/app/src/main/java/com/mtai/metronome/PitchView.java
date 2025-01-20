package com.mtai.metronome;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Switch;

import androidx.appcompat.widget.SwitchCompat;
import androidx.core.app.ActivityCompat;

public class PitchView extends View {
    public TunerFragment tf;
    private BackgroundRunner pitchRunner;

    private Paint linePaint;
    private Paint textPaint;
    private Paint pitchTextPaint;
    private Paint areaPaint;
    private Paint bingoPaint;
    private Paint sliderPaint;
    private int centerX;
    private int centerY;
    private Rect drawrect;
    private Rect bingorect;
    private boolean isOnTuner = false;
    private double detectPitch;

    int sampleRate;
    private final int SAMPLES = 1024;
    private short[] data;
    private AudioRecord recorder;
    private boolean bPitch = false;
    private float bingoK = 0.1f;


    private final BackgroundRunner mRunner = new PitchView.LoopRunner();

    private class LoopRunner extends BackgroundRunner {
        @Override
        void handleMessageInBackground(Message msg) {
            int what = msg.what;
            int arg1 = msg.arg1;
            if (!bPitch) return;
            int read = recorder.read(data, 0, data.length);
            if (read > 0) {
                double pitch = AudioEngine.getPitchFromShort(data, sampleRate);

                new Handler(Looper.getMainLooper()).post(() -> {
                    Log.i("detect pitch", "get pitch: " + String.valueOf(pitch));
                    detectPitch = pitch;
                    if (checkPitch(pitch)) {
                        invalidate();
                    }
                });
            }
            sendMessage(0, 0);
        }
    }

    public PitchView(Context context, TunerFragment fragment) {
        super(context);
        this.tf = fragment;
        init();
    }

    private boolean checkPitch(double pitch) {
        return pitch > 20 && pitch < 2000;
    }

    private void init() {
        if (linePaint == null) {
            linePaint = new Paint();
            linePaint.setColor(Color.BLACK);
            linePaint.setStrokeWidth(5);
        }
        if (textPaint == null) {
            textPaint = new Paint();
            textPaint.setColor(Color.BLACK);
            textPaint.setTextSize(40);
            textPaint.setTextAlign(Paint.Align.CENTER);
        }
        if (pitchTextPaint == null) {
            pitchTextPaint = new Paint();
            pitchTextPaint.setColor(Color.BLACK);
            pitchTextPaint.setTextSize(40);
            pitchTextPaint.setTextAlign(Paint.Align.CENTER);

        }
        areaPaint = new Paint();
        bingoPaint = new Paint();
        sliderPaint = new Paint();
        sliderPaint.setColor(Color.BLUE);

        initPitchDetect();
    }

    private int getMaxValidSampleRate() {
        int maxRate = 0;
        for (int rate : new int[]{8000, 11025, 16000, 22050}) {
            int bufferSize = AudioRecord.getMinBufferSize(rate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
            if (bufferSize > 0) {
                maxRate = rate;
            }
        }
        return maxRate;
    }

    public void initPitchDetect() {
        sampleRate = getMaxValidSampleRate();
        tf.getMicPression();
        SwitchCompat sw = tf.getActivity().findViewById(R.id.pitchSW);
        sw.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                startRecording();
            } else {
                stopRecording();
            }
        });
    }

    private void stopRecording() {
        recorder.stop();
        recorder.release();
        bPitch = false;
        Log.i("pitch view", "stop recording");
    }

    private void startRecording() {
        int n = AudioRecord.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
        data = new short[SAMPLES];
        if (ActivityCompat.checkSelfPermission(this.getContext(), android.Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            SwitchCompat sw = tf.getActivity().findViewById(R.id.pitchSW);
            sw.setChecked(false);
            return;
        }
        recorder = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, n * 10);
        recorder.startRecording();
        bPitch = true;
        Log.i("pitch view", "send msg");
        mRunner.sendMessage(0, 0);
    }


    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        centerX = w / 2;
        centerY = h - 30;

        int offset = 20;
        drawrect = new Rect();
        drawrect.top = centerY - offset;
        drawrect.bottom = centerY + offset;
        drawrect.left = centerX - w * 2 / 5;
        drawrect.right = centerX + w * 2 / 5;

        bingorect = new Rect();
        bingorect.top = drawrect.top;
        bingorect.bottom = drawrect.bottom;
        bingorect.left = (int) (centerX - bingoK * drawrect.width() / 2f);
        bingorect.right = (int) (centerX + bingoK * drawrect.width() / 2f);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        canvas.save();
        areaPaint.setColor(Color.GRAY);
        canvas.drawRect(drawrect, areaPaint);

        canvas.drawLine(drawrect.left, drawrect.top - 5, drawrect.left, drawrect.bottom + 5, linePaint);
        canvas.drawLine(drawrect.right, drawrect.top - 5, drawrect.right, drawrect.bottom + 5, linePaint);
        canvas.drawLine(centerX, drawrect.top - 5, centerX, drawrect.bottom + 5, linePaint);

        double offset = getOffset(detectPitch);
        int idx = 48 + (int) Math.round(offset);
        double tmpK = (48 + offset) % 1;
        canvas.drawText(note_names[idx - 1], drawrect.left, drawrect.top - 40, pitchTextPaint);
        canvas.drawText(note_names[idx], centerX, drawrect.top - 40, pitchTextPaint);
        canvas.drawText(note_names[idx + 1], drawrect.right, drawrect.top - 40, pitchTextPaint);

        if (Math.abs(tmpK) < bingoK) {
            bingoPaint.setColor(Color.GREEN);
        } else {
            bingoPaint.setColor(Color.RED);
        }

        canvas.drawRect(bingorect, bingoPaint);

        float sliderPosition = (float) (centerX + tmpK * drawrect.width() / 2);
        float sliderRadius = 20.0f;
        canvas.drawCircle(sliderPosition, (drawrect.top + drawrect.bottom) / 2f, sliderRadius, sliderPaint);

        canvas.restore();
    }

    // A4 at [48]
    private final String[] note_names = {
            "A0", "A#0", "B0", "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1",
            "A1", "A#1", "B1", "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2",
            "A2", "A#2", "B2", "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3",
            "A3", "A#3", "B3", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4",
            "A4", "A#4", "B4", "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5",
            "A5", "A#5", "B5", "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6",
            "A6", "A#6", "B6", "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7",
            "A7", "A#7", "B7", "C8"
    };
    private final double A4 = 440.0;

    // A4 = 440.0hz
    // pitch = A4 * 2 ^ (offset / 12)
    // A3 = A4 * 2 ^ ( (36 - 48) / 12) = 220hz
    // 2 ^ (offset / 12) = pitch / A4
    private double getOffset(double pitch) {
        return Math.log(pitch / A4) / Math.log(2) * 12;
    }

    private int center_note_idx(double pitch) {
        double offset = getOffset(pitch);
        return (int) Math.round(offset);
    }
}
