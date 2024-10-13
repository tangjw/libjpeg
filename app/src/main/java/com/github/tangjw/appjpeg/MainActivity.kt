package com.github.tangjw.appjpeg

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.github.tangjw.appjpeg.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private val binding by lazy { ActivityMainBinding.inflate(layoutInflater) }

    val bitmapSrc by lazy {
        BitmapFactory.decodeFile("${filesDir.absolutePath}/1719136138096.jpg")
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)
        binding.btnTest.setOnClickListener {
            test()
        }
    }

    private fun test() {
        // 1844 1240
//        if (bitmapSrc.width.toFloat() / bitmapSrc.height.toFloat() > 2656f / 1844f) {
//
//        }
       // val bitmap = scaleAndDrawBitmap(bitmapSrc,1240,  (3984f/(2656f/1240f)).toInt())   //1240 1859  -15
        val bitmap = rotateAndScaleBitmap(bitmapSrc,90f,  1240,1860)
        println(bitmap.width)
        println(bitmap.height)
        binding.ivImage.setImageBitmap(bitmap)
    }

    fun scaleAndDrawBitmap(
        originalBitmap: Bitmap,
        targetWidth: Int,
        targetHeight: Int
    ): Bitmap {
        // 创建一个指定大小的 Bitmap
        val scaledBitmap =
            Bitmap.createScaledBitmap(originalBitmap, targetWidth, targetHeight, true)

        // 创建一个空的 Bitmap，大小与缩放后的 Bitmap 相同
//        val resultBitmap = Bitmap.createBitmap(targetWidth, targetHeight, originalBitmap.config)
//
        // 使用 Canvas 绘制缩放后的 Bitmap
//        val canvas = Canvas(resultBitmap)
//        canvas.drawBitmap(scaledBitmap, 0f, 0f, null)

        return scaledBitmap
    }

    fun rotateAndScaleBitmap(
        originalBitmap: Bitmap,
        degrees: Float,
        targetWidth: Int,
        targetHeight: Int
    ): Bitmap {
        // 创建一个 Matrix 对象
        val matrix = Matrix()

        // 设置旋转角度
        matrix.postRotate(degrees)

        // 计算缩放比例
        val scaleWidth = targetWidth.toFloat() / originalBitmap.width
        val scaleHeight = targetHeight.toFloat() / originalBitmap.height

        // 设置缩放比例
        matrix.postScale(scaleWidth, scaleHeight)

        // 创建旋转和缩放后的 Bitmap
        val dstBitmap = Bitmap.createBitmap(
            originalBitmap, 0, 0, originalBitmap.width, originalBitmap.height, matrix, true
        )
       // dstBitmap.getPixel(0,0)
//        return dstMap
        return Bitmap.createBitmap(dstBitmap, 8, 0, 1844, 1240)
    }
}