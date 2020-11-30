package com.docwei.simplemusic;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.docwei.mediaplayer.MusicPlayer;
import com.docwei.mediaplayer.bean.Mute;
import com.docwei.mediaplayer.listener.OnLoadListener;
import com.docwei.mediaplayer.listener.OnPlayStatusListener;
import com.docwei.mediaplayer.listener.OnPreparedListener;
import com.docwei.mediaplayer.listener.OnTimeInfoListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {


    private MusicPlayer mMusicPlayer;
    private TextView mTv_time;
    private TextView mTv_time1;
    private SeekBar mSeekBar_time;
    private TextView mTv_volumn;
    private SeekBar mSeekBar_volumn;
    private boolean isHandleSeekTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTv_time = findViewById(R.id.tv_time);

        mTv_time1 = findViewById(R.id.tv_time);
        mSeekBar_time = findViewById(R.id.time);
        mTv_volumn = findViewById(R.id.tv_volumn);
        mSeekBar_volumn = findViewById(R.id.volumn);


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
                        if (!isHandleSeekTime) {
                            mSeekBar_time.setProgress((currentTime * 100 / totalTime));
                        }
                    }
                });
            }
        });
        mSeekBar_time.setProgress(0);
        mSeekBar_time.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && isHandleSeekTime) {
                    mMusicPlayer.seek(seekBar.getProgress() * mMusicPlayer.getDuration() / 100);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isHandleSeekTime = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isHandleSeekTime = false;

            }
        });

        mMusicPlayer.setVolumnPercent(50);
        mSeekBar_volumn.setProgress(50);
        mSeekBar_volumn.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mMusicPlayer.setVolumnPercent(progress);
                mTv_volumn.setText(progress + "%");

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });


    }

    public void begin(View view) {
        //mMusicPlayer.setSource("http://jzvd.nathen.cn/video/4542c17b-170c25a8e14-0007-1823-c86-de200.mp4");
        File file = new File("/storage/emulated/0/$MuMu共享文件夹/sss.mp4");
         mMusicPlayer.setSource(file.getAbsolutePath());
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


    public void right(View view) {
        mMusicPlayer.setMute(Mute.MUTE_RIGHT);
    }

    public void center(View view) {
        mMusicPlayer.setMute(Mute.MUTE_CENTER);
    }

    public void left(View view) {
        mMusicPlayer.setMute(Mute.MUTE_LEFT);
    }
}
