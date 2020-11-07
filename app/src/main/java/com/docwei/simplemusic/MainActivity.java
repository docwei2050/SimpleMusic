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
import com.docwei.mediaplayer.listener.OnTimeRecordListener;
import com.docwei.mediaplayer.listener.OnVolumnDBListener;

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
        mMusicPlayer.setOnVolumnDBListener(new OnVolumnDBListener() {
            @Override
            public void onSuccess(int db) {
                Log.e("player","db-->"+db);
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
        mMusicPlayer.setOnTimeRecordListener(new OnTimeRecordListener() {
            @Override
            public void onTime(double time) {
                Log.e("player","记录的时长--》"+time);
            }
        });


    }

    public void begin(View view) {
       // mMusicPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        File file = new File("/storage/emulated/0/$MuMu共享文件夹/1.mp3");
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

    public void speed(View view) {
        mMusicPlayer.setSpeed(1.5f);
        mMusicPlayer.setTune(1.0f);
    }

    public void normalSeep(View view) {
        mMusicPlayer.setSpeed(1.0f);
        mMusicPlayer.setTune(1.0f);
    }

    public void tune(View view) {
        mMusicPlayer.setSpeed(1.0f);
        mMusicPlayer.setTune(1.5f);
    }

    public void speedTune(View view) {
        mMusicPlayer.setSpeed(1.0f);
        mMusicPlayer.setTune(1.5f);
    }

    public void start_record(View view) {
        mMusicPlayer.startRecord(new File("/storage/emulated/0/player_1.aac"));
    }

    public void pause_record(View view) {
        mMusicPlayer.pauseRecord();
    }

    public void continue_record(View view) {
        mMusicPlayer.resumeRecord();
    }

    public void stop_record(View view) {
        mMusicPlayer.stopRecord();
    }
}
