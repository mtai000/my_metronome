package com.mtai.metronome;
import android.os.Message;
import android.os.Handler;
import android.os.HandlerThread;
import androidx.annotation.NonNull;

public abstract class BackgroundRunner {
    private final Handler mHandler;
    private final HandlerThread mThread = new HandlerThread("BackgroundRunner");

    public BackgroundRunner(){
        mThread.start();
        mHandler = new Handler(mThread.getLooper()){
            public void handleMessage(@NonNull Message message){
                super.handleMessage(message);
                handleMessageInBackground(message);
            }
        };
    }

    public boolean sendMessage(Message message){
        return mHandler.sendMessage(message);
    }

    public boolean sendMessage(int what, int arg1){
        Message message = mHandler.obtainMessage();
        message.what = what;
        message.arg1 = arg1;
        return sendMessage(message);
    }

    public boolean sendMessage(int what){
        return sendMessage(what,0);
    }
    abstract void handleMessageInBackground(Message message);

}
