package com.github.tangjw.libjpeg;

public class JpegInfo {

    private byte[] pixels;
    private int width;
    private int height;

    public byte[] getPixels() {
        return pixels;
    }

    public void setPixels(byte[] pixels) {
        this.pixels = pixels;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public JpegInfo() {
    }

    public JpegInfo(byte[] pixels, int width, int height) {
        this.pixels = pixels;
        this.width = width;
        this.height = height;
    }

    @Override
    public String toString() {
        return "JpegInfo@" + Integer.toHexString(hashCode()) +
                ": " + width + "*" + height;
    }
}
