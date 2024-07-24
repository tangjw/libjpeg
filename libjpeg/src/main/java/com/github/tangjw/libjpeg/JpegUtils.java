package com.github.tangjw.libjpeg;

import java.io.IOException;
import java.nio.ByteBuffer;

public class JpegUtils {
    static {
        System.loadLibrary("libjpegutils");
    }

    public native static int tJpegCompressFile(JpegInfo jpegInfo, String jpegPath) throws IOException;

    public native static int tJpegCompressTexture(int texture, String jpegPath) throws IOException;

    public native static JpegInfo tJpegDecodeBuffer(
            JpegInfo jpegInfo,
            ByteBuffer jpegBuffer,
            int start, int length, int orientation) throws IOException;

    public static JpegInfo tJpegDecodeBuffer(
            JpegInfo jpegInfo,
            ByteBuffer jpegBuffer,
            int start, int length) throws IOException {
        return tJpegDecodeBuffer(jpegInfo, jpegBuffer, start, length, 0);
    }

    public native static JpegInfo tJpegDecodeFile(
            JpegInfo jpegInfo,
            String jpegPath, int orientation) throws IOException;

}
