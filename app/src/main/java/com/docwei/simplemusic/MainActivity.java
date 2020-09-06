package com.docwei.simplemusic;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.docwei.mediaplayer.Demo;

public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Demo demo=new Demo();
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(demo.stringFromJNI());
        demo.testFfmpeg();
    }


}
