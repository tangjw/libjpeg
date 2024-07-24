package com.github.tangjw.appjpeg

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.github.tangjw.appjpeg.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private val binding by lazy { ActivityMainBinding.inflate(layoutInflater) }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)
    }
}