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

JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_layerOverlay(JNIEnv *env, jobject instance, jobject bitmap,
                                                     jobject layerDrawable);

JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_chromaChange(JNIEnv *env, jobject instance, jobject bitmap);

JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_sketchpad(JNIEnv *env, jobject instance, jobject bitmap);

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

// 底片效果
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
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (channels == 3) {
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
            } else if (channels == 1) {
                uchar pixels = testMat.at<uchar>(i, j);
                testMat.at<uchar>(i, j) = 255 - pixels;
            }
        }
    }

    mat2Bitmap(env, testMat, bitmap);
    return 0;
}

// 图层叠加
JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_layerOverlay(JNIEnv *env, jobject instance, jobject bitmap,
                                                     jobject layerDrawable) {
    Mat img;
    bitmap2Mat(env, img, bitmap);

    Mat logo;
    bitmap2Mat(env, logo, layerDrawable);

    Mat imgROI1 = img(Rect(0, 0, logo.cols, logo.rows));
    Mat imgROI2 = img(Rect(img.cols - logo.cols, img.rows - logo.rows, logo.cols, logo.rows));
    /**
     * addWeighted 方法必须两张图的大小一样
     * addWeighted(InputArray src1, double alpha, InputArray src2, double beta, double gamma, OutputArray dst, int dtype=-1);
     * 第一个参数，InputArray类型的src1，表示需要加权的第一个数组，常常填一个Mat。
     * 第二个参数，alpha，表示第一个数组的权重
     * 第三个参数，src2，表示第二个数组，它需要和第一个数组拥有相同的尺寸和通道数。
     * 第四个参数，beta，表示第二个数组的权重值。
     * 第五个参数，dst，输出的数组，它和输入的两个数组拥有相同的尺寸和通道数。
     * 第六个参数，gamma，一个加到权重总和上的标量值。
     * 第七个参数，dtype，输出阵列的可选深度，有默认值-1。;当两个输入数组具有相同的深度时，这个参数设置为-1（默认值），即等同于src1.depth（）
     */
    addWeighted(imgROI1, 1, logo, 1, 0.0, imgROI1);
    addWeighted(imgROI2, 1, logo, 1, 0.0, imgROI2);

    mat2Bitmap(env, img, bitmap);
    return 0;
}

// 饱和度、对比度、亮度调节
JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_chromaChange(JNIEnv *env, jobject instance, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    int cols = src.cols;// 宽
    int rows = src.rows;// 高
    int channels = src.channels();// 通道

    LOGE("chromaChange-->channels=%d", channels);   // 4

    // alpha 饱和度 , 对比度
    // beta 亮度
    // F(R) = alpha*R + beta;
    // F(G) = alpha*G + beta;
    // F(B) = alpha*B + beta;

    float alpha = 1.2f;
    float beta = 20;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {

            if (channels == 3) {
                // 获取像素 at  Vec3b 个参数
                int b = src.at<Vec3b>(i, j)[0];
                int g = src.at<Vec3b>(i, j)[1];
                int r = src.at<Vec3b>(i, j)[2];

                src.at<Vec3b>(i, j)[0] = saturate_cast<uchar>(b * alpha + beta);
                src.at<Vec3b>(i, j)[1] = saturate_cast<uchar>(g * alpha + beta);
                src.at<Vec3b>(i, j)[2] = saturate_cast<uchar>(r * alpha + beta);
            } else if (channels == 4) {
                // 获取像素 at  Vec4b 个参数
                int b = src.at<Vec4b>(i, j)[0];
                int g = src.at<Vec4b>(i, j)[1];
                int r = src.at<Vec4b>(i, j)[2];
                int a = src.at<Vec4b>(i, j)[3];

                src.at<Vec4b>(i, j)[0] = saturate_cast<uchar>(b * alpha + beta);
                src.at<Vec4b>(i, j)[1] = saturate_cast<uchar>(g * alpha + beta);
                src.at<Vec4b>(i, j)[2] = saturate_cast<uchar>(r * alpha + beta);
                src.at<Vec4b>(i, j)[3] = 255;
            } else if (channels == 1) {
                uchar pixels = src.at<uchar>(i, j);
                src.at<uchar>(i, j) = saturate_cast<uchar>(pixels * alpha + beta);
            }
        }
    }

    mat2Bitmap(env, src, bitmap);
    return 0;
}

// 绘制形状和文字
JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_sketchpad(JNIEnv *env, jobject instance, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    // 注意：Scalar 四个参数分别对应 B G R A

    // 线 line
    line(src, Point(0, 0), Point(500, 500), Scalar(0, 0, 255, 255), 20, LINE_8);

    // 矩形 rectangle
    rectangle(src, Point(500, 500), Point(1000, 1000), Scalar(255, 0, 0, 255), 20, LINE_8);

    // 椭圆 ellipse
    // 第二个参数是： 椭圆的中心点
    // 第三个参数是： Size 第一个值是椭圆 x width 的半径 ，第二个 ...
    ellipse(src, Point(src.cols / 2, src.rows / 2), Size(src.cols / 8, src.rows / 4), 360, 0, 360,
            Scalar(0, 255, 255, 255), 20);

    // 三角形
    Point pts[1][4];
    pts[0][0] = Point(500, 500);
    pts[0][1] = Point(500, 1000);
    pts[0][2] = Point(1000, 1000);
    pts[0][3] = Point(500, 500);

    const Point *ptss[] = {pts[0]};
    const int npts[] = {4};
    /*
     * 填充 fillPoly 多边形
     Mat& img, const Point** pts,
                         const int* npts, int ncontours,
                         const Scalar& color, int lineType = LINE_8, int shift = 0,
                         Point offset = Point()
     */

    fillPoly(src, ptss, npts, 1, Scalar(255, 0, 0), 20);

    // 圆 circle
    circle(src, Point(src.cols / 2, src.rows / 2), src.rows / 4, Scalar(255, 255, 0, 255), 20, LINE_AA);

    // 文字
    const String text = "Hello World";
    int fontFace = CV_FONT_BLACK;   // 字体
    double fontScale = 6;           // 字体缩放比
    int thickness = 2;              // 画笔厚度
    int baseline = 0;               // 基线
    // 获取文字宽度
    /*
     const String& text, int fontFace,
                            double fontScale, int thickness,
                            CV_OUT int* baseLine
     */
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
    // 文字 putText
    /*
     InputOutputArray img, const String& text, Point org,
                         int fontFace, double fontScale, Scalar color,
                         int thickness = 1, int lineType = LINE_8,
                         bool bottomLeftOrigin = false
     */
    putText(src, text, Point(src.cols / 2 - textSize.width / 2, 200), fontFace, fontScale, Scalar(255, 255, 255, 255),
            thickness, LINE_AA);

    // 随机画 srand 画线
    // opencv 做随机 srand random 效果一样
    RNG rng(time(NULL));

    // 随机生成十条线
    for (int i = 0; i < 10; i++) {
        Point sp;
        sp.x = rng.uniform(0, src.cols);
        sp.y = rng.uniform(0, src.rows);
        Point ep;
        ep.x = rng.uniform(0, src.cols);
        ep.y = rng.uniform(0, src.rows);
        line(src, sp, ep, Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), 255), 4);
    }

    mat2Bitmap(env, src, bitmap);
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
