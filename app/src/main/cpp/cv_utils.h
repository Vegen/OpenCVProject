//
// Created by vegen on 2019/10/27.
//

#ifndef OPENCVPROJECT_CV_UTILS_H
#define OPENCVPROJECT_CV_UTILS_H

#include <jni.h>
#include "opencv2/opencv.hpp"
#include <android/bitmap.h>

class cv_utils {
public:
    static int bitmap2Mat(JNIEnv *env, cv::Mat &dst, jobject &bitmap);

    static int mat2Bitmap(JNIEnv *env, cv::Mat &src, jobject &bitmap);

    static jobject createBitmap(JNIEnv *env, jint width, jint height, int type);
};


#endif //OPENCVPROJECT_CV_UTILS_H
