package com.github.tangjw.appjpeg

import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.github.tangjw.appjpeg.databinding.ActivityMainBinding
import com.github.tangjw.libjpeg.JpegUtils
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.File

class MainActivity : AppCompatActivity() {
    private val binding by lazy { ActivityMainBinding.inflate(layoutInflater) }

    private val fileJpeg by lazy { File(filesDir, "test.jpg") }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)
        binding.btnTest.setOnClickListener {
            test0()
        }
        binding.btnTest1.setOnClickListener {
            test1()
        }
        lifecycleScope.launch(Dispatchers.IO) {
            assets.open("test6000x4000.jpg").use { input ->
                fileJpeg.outputStream().use { output ->
                    input.copyTo(output)
                }
            }
        }

    }

    private fun test0() {
        val l = System.currentTimeMillis()
        JpegUtils.tJpegDecodeFile(null, fileJpeg.absolutePath, 0)?.let {
            Log.e("test0", "$it")
        }
        Log.e("test0", "${System.currentTimeMillis() - l}")
    }

    private fun test1() {
        val l = System.currentTimeMillis()
        BitmapFactory.decodeFile(fileJpeg.absolutePath)?.let {
            Log.e("test1", "${it.width}*${it.height}")
        }
        Log.e("test1", "${System.currentTimeMillis() - l}")
    }

}