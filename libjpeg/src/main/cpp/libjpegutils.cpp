#include <jni.h>
#include <string>

#include <malloc.h>
#include <stdio.h>
#include <setjmp.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "base/Log.h"
#include "libjpeg/turbojpeg.h"

/*
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_github_tangjw_libjpeg_LibJpegUtils_tBytesToRgb(JNIEnv *env, jclass clazz,
                                                         jbyteArray bytes, jint start, jint length,
                                                         jbyteArray jPixels) {

    tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);
    jbyte *pFileBuf = env->GetByteArrayElements(bytes, 0) + start;
    int ret = tj3DecompressHeader(tjInstance, (uint8_t *) pFileBuf, length);
    Log::d("tj3DecompressHeader fail: %d", ret);

    int width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
    int height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);

    long pixelSize = width * height * 4 + 8;

    if (pixelSize > INT32_MAX)
        Log::d("allocating uncompressed image buffer Image is too large %d", pixelSize);
    if (jPixels == nullptr || pixelSize != env->GetArrayLength(jPixels)) {
        env->DeleteLocalRef(jPixels);
        jPixels = env->NewByteArray(pixelSize);
    }
    jbyte *pPixelsBuf = env->GetByteArrayElements(jPixels, 0);
    if (tj3Decompress8(tjInstance, (uint8_t *) pFileBuf, length,
                       (uint8_t *) pPixelsBuf, 0, TJPF_RGBX) < 0)
        Log::d("decompressing JPEG image");
    env->ReleaseByteArrayElements(jPixels, pPixelsBuf, 0);
    tj3Destroy(tjInstance);
    tjInstance = nullptr;

    env->SetByteArrayRegion(jPixels, pixelSize - 8, 4, (jbyte *) &width);
    env->SetByteArrayRegion(jPixels, pixelSize - 4, 4, (jbyte *) &height);

    return jPixels;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_github_tangjw_libjpeg_LibJpegUtils_tJpegToRgb(JNIEnv *env, jclass clazz,

                                                        jstring jInPath, jbyteArray jPixels) {
    const char *pInPath = env->GetStringUTFChars(jInPath, 0);
    FILE *pFile = fopen(pInPath, "rb");
    env->ReleaseStringUTFChars(jInPath, pInPath);

    long fileSize = 0;
    if (fseek(pFile, 0, SEEK_END) < 0
        || ((fileSize = ftell(pFile)) < 0)
        || fseek(pFile, 0, SEEK_SET) < 0
        || fileSize == 0)
        Log::d("文件无法访问或者文件没有数据");

    uint8_t *pFileBuf = (uint8_t *) tj3Alloc(fileSize);
    fread(pFileBuf, fileSize, 1, pFile);
    fclose(pFile);
    tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);

    tj3DecompressHeader(tjInstance, pFileBuf, fileSize);
    int width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
    int height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);

    long pixelSize = width * height * 4 + 8;

    if (pixelSize > INT32_MAX)
        Log::d("allocating uncompressed image buffer Image is too large %d", pixelSize);
    if (jPixels == nullptr || pixelSize != env->GetArrayLength(jPixels)) {
        env->DeleteLocalRef(jPixels);
        jPixels = env->NewByteArray(pixelSize);
    }

    jbyte *pPixelsBuf = env->GetByteArrayElements(jPixels, 0);
    if (tj3Decompress8(tjInstance, pFileBuf, fileSize,
                       (uint8_t *) pPixelsBuf, 0, TJPF_RGBX) < 0)
        Log::d("decompressing JPEG image");
    free(pFileBuf);
    env->ReleaseByteArrayElements(jPixels, pPixelsBuf, 0);
    tj3Destroy(tjInstance);

    env->SetByteArrayRegion(jPixels, pixelSize - 8, 4, (jbyte *) &width);
    env->SetByteArrayRegion(jPixels, pixelSize - 4, 4, (jbyte *) &height);

    return jPixels;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_LibJpegUtils_tRgbToJpeg(JNIEnv *env, jclass clazz,
                                                        jstring jOutPath, jbyteArray jPixels,
                                                        jint jWidth, jint jHeight) {

    tjhandle tjInstance = tj3Init(TJINIT_COMPRESS);

    if (tj3Set(tjInstance, TJPARAM_SUBSAMP, TJSAMP_420) < 0) Log::d("setting TJPARAM_SUBSAMP");
    if (tj3Set(tjInstance, TJPARAM_COLORSPACE, TJCS_YCbCr) < 0)
        Log::d("setting TJPARAM_COLORSPACE");
    if (tj3Set(tjInstance, TJPARAM_QUALITY, 95) < 0) Log::d("setting TJPARAM_QUALITY");
    if (tj3Set(tjInstance, TJPARAM_FASTDCT, 1) < 0) Log::d("setting TJPARAM_FASTDCT");

    size_t fileSize = 0;
    uint8_t *pFileBuf = nullptr;

    jbyte *pPixelsBuf = env->GetByteArrayElements(jPixels, 0);
    if (tj3Compress8(tjInstance, (uint8_t *) pPixelsBuf, jWidth, 0, jHeight,
                     TJPF_RGBX, &pFileBuf, &fileSize) < 0)
        Log::d("compressing image");
    env->ReleaseByteArrayElements(jPixels, pPixelsBuf, 0);
    tj3Destroy(tjInstance);
    tjInstance = nullptr;

    const char *pOutPath = env->GetStringUTFChars(jOutPath, 0);
    FILE *pFile = fopen(pOutPath, "wb");
    env->ReleaseStringUTFChars(jOutPath, pOutPath);
    if (fwrite(pFileBuf, fileSize, 1, pFile) < 1) Log::d("writing output file");
    free(pFileBuf);
    pFileBuf = nullptr;
    fclose(pFile);

    return 0;
}*/

void throwExc(JNIEnv *env, const char *msg) {
    jclass cls = env->FindClass("java/io/IOException");
    if (cls != NULL) {
        env->ThrowNew(cls, msg);
    }
    env->DeleteLocalRef(cls);
}

jobject decompressJpeg(JNIEnv *env, jobject jpegInfo, uint8_t *pDatas, size_t dataSize) {
    tjhandle tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);

    int ret = tj3DecompressHeader(tjInstance, pDatas, dataSize);
    if (ret != 0) {
        tj3Destroy(tjInstance);
        throwExc(env, "tj3DecompressHeader fail!");
        return jpegInfo;
    }

    jclass classJpegInfo = env->FindClass("com/github/tangjw/libjpeg/JpegInfo");
    jmethodID jpegInfoInit = env->GetMethodID(classJpegInfo, "<init>", "()V");
    if (jpegInfo == nullptr) {
        jpegInfo = env->NewObject(classJpegInfo, jpegInfoInit);
    }

    int width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
    env->SetIntField(jpegInfo, env->GetFieldID(classJpegInfo, "width", "I"), width);
    int height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);
    env->SetIntField(jpegInfo, env->GetFieldID(classJpegInfo, "height", "I"), height);

    long pixelSize = width * height * 4;
    if (pixelSize > INT32_MAX) {
        tj3Destroy(tjInstance);
        throwExc(env, "Image is too large!");
        return jpegInfo;
    }
    jfieldID pixelsField = env->GetFieldID(classJpegInfo, "pixels", "[B");
    jbyteArray jPixels = (jbyteArray) env->GetObjectField(jpegInfo, pixelsField);

    if (jPixels == nullptr || pixelSize != env->GetArrayLength(jPixels)) {
        env->DeleteLocalRef(jPixels);
        jPixels = env->NewByteArray(pixelSize);
    }

    jbyte *pPixels = env->GetByteArrayElements(jPixels, 0);
    ret = tj3Decompress8(tjInstance, (uint8_t *) pDatas, dataSize,
                         (uint8_t *) pPixels, 0, TJPF_RGBX);
    tj3Destroy(tjInstance);
    env->SetObjectField(jpegInfo, pixelsField, jPixels);
    env->ReleaseByteArrayElements(jPixels, pPixels, 0);

    if (ret != 0) {
        throwExc(env, "tj3Decompress8 fail ");
    }

    return jpegInfo;
}

jobject decompressJpegRota(JNIEnv *env, jobject jpegInfo, uint8_t *pDatas, size_t dataSize,
                           TJXOP tjxop) {
    tjhandle tjInstance;
    int ret;
    if (tjxop != TJXOP_NONE) {
        tjtransform xform;
        memset(&xform, 0, sizeof(tjtransform));
        xform.op = tjxop;
        xform.options |= TJXOPT_TRIM;
        unsigned char *dstBuf = nullptr;  /* Dynamically allocate the JPEG buffer */
        size_t dstSize = 0;
        tjInstance = tj3Init(TJINIT_TRANSFORM);
        ret = tj3Transform(tjInstance, pDatas, dataSize, 1,
                           &dstBuf, &dstSize, &xform);
        if (ret < 0) {
            tj3Free(dstBuf);
            tj3Destroy(tjInstance);
            throwExc(env, "tj3Transform fail!");
            return jpegInfo;
        }
        pDatas = dstBuf;
        dataSize = dstSize;
        tj3Destroy(tjInstance);
    }

    tjInstance = tj3Init(TJINIT_DECOMPRESS);
    tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);

    ret = tj3DecompressHeader(tjInstance, pDatas, dataSize);

    if (ret < 0) {
        tj3Destroy(tjInstance);
        throwExc(env, "tj3DecompressHeader fail!");
        return jpegInfo;
    }

    jclass classJpegInfo = env->FindClass("com/github/tangjw/libjpeg/JpegInfo");
    jmethodID jpegInfoInit = env->GetMethodID(classJpegInfo, "<init>", "()V");
    if (jpegInfo == nullptr) {
        jpegInfo = env->NewObject(classJpegInfo, jpegInfoInit);
    }

    int width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
    env->SetIntField(jpegInfo, env->GetFieldID(classJpegInfo, "width", "I"), width);
    int height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);
    env->SetIntField(jpegInfo, env->GetFieldID(classJpegInfo, "height", "I"), height);

    long pixelSize = width * height * 4;
    if (pixelSize > INT32_MAX) {
        tj3Destroy(tjInstance);
        throwExc(env, "Image is too large!");
        return jpegInfo;
    }
    jfieldID pixelsField = env->GetFieldID(classJpegInfo, "pixels", "[B");
    jbyteArray jPixels = (jbyteArray) env->GetObjectField(jpegInfo, pixelsField);

    if (jPixels == nullptr || pixelSize != env->GetArrayLength(jPixels)) {
        env->DeleteLocalRef(jPixels);
        jPixels = env->NewByteArray(pixelSize);
    }

    jbyte *pPixels = env->GetByteArrayElements(jPixels, 0);
    ret = tj3Decompress8(tjInstance, (uint8_t *) pDatas, dataSize,
                         (uint8_t *) pPixels, 0, TJPF_RGBX);
    if (tjxop != TJXOP_NONE) {
        tj3Free(pDatas);
    }

    tj3Destroy(tjInstance);
    env->SetObjectField(jpegInfo, pixelsField, jPixels);
    env->ReleaseByteArrayElements(jPixels, pPixels, 0);
    if (ret < 0) {
        throwExc(env, "tj3Decompress8 fail ");
    }
    return jpegInfo;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegDecodeFile(JNIEnv *env, jclass clazz,
                                                         jobject jpeg_info,
                                                         jstring jpeg_path, jint orientation) {

    const char *pJepgPath = env->GetStringUTFChars(jpeg_path, 0);
    FILE *pJpegFile = fopen(pJepgPath, "rb");
    env->ReleaseStringUTFChars(jpeg_path, pJepgPath);

    if (pJpegFile == nullptr) {
        throwExc(env, "文件不存在或者无法访问");
        return jpeg_info;
    }

    size_t dataSize;
    if (fseek(pJpegFile, 0, SEEK_END) < 0
        || ((dataSize = ftell(pJpegFile)) < 0)
        || fseek(pJpegFile, 0, SEEK_SET) < 0
        || dataSize == 0) {
        throwExc(env, "文件没有数据");
        return jpeg_info;
    }

    void *pDatas = malloc(dataSize);
    fread(pDatas, dataSize, 1, pJpegFile);
    fclose(pJpegFile);

//    jobject jpegInfo = decompressJpeg(env, jpeg_info, (uint8_t *) pDatas, dataSize);
//    free(pDatas);
    TJXOP tjxop;
    if (orientation == 90) {
        tjxop = TJXOP_ROT90;
    } else if (orientation == 180) {
        tjxop = TJXOP_ROT180;
    } else if (orientation == 270) {
        tjxop = TJXOP_ROT270;
    } else {
        tjxop = TJXOP_NONE;
//        tjxop = TJXOP_HFLIP;
    }
    jobject jpegInfo = decompressJpegRota(env, jpeg_info, (uint8_t *) pDatas,
                                          dataSize, tjxop);
    free(pDatas);
    return jpegInfo;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegDecodeBuffer(JNIEnv *env, jclass clazz,
                                                           jobject jpeg_info,
                                                           jobject jpeg_buffer,
                                                           jint start, jint length, jint orientation) {
    void *pDatas = env->GetDirectBufferAddress(jpeg_buffer);
    if (pDatas == nullptr) {
        throwExc(env, "DirectBuffer不存在或者无法访问");
        return jpeg_info;
    }
    TJXOP tjxop;
    if (orientation == 90) {
        tjxop = TJXOP_ROT90;
    } else if (orientation == 180) {
        tjxop = TJXOP_ROT180;
    } else if (orientation == 270) {
        tjxop = TJXOP_ROT270;
    } else {
        tjxop = TJXOP_NONE;
    }
    return decompressJpegRota(env, jpeg_info, (uint8_t *) pDatas + start, length,tjxop);
}


int compressJpeg(uint8_t *pPixels, int width, int height, int quality,
                 const char *pOutPath) {
    Log::d("width %d height %d", width, height);
    Log::d("quality %d", quality);
    tjhandle tjInstance = tj3Init(TJINIT_COMPRESS);
    tj3Set(tjInstance, TJPARAM_SUBSAMP, TJSAMP_420);
    tj3Set(tjInstance, TJPARAM_COLORSPACE, TJCS_YCbCr);
    tj3Set(tjInstance, TJPARAM_QUALITY, quality);
    tj3Set(tjInstance, TJPARAM_FASTDCT, 1);

    size_t fileSize = 0;
    uint8_t *pFileBuf = nullptr;

    int ret = tj3Compress8(tjInstance, pPixels, width, 0, height,
                           TJPF_RGBX, &pFileBuf, &fileSize);
    tj3Destroy(tjInstance);

    if (ret != 0) {
        free(pFileBuf);
        return -11;
    }

    FILE *pFile = fopen(pOutPath, "wb");
    if (pFile == nullptr) {
        free(pFileBuf);
        return -12;
    }

    fwrite(pFileBuf, fileSize, 1, pFile);
    fclose(pFile);
    free(pFileBuf);

    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegCompressFile(JNIEnv *env, jclass clazz,
                                                           jobject jpeg_info,
                                                           jstring jpeg_path) {
    if (jpeg_info == nullptr) return -1;

    jclass classJpegInfo = env->FindClass("com/github/tangjw/JpegInfo");
    jbyteArray jPixels =
            (jbyteArray) env->GetObjectField(jpeg_info,
                                             env->GetFieldID(classJpegInfo, "pixels", "[B"));
    if (jPixels == nullptr) return -2;

    jint width = env->GetIntField(jpeg_info, env->GetFieldID(classJpegInfo, "width", "I"));
    jint height = env->GetIntField(jpeg_info, env->GetFieldID(classJpegInfo, "height", "I"));

    size_t pixelSize = width * height * 4;
    Log::d("%d %d %d", width, height, pixelSize);
    if (pixelSize > env->GetArrayLength(jPixels)) return -3;

    jbyte *pPixels = env->GetByteArrayElements(jPixels, 0);
    const char *pOutPath = env->GetStringUTFChars(jpeg_path, 0);
    int ret = compressJpeg((uint8_t *) pPixels, width, height, 95, pOutPath);
    env->ReleaseStringUTFChars(jpeg_path, pOutPath);
    env->ReleaseByteArrayElements(jPixels, pPixels, 0);

    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_github_tangjw_libjpeg_JpegUtils_tJpegCompressTexture(JNIEnv *env, jclass clazz,
                                                              jint texture, jstring jpeg_path) {
    GLuint iFBO = 0;
    glGenFramebuffers(1, &iFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, iFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) return -1;

    GLint vp[4] = {0};
    glGetIntegerv(GL_VIEWPORT, vp);
    int width = vp[2], height = vp[3];
    Log::d("%d,%d,%d,%d", vp[0], vp[1], vp[2], vp[3]);

    uint8_t *pPixels = (uint8_t *) malloc(width * height * 4);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);
    // glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);    // 绑定 0 渲染到主窗口 不需要直接删除
    glDeleteFramebuffers(1, &iFBO);     // 删除帧缓冲对象

    const char *pOutPath = env->GetStringUTFChars(jpeg_path, 0);
    int ret = compressJpeg((uint8_t *) pPixels, width, height, 95, pOutPath);
    env->ReleaseStringUTFChars(jpeg_path, pOutPath);
    free(pPixels);

    return ret;
}
