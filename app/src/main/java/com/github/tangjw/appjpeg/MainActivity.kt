package com.github.tangjw.appjpeg

import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.exifinterface.media.ExifInterface
import com.github.tangjw.appjpeg.databinding.ActivityMainBinding
import com.github.tangjw.libjpeg.JpegInfo
import com.github.tangjw.libjpeg.JpegUtils
import java.io.File
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity() {
    private val binding by lazy { ActivityMainBinding.inflate(layoutInflater) }

    private val jpegFile by lazy { File(filesDir, "test.jpg") }


    private lateinit var bytes: ByteArray
    private lateinit var buffer: ByteBuffer

    private val header = byteArrayOf(0, 1, 2, 3)

    private val jpegBytes by lazy { header + bytes }

    private val jpegList = mutableListOf<JpegInfo>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)
        binding.btnTest1.setOnClickListener {
            test1()
        }
        binding.btnTest2.setOnClickListener {
            test2()
        }

        assets.open("test2.jpg").use {
            bytes = it.readBytes()
            buffer = ByteBuffer.allocateDirect(header.size + bytes.size).apply {
                put(header)
                put(bytes)
            }
            jpegFile.outputStream().use { out ->
                out.write(bytes)
            }
        }
    }

    private var jpegInfo: JpegInfo? = null

    private fun test1() {
        val l = System.currentTimeMillis()
        /*jpegInfo = JpegUtils.decodeBytes(jpegBytes, header.size, bytes.size, 0, jpegInfo)
        Log.e(
            "JpegUtils",
            "decodeBytes: ${System.currentTimeMillis() - l}ms, ${jpegInfo?.width}*${jpegInfo?.height}"
        )
        jpegList.add(jpegInfo!!)*/

        /*jpegInfo = JpegUtils.decodeBuffer(buffer, header.size, bytes.size, 0, jpegInfo)
        Log.e(
            "JpegUtils",
            "decodeBytes: ${System.currentTimeMillis() - l}ms, ${jpegInfo?.width}*${jpegInfo?.height}"
        )
        jpegList.add(jpegInfo!!)*/
        val orientation = ExifInterface(jpegFile).getAttributeInt(
            ExifInterface.TAG_ORIENTATION,
            ExifInterface.ORIENTATION_UNDEFINED
        )
        JpegUtils.compressFile(jpegFile, orientation, 100, File(cacheDir, "test_out.jpg").absolutePath)
    }

    fun test2() {
        var l = System.currentTimeMillis()
        val orientation = ExifInterface(jpegFile).getAttributeInt(
            ExifInterface.TAG_ORIENTATION,
            ExifInterface.ORIENTATION_UNDEFINED
        )
        Log.e("test0", "${System.currentTimeMillis() - l} => $orientation")
        l = System.currentTimeMillis()

        JpegUtils.decodeFile(jpegFile, orientation, null)?.let {
            Log.e("test0", "$it")
        }
        Log.e("test0", "${System.currentTimeMillis() - l}")
    }

    fun test01() {
        val l = System.currentTimeMillis()
        JpegUtils.decodeBytes(jpegBytes, 0, null)?.let {
            Log.e("test0", "$it")
            //jpegList.add(it)
        }
        Log.e("test0", "${System.currentTimeMillis() - l}")
    }

    fun test0() {
        val l = System.currentTimeMillis()
        BitmapFactory.decodeFile(jpegFile.absolutePath)?.let {
            Log.e("test1", "${it.width}*${it.height}")
        }
        Log.e("test1", "${System.currentTimeMillis() - l}")
    }

}