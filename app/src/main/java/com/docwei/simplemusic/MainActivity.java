package com.docwei.simplemusic;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.docwei.mediaplayer.MusicPlayer;
import com.docwei.mediaplayer.listener.OnPreparedListener;

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
              Log.e("simpleMusic","onPrepared");
            }
        });

    }
    public void begin(View view){
        mMusicPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        mMusicPlayer.prepared();

    }

}
