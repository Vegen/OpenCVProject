package com.vegen.opencvproject

import android.graphics.Bitmap

/**
 * Created by vegen on 2019/10/5.
 * Description:
 */
object ResultUtil {

    init {
        System.loadLibrary("native-lib")
    }

    /**
     * 转灰度
     * @param bitmap
     */
    external fun toGray(bitmap: Bitmap): Int

}
