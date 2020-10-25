package com.docwei.simplemusic;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.docwei.mediaplayer.MusicPlayer;
import com.docwei.mediaplayer.listener.OnLoadListener;
import com.docwei.mediaplayer.listener.OnPlayStatusListener;
import com.docwei.mediaplayer.listener.OnPreparedListener;
import com.docwei.mediaplayer.listener.OnTimeInfoListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {


    private MusicPlayer mMusicPlayer;
    private TextView mTv_time;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTv_time = findViewById(R.id.tv_time);
        mMusicPlayer = new MusicPlayer();
        mMusicPlayer.setOnPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                mMusicPlayer.start();
            }
        });
        mMusicPlayer.setOnLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if (load) {
                    Log.e("player", "加载中");
                }
            }
        });
        mMusicPlayer.setOnPlayStatusListener(new OnPlayStatusListener() {
            @Override
            public void onPause(boolean pause) {
                Log.e("player", pause ? "暂停中" : "播放中");
            }
        });
        mMusicPlayer.setOnTimeInfoListener(new OnTimeInfoListener() {
            @Override
            public void onTime(final int currentTime, final int totalTime) {
                //这里是子线程哦
                mTv_time.post(new Runnable() {
                    @Override
                    public void run() {
                        mTv_time.setText(TimeUtil.seconds2Format(currentTime) + "/" + TimeUtil.seconds2Format(totalTime));
                    }
                });
            }
        });
    }

    public void begin(View view) {
        mMusicPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        //File file = new File("/storage/emulated/0/$MuMu共享文件夹/1.mp3");
      // mMusicPlayer.setSource(file.getAbsolutePath());
        mMusicPlayer.prepared();

    }

    public void play(View view) {
        mMusicPlayer.resume();
    }

    public void pause(View view) {
        mMusicPlayer.pause();
    }

    public void stop(View view) {
        mMusicPlayer.stop();
    }

    public void seek(View view) {
        mMusicPlayer.seek(216);
    }

    public void next(View view) {

        mMusicPlayer.playNext("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
    }

}
