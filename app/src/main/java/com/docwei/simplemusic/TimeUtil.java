package com.docwei.simplemusic;

/**
 * Created by liwk on 2020/10/24.
 */
public class TimeUtil {
    public static String seconds2Format(int seconds) {
        long hour = seconds / (60 * 60);
        long minute = (seconds % (60 * 60)) / 60;
        long second = seconds % 60;
        String sh = "00";
        if (hour > 0) {
            if (hour < 10) {
                sh = "0" + hour;
            } else {
                sh = hour + "";
            }
        }
        String sm = "00";
        if (minute > 0) {
            if (minute < 10) {
                sm = "0" + minute;
            } else {
                sm = minute + "";
            }
        }
        String sc = "00";
        if (second > 0) {
            if (second < 10) {
                sc = "0" + second;
            } else {
                sc = second + "";
            }
        }
        if (seconds >= 3600) {
            return sh + ":" + sm + ":" + sc;
        }
        return sm + ":" + sc;
    }
}
