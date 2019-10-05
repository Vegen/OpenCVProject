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
     * 转灰度图
     */
    external fun toGray(bitmap: Bitmap): Int

    /**
     * 底片效果
     */
    external fun negative(bitmap: Bitmap): Int

}
