package com.vegen.opencvproject

import android.Manifest
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import com.tbruyelle.rxpermissions2.RxPermissions
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        // 后面例子可能用到的权限
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            RxPermissions(this).request(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .subscribe {
                    if (it) {
                        Toast.makeText(this, "欢迎学习 OpenCV", Toast.LENGTH_SHORT).show()
                    } else {
                        Toast.makeText(this, "请赋予本项目必要的权限", Toast.LENGTH_LONG).show()
                        finish()
                    }
                }
        } else {
            Toast.makeText(this, "欢迎学习 OpenCV", Toast.LENGTH_SHORT).show()
        }

        btn1.setOnClickListener {
            ShowResultActivity.start(this, 1)
        }

        btn2.setOnClickListener {
            ShowResultActivity.start(this, 2)
        }
    }
}
