package com.vegen.opencvproject

import android.graphics.Bitmap
import android.graphics.drawable.LayerDrawable

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

    /**
     * 图层叠加
     */
    external fun layerOverlay(bitmap: Bitmap, layerDrawable: Bitmap): Int

}
