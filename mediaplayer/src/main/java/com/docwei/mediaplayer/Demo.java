package com.docwei.mediaplayer;

/**
 * Created by liwk on 2020/8/19.
 */
public class Demo {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avdevice");
        System.loadLibrary("avutil");
        System.loadLibrary("avresample");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }

    public native static String stringFromJNI();
    public native static void testFfmpeg();

}
