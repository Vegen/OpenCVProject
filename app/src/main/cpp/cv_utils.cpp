//
// Created by vegen on 2019/10/27.
//

#include "cv_utils.h"


#include <android/bitmap.h>
#include "cv_utils.h"

int cv_utils::bitmap2Mat(JNIEnv *env, cv::Mat &dst, jobject &bitmap) {
    AndroidBitmapInfo bitmapInfo;
    int getInfoRes = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    if (getInfoRes < 0) {
        return getInfoRes;
    }

    void *pixels;
    int lockPixelsRes = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lockPixelsRes < 0) {
        return lockPixelsRes;
    }

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        //  ANDROID_BITMAP_FORMAT_RGBA_8888 -> CV_8UC4
        dst.create(bitmapInfo.height, bitmapInfo.width, CV_8UC4);
        dst.data = reinterpret_cast<uchar *>(pixels);
    } else if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        dst.create(bitmapInfo.height, bitmapInfo.width, CV_8UC2);
        dst.data = reinterpret_cast<uchar *>(pixels);
    } else {
        return -1;
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}

int cv_utils::mat2Bitmap(JNIEnv *env, cv::Mat &src, jobject &bitmap) {
    AndroidBitmapInfo bitmapInfo;
    int getInfoRes = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    if (getInfoRes < 0) {
        return getInfoRes;
    }

    void *pixels;
    int lockPixelsRes = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lockPixelsRes < 0) {
        return lockPixelsRes;
    }

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat temp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, pixels);
        if (src.type() == CV_8UC1) {
            cvtColor(src, temp, cv::COLOR_GRAY2RGBA);
        } else if (src.type() == CV_8UC3) {
            cvtColor(src, temp, cv::COLOR_RGB2RGBA);
        } else if (src.type() == CV_8UC4) {
            src.copyTo(temp);
        }
    } else if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        cv::Mat temp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, pixels);
        if (src.type() == CV_8UC1) {
            cvtColor(src, temp, cv::COLOR_GRAY2BGR565);
        } else if (src.type() == CV_8UC3) {
            cvtColor(src, temp, cv::COLOR_RGB2BGR565);
        } else if (src.type() == CV_8UC4) {
            cvtColor(src, temp, cv::COLOR_RGBA2BGR565);
        }
    } else {
        return -1;
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    return 0;
}

jobject cv_utils::createBitmap(JNIEnv *env, jint width, jint height, int type) {
    // Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);

    // Config correlation
    const char *bitmap_config_class_name = "android/graphics/Bitmap$Config";
    jclass bitmap_config_class = env->FindClass(bitmap_config_class_name);
    jmethodID bitmap_config_mid = env->GetStaticMethodID(bitmap_config_class, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jobject bitmap_config = env->CallStaticObjectMethod(bitmap_config_class, bitmap_config_mid, configName);

    // bitmap correlation
    const char *bitmap_class_name = "android/graphics/Bitmap";
    jclass bitmap_class = env->FindClass(bitmap_class_name);
    const char *create_bitmap_sig = "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;";
    jmethodID create_bitmap_mid = env->GetStaticMethodID(bitmap_class, "createBitmap", create_bitmap_sig);
    jobject bitmap = env->CallStaticObjectMethod(bitmap_class, create_bitmap_mid, width, height, bitmap_config);

    return bitmap;
}