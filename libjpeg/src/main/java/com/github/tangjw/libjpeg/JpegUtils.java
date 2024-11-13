package com.github.tangjw.libjpeg;

import android.util.Log;

import org.jetbrains.annotations.Nullable;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;

public class JpegUtils {
    static {
        System.loadLibrary("libjpegutils");
    }

    private native static int tJpegCompressFile(String path, int size, int exifOrientation, int quality, String outPath) throws IOException;

    private native static int tJpegCompressTexture(int textureId, int width, int height, int quality, String outPath);

    private native static int tJpegCompressBytes(byte[] pixelsArray, int offset, int width, int height, int quality, String outPath);

    private native static int tJpegCompressDirectBuffer(ByteBuffer byteBuffer, int offset, int width, int height, int quality, String outPath);

    private native static int tJpegDecompressFile(String path, int size, int exifOrientation, JpegInfo jpegInfo);

    private native static int tJpegDecompressByteArray(byte[] byteArray, int offset, int length, int exifOrientation, JpegInfo jpegInfo);

    private native static int tJpegDecompressDirectBuffer(ByteBuffer byteBuffer, int offset, int length, int exifOrientation, JpegInfo jpegInfo);

    public static void compressFile(File jpegFile, int exifOrientation, int quality, String outPath) throws IOException {
        long fileSize = 0;
        if (jpegFile.exists() && jpegFile.isFile()) {
            fileSize = jpegFile.length();
        }
        if (fileSize <= 0L) {
            throw new IOException("文件不存在或不是有效的文件");
        }
        if (fileSize > Integer.MAX_VALUE) {
            throw new IOException("文件过大: " + (fileSize / 1024 / 1024) + "MB");
        }
        Log.e("tJpegCompressFile","fileSize: "+fileSize);
        int ret = tJpegCompressFile(jpegFile.getAbsolutePath(), (int) fileSize, exifOrientation, quality, outPath);
        checkResult(ret);
    }

    public static void compressTexture(int textureId, int width, int height, int quality, String outPath) throws IOException {
        int ret = tJpegCompressTexture(textureId, width, height, quality, outPath);
        checkResult(ret);
    }

    public static void compressPixels(byte[] pixelsArray, int width, int height, int quality, String outPath) throws IOException {
        compressPixels(pixelsArray, 0, width, height, quality, outPath);
    }

    public static void compressPixels(byte[] pixelsArray, int offset, int width, int height, int quality, String outPath) throws IOException {
        if (pixelsArray.length != offset + width * height * 4) {
            throw new IOException("pixelsArray 必须包含 rgba: width*height*4");
        }
        int ret = tJpegCompressBytes(pixelsArray, offset, width, height, quality, outPath);
        checkResult(ret);
    }

    public static void compressPixels(ByteBuffer pixelsBuffer, int width, int height, int quality, String outPath) throws IOException {
        compressPixels(pixelsBuffer, 0, width, height, quality, outPath);
    }

    public static void compressPixels(ByteBuffer pixelsBuffer, int offset, int width, int height, int quality, String outPath) throws IOException {
        if (pixelsBuffer.capacity() < offset + width * height * 4) {
            throw new IOException("pixelsBuffer 必须包含 rgba: width*height*4");
        }
        int ret;
        if (pixelsBuffer.isDirect()) {
            ret = tJpegCompressDirectBuffer(pixelsBuffer, offset, width, height, quality, outPath);
        } else {
            ret = tJpegCompressBytes(pixelsBuffer.array(), offset, width, height, quality, outPath);
        }
        checkResult(ret);
    }

    public static JpegInfo decodeFile(File jpegFile, int exifOrientation, @Nullable JpegInfo jpegInfo) throws IOException {
        long fileSize = 0;
        if (jpegFile.exists() && jpegFile.isFile()) {
            fileSize = jpegFile.length();
        }
        if (fileSize <= 0L) {
            throw new IOException("文件不存在或不是有效的文件");
        }
        if (fileSize > Integer.MAX_VALUE) {
            throw new IOException("文件过大: " + (fileSize / 1024 / 1024) + "MB");
        }
        if (jpegInfo == null) {
            jpegInfo = new JpegInfo();
        }
        int ret = tJpegDecompressFile(jpegFile.getAbsolutePath(), (int) fileSize, exifOrientation, jpegInfo);
        checkResult(ret);
        return jpegInfo;
    }

    public static JpegInfo decodeBuffer(ByteBuffer jpegBuffer, int exifOrientation, @Nullable JpegInfo jpegInfo) throws IOException {
        return decodeBuffer(jpegBuffer, 0, jpegBuffer.capacity(), exifOrientation, jpegInfo);
    }

    public static JpegInfo decodeBuffer(ByteBuffer jpegBuffer, int offset, int length, int exifOrientation, @Nullable JpegInfo jpegInfo) throws IOException {
        if (jpegBuffer.capacity() < offset + length) {
            throw new IOException("jpegBuffer不合法: capacity < offset+length");
        }
        if (jpegInfo == null) {
            jpegInfo = new JpegInfo();
        }
        int ret;
        if (jpegBuffer.isDirect()) {
            ret = tJpegDecompressDirectBuffer(jpegBuffer, offset, length, exifOrientation, jpegInfo);
        } else {
            ret = tJpegDecompressByteArray(jpegBuffer.array(), offset, length, exifOrientation, jpegInfo);
        }
        checkResult(ret);
        return jpegInfo;
    }

    public static JpegInfo decodeBytes(byte[] jpegBytes, int exifOrientation, @Nullable JpegInfo jpegInfo) throws IOException {
        return decodeBytes(jpegBytes, 0, jpegBytes.length, exifOrientation, jpegInfo);
    }

    public static JpegInfo decodeBytes(byte[] jpegBytes, int offset, int length, int exifOrientation, @Nullable JpegInfo jpegInfo) throws IOException {
        if (offset >= 0 && length > 0 && jpegBytes.length >= offset + length) {
            if (jpegInfo == null) {
                jpegInfo = new JpegInfo();
            }
            int ret = tJpegDecompressByteArray(jpegBytes, offset, length, exifOrientation, jpegInfo);
            checkResult(ret);
            return jpegInfo;
        }
        throw new IOException("jpegBuffer不合法: capacity < offset + length");
    }

    private static void checkResult(int ret) throws IOException {
        switch (ret) {
            case -11:
                throw new IOException("压缩函数执行失败: tj3Compress8()");
            case -12:
                throw new IOException("解压函数执行失败: tj3Decompress8()");
            case -13:
                throw new IOException("解压函数执行失败: tj3DecompressHeader()");
            case -14:
                throw new IOException("变换函数执行失败: tj3Transform()");
            case -15:
                throw new IOException("需要处理的图片分辨率过大 超过 10 亿像素");
            case -16:
                throw new IOException("gl函数执行失败");
        }
    }
}
