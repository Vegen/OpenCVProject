#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;
using namespace std;

extern "C" {
JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_toGray(JNIEnv *env, jobject instance, jobject bitmap);

JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_negative(JNIEnv *env, jobject instance, jobject bitmap);

// bitmap 转成 Mat
void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap);
// mat 转成 Bitmap
void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap);
}

// 转灰度图
JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_toGray(JNIEnv *env, jobject instance, jobject bitmap) {
    // --------- 第一种方法：使用 api 转灰度图 ---------
    /*
    Mat mat;
    bitmap2Mat(env, mat, bitmap);
    Mat gray_mat;
    cvtColor(mat, gray_mat, COLOR_BGRA2GRAY);
    mat2Bitmap(env, gray_mat, bitmap);
    */

    // --------- 第二种方法：原理层面转灰度图 ---------
    AndroidBitmapInfo bitmapInfo;
    int info_res = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    if (info_res != 0) {
        return info_res;
    }

    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    // 判断颜色通道
    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        for (int i = 0; i < bitmapInfo.width * bitmapInfo.height; ++i) {
            uint32_t *pixel_p = reinterpret_cast<uint32_t *>(pixels) + i;
            uint32_t pixel = *pixel_p;
            int a = (pixel >> 24) & 0xff;
            int r = (pixel >> 16) & 0xff;
            int g = (pixel >> 8) & 0xff;
            int b = pixel & 0xff;
            // f = 0.213f * r + 0.715f * g + 0.072f * b
            int gery = (int) (0.213f * r + 0.715f * g + 0.072f * b);
            *pixel_p = (a << 24) | (gery << 16) | (gery << 8) | gery;
        }
    } else if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        for (int i = 0; i < bitmapInfo.width * bitmapInfo.height; ++i) {
            uint16_t *pixel_p = reinterpret_cast<uint16_t *>(pixels) + i;
            uint16_t pixel = *pixel_p;
            // 8888 -> 565
            int r = ((pixel >> 11) & 0x1f) << 3; // 5
            int g = ((pixel >> 5) & 0x3f) << 2; // 6
            int b = (pixel & 0x1f) << 3; // 5
            // f = 0.213f * r + 0.715f * g + 0.072f * b
            int gery = (int) (0.213f * r + 0.715f * g + 0.072f * b); // 8位

            *pixel_p = ((gery >> 3) << 11) | ((gery >> 2) << 5) | (gery >> 3);
        }
    }
    // 其他通道暂不介绍

    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_negative(JNIEnv *env, jobject instance, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    Mat gary;
    cvtColor(src, gary, COLOR_BGR2GRAY);

    Mat testMat = src.clone();    // 4 通道
    //Mat testMat = gary.clone();     // 1 通道
    // 获取信息
    int cols = testMat.cols;// 宽
    int rows = testMat.rows;// 高
    int channels = testMat.channels();// 1
    LOGE("cols:%d  rows:%d  channels:%d", cols, rows, channels);

    // Bitmap 里面转的是 4 通道 ， 一个通道就可以代表灰度
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (channels == 3){
                // 获取像素 at  Vec3b 个参数
                int b = testMat.at<Vec3b>(i, j)[0];
                int g = testMat.at<Vec3b>(i, j)[1];
                int r = testMat.at<Vec3b>(i, j)[2];

                // 修改像素 (底片效果)
                testMat.at<Vec3b>(i, j)[0] = 255 - b;
                testMat.at<Vec3b>(i, j)[1] = 255 - g;
                testMat.at<Vec3b>(i, j)[2] = 255 - r;
            } else if (channels == 4) {
                // 获取像素 at  Vec4b 个参数
                int b = testMat.at<Vec4b>(i, j)[0];
                int g = testMat.at<Vec4b>(i, j)[1];
                int r = testMat.at<Vec4b>(i, j)[2];
                int a = testMat.at<Vec4b>(i, j)[3];
                // 修改像素 (底片效果)
                testMat.at<Vec4b>(i, j)[0] = 255 - b;
                testMat.at<Vec4b>(i, j)[1] = 255 - g;
                testMat.at<Vec4b>(i, j)[2] = 255 - r;
            } else if (channels == 1){
                uchar pixels = testMat.at<uchar>(i, j);
                testMat.at<uchar>(i, j) = 255 - pixels;
            }
        }
    }

    mat2Bitmap(env, testMat, bitmap);
    return 0;
}

JNIEXPORT void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap) {
    // Mat 里面有个 type ： CV_8UC4 刚好对上我们的 Bitmap 中 ARGB_8888 , CV_8UC2 刚好对象我们的 Bitmap 中 RGB_565
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    // 指定 mat 的宽高和type  BGRA
    mat.create(info.height, info.width, CV_8UC4);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        // 对应的 mat 应该是  CV_8UC4
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        // 把数据 temp 复制到 mat 里面
        temp.copyTo(mat);
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        // 对应的 mat 应该是  CV_8UC2
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        // mat 是 CV_8UC4 ，CV_8UC2 -> CV_8UC4
        cvtColor(temp, mat, COLOR_BGR5652BGRA);
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap) {
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {// C4
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        if (mat.type() == CV_8UC4) {
            mat.copyTo(temp);
        } else if (mat.type() == CV_8UC2) {
            cvtColor(mat, temp, COLOR_BGR5652BGRA);
        } else if (mat.type() == CV_8UC1) {// 灰度 mat
            cvtColor(mat, temp, COLOR_GRAY2BGRA);
        }
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {// C2
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        if (mat.type() == CV_8UC4) {
            cvtColor(mat, temp, COLOR_BGRA2BGR565);
        } else if (mat.type() == CV_8UC2) {
            mat.copyTo(temp);

        } else if (mat.type() == CV_8UC1) {// 灰度 mat
            cvtColor(mat, temp, COLOR_GRAY2BGR565);
        }
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env, bitmap);
}
