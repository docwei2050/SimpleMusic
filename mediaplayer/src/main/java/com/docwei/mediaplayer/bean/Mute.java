package com.docwei.mediaplayer.bean;

/**
 * Created by liwk on 2020/10/27.
 */
public enum Mute {
    MUTE_LEFT("left", 1), MUTE_RIGHT("right", 0), MUTE_CENTER("center", 2);
    private String name;
    private int value;

    Mute(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
