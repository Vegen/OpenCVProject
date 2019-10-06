package com.vegen.opencvproject

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.drawable.LayerDrawable
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_show_result.*

/**
 * Created by vegen on 2019/10/5.
 * Description: 显示效果的页面
 */
class ShowResultActivity : AppCompatActivity() {

    companion object {
        fun start(context: Context, button: Int) {
            val intent = Intent(context, ShowResultActivity::class.java)
            intent.putExtra("whichButton", button)
            context.startActivity(intent)
        }
    }

    private var whichButton: Int = 0        // 首页的哪个按钮来的
    private lateinit var originBitmap: Bitmap
    private lateinit var resultBitmap: Bitmap

    private lateinit var layerDrawable: Bitmap

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        whichButton = intent.getIntExtra("whichButton", 0)
        setContentView(R.layout.activity_show_result)

        originBitmap = BitmapFactory.decodeResource(resources, R.drawable.timg1)
        iv_origin_pic.setImageBitmap(originBitmap)

        var options: BitmapFactory.Options = BitmapFactory.Options()
        // 复用 Bitmap
        options.inMutable = true
        resultBitmap = BitmapFactory.decodeResource(resources, R.drawable.timg1, options)

        layerDrawable = BitmapFactory.decodeResource(resources, R.drawable.ic_layer)

        showResult()
    }

    /**
     * 显示效果
     */
    private fun showResult() {
        when (whichButton) {
            1 -> {
                // 转灰度图
                title = "转灰度图"
                ResultUtil.toGray(resultBitmap)
            }
            2-> {
                title = "底片效果"
                ResultUtil.negative(resultBitmap)
            }
            3-> {
                title = "图层叠加"
                ResultUtil.layerOverlay(resultBitmap, layerDrawable)
            }
            else -> {
                title = "显示效果的页面"
            }
        }
        iv_result_pic.setImageBitmap(resultBitmap)
    }


}
