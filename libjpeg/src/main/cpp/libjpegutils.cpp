#include <jni.h>
#include <string>

#include <malloc.h>
#include <cstdio>
#include <csetjmp>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "libjpeg/turbojpeg.h"
//#include "base/Log.h"

const int ERR_OK = 0;
const int ERR_COMPRESS = -11;
const int ERR_DECOMPRESS = -12;
const int ERR_DECOMPRESS_HEADER = -13;
const int ERR_TRANSFORM = -14;
const int ERR_RESOLUTION = -15;
const int ERR_GL_ERROR = -16;


int compressJpeg(uint8_t *pPixels, int width, int height, int quality, const char *pOutPath) {
    tjhandle tjInstance = tj3Init(TJINIT_COMPRESS);
    tj3Set(tjInstance, TJPARAM_SUBSAMP, TJSAMP_420);
    tj3Set(tjInstance, TJPARAM_COLORSPACE, TJCS_YCbCr);
    tj3Set(tjInstance, TJPARAM_QUALITY, quality);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);

    size_t fileSize = 0;
    uint8_t *pFileBuf = nullptr;

    int ret = tj3Compress8(tjInstance, pPixels, width, 0, height, TJPF_RGBX, &pFileBuf, &fileSize);
    tj3Destroy(tjInstance);

    if (ret != ERR_OK) {
        free(pFileBuf);
        return ERR_COMPRESS;
    }

    FILE *pFile = fopen(pOutPath, "wb");
    fwrite(pFileBuf, fileSize, 1, pFile);
    fclose(pFile);
    free(pFileBuf);

    return ret;
}

TJXOP calcTransform(int orientation) {
    TJXOP tjxop;
    switch (orientation) {
        case 1:
            tjxop = TJXOP_NONE;
            break;
        case 2:
            tjxop = TJXOP_HFLIP;
            break;
        case 3:
            tjxop = TJXOP_ROT180;
            break;
        case 4:
            tjxop = TJXOP_VFLIP;
            break;
        case 5:
            tjxop = TJXOP_TRANSPOSE;
            break;
        case 6:
            tjxop = TJXOP_ROT90;
            break;
        case 7:
            tjxop = TJXOP_TRANSVERSE;
            break;
        case 8:
            tjxop = TJXOP_ROT270;
            break;
        default:
            tjxop = TJXOP_NONE;
            break;
    }
    return tjxop;
}

int transform(uint8_t **jpegBuffer, size_t *jpegSize, int orientation, bool freeOld) {
    TJXOP tjxop = calcTransform(orientation);
    if (tjxop == TJXOP_NONE) {
        return ERR_OK;
    }
    tjtransform xform;
    memset(&xform, 0, sizeof(tjtransform));
    xform.op = tjxop;
    xform.options |= TJXOPT_TRIM;
    uint8_t *dstBuf = nullptr;  /* Dynamically allocate the JPEG buffer */
    size_t dstSize = 0;
    tjhandle tjInstance = tj3Init(TJINIT_TRANSFORM);
    int ret = tj3Transform(tjInstance, *jpegBuffer, *jpegSize, 1, &dstBuf, &dstSize, &xform);
    tj3Destroy(tjInstance);
    if (ret != ERR_OK) {
        tj3Free(dstBuf);
        return ERR_TRANSFORM; // tj3Transform() => 变换出错
    }
    if (freeOld) free(*jpegBuffer);
    *jpegBuffer = dstBuf;
    *jpegSize = dstSize;
    return ret;
}

int decompressHeader(tjhandle tjInstance, uint8_t *jpegBuffer, size_t jpegSize, int *width,
                     int *height) {
    int ret = tj3DecompressHeader(tjInstance, jpegBuffer, jpegSize);
    if (ret != ERR_OK) return ERR_DECOMPRESS_HEADER;  // tj3DecompressHeader() => 获取 JPEG 信息出错

    int w = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
    int h = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);
    int size = w * h * 4;

    if (0 < size && size < INT32_MAX) {
        if (width != nullptr) *width = w;
        if (height != nullptr) *height = h;
        return ret;
    }
    return ERR_RESOLUTION;
}

int decompress(uint8_t **jpegBuffer, size_t jpegSize, int orientation, uint8_t **pixelData,
               int *width, int *height) {
    // 根据 orientation 转换 jpeg
    TJXOP tjxop = calcTransform(orientation);
    if (tjxop != TJXOP_NONE) transform(jpegBuffer, &jpegSize, orientation, true);
    tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);
    // decode header 获取 宽高信息
    int w = 0;
    int h = 0;
    int ret = decompressHeader(tjInstance, *jpegBuffer, jpegSize, &w, &h);
    if (ret != ERR_OK) {
        tj3Destroy(tjInstance);
        return ret;
    }
    //Log::d("%d*%d", *width, *height);
    auto *pixels = (uint8_t *) malloc(w * h * 4);

    ret = tj3Decompress8(tjInstance, *jpegBuffer, jpegSize, pixels, 0, TJPF_RGBX);
    //Log::d("ret=%d", ret);
    tj3Destroy(tjInstance);
    if (ret != ERR_OK) {
        free(pixels);
        return ERR_DECOMPRESS;
    }
    *width = w;
    *height = h;
    *pixelData = pixels;
    return ret;
}

int decompressJpegInfo(JNIEnv *env, jobject jpegInfo, uint8_t *jpegBuffer, size_t jpegSize,
                       int orientation) {
    // 根据 orientation 转换 jpeg
    TJXOP tjxop = calcTransform(orientation);
    if (tjxop != TJXOP_NONE) transform(&jpegBuffer, &jpegSize, orientation, false);
    tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);
    // decode header 获取 宽高信息
    int width = 0;
    int height = 0;
    int ret = decompressHeader(tjInstance, jpegBuffer, jpegSize, &width, &height);
    if (ret != ERR_OK) {
        tj3Destroy(tjInstance);
        return ret;
    }
    // 设置 jobject: jpegInfo #width #height #pixels
    jclass classJpegInfo = env->FindClass("com/github/tangjw/libjpeg/JpegInfo");
    env->SetIntField(jpegInfo, env->GetFieldID(classJpegInfo, "width", "I"), width);
    env->SetIntField(jpegInfo, env->GetFieldID(classJpegInfo, "height", "I"), height);
    jfieldID pixelsField = env->GetFieldID(classJpegInfo, "pixels", "[B");
    auto jPixels = (jbyteArray) env->GetObjectField(jpegInfo, pixelsField);
    int pixelsSize = width * height * 4;
    // 如果 pixels为null或者length不等于
    if (jPixels == nullptr || pixelsSize != env->GetArrayLength(jPixels)) {
        env->DeleteLocalRef(jPixels);   // 删除旧引用
        jPixels = env->NewByteArray(pixelsSize);    // 创建新的
    }
    jbyte *pPixels = env->GetByteArrayElements(jPixels, JNI_FALSE);
    // 解码 jpeg
    ret = tj3Decompress8(tjInstance, jpegBuffer, jpegSize, (uint8_t *) pPixels, 0, TJPF_RGBX);
    env->ReleaseByteArrayElements(jPixels, pPixels, 0);
    env->SetObjectField(jpegInfo, pixelsField, jPixels);
    tj3Destroy(tjInstance);
    if (ret != ERR_OK) return ERR_DECOMPRESS;
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegDecompressFile(JNIEnv *env, jclass clazz,
                                                             jstring file_path, jint file_size,
                                                             jint orientation,
                                                             jobject jpeg_info) {

    const char *path = env->GetStringUTFChars(file_path, JNI_FALSE);
    FILE *file = fopen(path, "rb");
    env->ReleaseStringUTFChars(file_path, path);

    size_t bufferSize = file_size;
    void *buffer = malloc(bufferSize);
    fread(buffer, bufferSize, 1, file);
    fclose(file);
    int ret = decompressJpegInfo(env, jpeg_info, (uint8_t *) buffer, bufferSize, orientation);
    free(buffer);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegDecompressDirectBuffer(JNIEnv *env, jclass clazz,
                                                                     jobject byte_buffer,
                                                                     jint offset,
                                                                     jint length,
                                                                     jint exif_orientation,
                                                                     jobject jpeg_info) {
    auto *buffer = (uint8_t *) env->GetDirectBufferAddress(byte_buffer);
    return decompressJpegInfo(env, jpeg_info, buffer + offset, length, exif_orientation);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegDecompressByteArray(JNIEnv *env, jclass clazz,
                                                                  jbyteArray byte_array,
                                                                  jint offset,
                                                                  jint length,
                                                                  jint exif_orientation,
                                                                  jobject jpeg_info) {
    jbyte *bytes = env->GetByteArrayElements(byte_array, JNI_FALSE);
    int ret = decompressJpegInfo(env, jpeg_info, (uint8_t *) bytes + offset, length,
                                 exif_orientation);
    env->ReleaseByteArrayElements(byte_array, bytes, 0);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegCompressFile(JNIEnv *env, jclass clazz,
                                                           jstring file_path,
                                                           jint file_size,
                                                           jint exif_orientation,
                                                           jint quality,
                                                           jstring out_path) {

    const char *pJpegPath = env->GetStringUTFChars(file_path, JNI_FALSE);
    FILE *pJpegFile = fopen(pJpegPath, "rb");
    env->ReleaseStringUTFChars(file_path, pJpegPath);

    size_t jpegSize = file_size;
    auto *pJpegBuffer =(uint8_t *) malloc(jpegSize);
    fread(pJpegBuffer, jpegSize, 1, pJpegFile);
    fclose(pJpegFile);

    tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);
    // decode header 获取 宽高信息
    int width = 0;
    int height = 0;
    uint8_t *pPixelsBuffer = nullptr;
    // int ret = decompressHeader(tjInstance,(uint8_t *) pJpegBuffer, jpegSize, &width, &height);
    int ret = decompress( &pJpegBuffer, jpegSize, exif_orientation, &pPixelsBuffer, &width, &height);
    free(pJpegBuffer);

    if (ret != ERR_OK) {
        free(pPixelsBuffer);
        return ERR_DECOMPRESS;
    }
    // 压缩
    const char *pOutPath = env->GetStringUTFChars(out_path, JNI_FALSE);
    ret = compressJpeg(pPixelsBuffer, width, height, quality, pOutPath);
    free(pPixelsBuffer);
    env->ReleaseStringUTFChars(out_path, pOutPath);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegCompressTexture(JNIEnv *env, jclass clazz,
                                                              jint textureId,
                                                              jint width,
                                                              jint height,
                                                              jint quality,
                                                              jstring out_path) {
    if (glGetError() != GL_NO_ERROR) return ERR_GL_ERROR;

    GLuint iFBO = 0;
    glGenFramebuffers(1, &iFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, iFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) return ERR_GL_ERROR;

    void *pPixels = malloc(width * height * 4);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);    // 绑定 0 渲染到主窗口 不需要直接删除
    glDeleteFramebuffers(1, &iFBO);     // 删除帧缓冲对象

    const char *pOutPath = env->GetStringUTFChars(out_path, JNI_FALSE);
    int ret = compressJpeg((uint8_t *) pPixels, width, height, quality, pOutPath);
    env->ReleaseStringUTFChars(out_path, pOutPath);
    free(pPixels);

    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegCompressBytes(JNIEnv *env, jclass clazz,
                                                            jbyteArray pixels_array, jint offset,
                                                            jint width, jint height,
                                                            jint quality, jstring out_path) {
    jbyte *pPixels = env->GetByteArrayElements(pixels_array, JNI_FALSE);
    const char *pOutPath = env->GetStringUTFChars(out_path, JNI_FALSE);
    int ret = compressJpeg((uint8_t *) pPixels + offset, width, height, quality, pOutPath);
    env->ReleaseStringUTFChars(out_path, pOutPath);
    env->ReleaseByteArrayElements(pixels_array, pPixels, 0);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegCompressDirectBuffer(JNIEnv *env, jclass clazz,
                                                                   jobject pixels_buffer,
                                                                   jint offset,
                                                                   jint width, jint height,
                                                                   jint quality, jstring out_path) {
    const char *pOutPath = env->GetStringUTFChars(out_path, JNI_FALSE);
    void *pPixels = env->GetDirectBufferAddress(pixels_buffer);
    int ret = compressJpeg((uint8_t *) pPixels + offset, width, height, quality, pOutPath);
    env->ReleaseStringUTFChars(out_path, pOutPath);
    return ret;
}