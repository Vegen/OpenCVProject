package com.vegen.opencvproject

import android.graphics.Bitmap

/**
 * Created by vegen on 2019/10/5.
 * Description: 和 native 通信实现效果的工具类
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

    /**
     * 色值（饱和度、亮度）调节
     */
    external fun chromaChange(bitmap: Bitmap): Int

}
