package com.docwei.mediaplayer.util;

import android.media.MediaCodecList;
import android.util.Log;

import java.util.HashMap;

/**
 * Created by liwk on 2021/1/3.
 */
public class VideoUtil {

    public static HashMap<String, String> sMap = new HashMap<>();

    static {
        sMap.put("h264", "video/avc");
    }

    public static String findVideoCodecName(String ffcodeName) {
        if (sMap.containsKey(ffcodeName)) {
            return sMap.get(ffcodeName);
        }
        return "";
    }

    public static boolean isSupportCodecType(String ffcodecName) {
        boolean isSupport = false;
        int count = MediaCodecList.getCodecCount();
        for (int i = 0; i < count; i++) {
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                Log.e("simplePlayer",types[j]+"---------"+findVideoCodecName(ffcodecName));
                if (types[j].equals(findVideoCodecName(ffcodecName))) {
                    isSupport = true;
                    break;
                }
            }

            if (isSupport) {
                break;
            }
        }
        return isSupport;
    }
}
