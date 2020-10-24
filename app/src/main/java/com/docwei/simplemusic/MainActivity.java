package com.docwei.simplemusic;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.docwei.mediaplayer.MusicPlayer;
import com.docwei.mediaplayer.listener.OnPreparedListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {


    private MusicPlayer mMusicPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mMusicPlayer = new MusicPlayer();
        mMusicPlayer.setOnPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
              mMusicPlayer.start();
            }
        });

    }
    public void begin(View view){
       // mMusicPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        File file=new File("/storage/emulated/0/$MuMu共享文件夹/1.mp3");
        mMusicPlayer.setSource(file.getAbsolutePath());
        mMusicPlayer.prepared();

    }

}
