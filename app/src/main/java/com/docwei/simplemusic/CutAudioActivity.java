package com.docwei.simplemusic;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.docwei.mediaplayer.MusicPlayer;
import com.docwei.mediaplayer.listener.OnPcmInfoListener;
import com.docwei.mediaplayer.listener.OnPreparedListener;
import com.docwei.mediaplayer.listener.OnTimeInfoListener;

import java.io.File;

/**
 * Created by liwk on 2020/11/7.
 */
public class CutAudioActivity extends AppCompatActivity {

    private MusicPlayer mMusicPlayer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cut);
        mMusicPlayer = new MusicPlayer();
        mMusicPlayer.setOnPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                mMusicPlayer.cutAudioPlay(20,40,true);
            }
        });
        mMusicPlayer.setOnPcmInfoListener(new OnPcmInfoListener() {
            @Override
            public void onPcmInfo(byte[] buffer, int buffersize) {
               //  Log.e("player","buffersize-->"+buffersize);
            }

            @Override
            public void onPcmRate(int sampleRate, int channel, int bit) {
                Log.e("player","sampleRate-->"+sampleRate);
            }
        });
        mMusicPlayer.setOnTimeInfoListener(new OnTimeInfoListener() {
            @Override
            public void onTime(int currentTime, int totalTime) {
                Log.e("player","currentTime-->"+currentTime);
            }
        });

    }

    public void cutAudio(View view) {
        File file = new File("/storage/emulated/0/$MuMu共享文件夹/yun.ape");
        mMusicPlayer.setSource(file.getAbsolutePath());
        mMusicPlayer.prepared();
    }
}
