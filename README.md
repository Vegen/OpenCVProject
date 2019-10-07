#### 前言

 `OpenCV` 提供的视觉处理算法非常丰富，对图像、视频处理提供比较方便的处理方法，本文介绍使用 `OpenCV` 对图像进行处理，本文例子基于 `Android Studio 3.4.1`，`OpenCV 3.4.6`，`gradle-5.1.1`，`build:gradle:3.4.1`。若下载 Demo 编译不成功请升级 AS 或 将相关配置修改，项目源码在文末链接下载。

 #### 1. 转灰度图

 主要使用 `cvtColor` 方法进行转换，亦可拿到图片的像素进行自行转换

 ```
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
 ```

![转灰度图](https://user-gold-cdn.xitu.io/2019/10/7/16da51bbaae6b0d1?w=1080&h=2340&f=png&s=2389934)


 #### 2. 底片效果

 主要是拿到像素值用 255 减之

 ```
 JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_negative(JNIEnv *env, jobject instance, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    Mat gary;
    cvtColor(src, gary, COLOR_BGR2GRAY);

    // 读者可以注释下面实现查看效果
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
 ```


![底片效果](https://user-gold-cdn.xitu.io/2019/10/7/16da51c82c0da0a7?w=1080&h=2340&f=png&s=3441272)

 #### 3. 图层叠加

使用 `addWeighted` 方法，必须两张图的大小一样

addWeighted(InputArray src1, double alpha, InputArray src2, double beta, double gamma, OutputArray dst, int dtype=-1);
* 第一个参数，InputArray类型的src1，表示需要加权的第一个数组，常常填一个Mat。
* 第二个参数，alpha，表示第一个数组的权重
* 第三个参数，src2，表示第二个数组，它需要和第一个数组拥有相同的尺寸和通道数。
* 第四个参数，beta，表示第二个数组的权重值。
* 第五个参数，dst，输出的数组，它和输入的两个数组拥有相同的尺寸和通道数。
* 第六个参数，gamma，一个加到权重总和上的标量值。
* 第七个参数，dtype，输出阵列的可选深度，有默认值-1。;当两个输入数组具有相同的深度时，这个参数设置为-1（默认值），即等同于src1.depth（）

 ```
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
 ```


![图层叠加](https://user-gold-cdn.xitu.io/2019/10/7/16da51cdabc8b8bb?w=1080&h=2340&f=png&s=3453398)

 #### 4. 饱和度、对比度、亮度调节

alpha：饱和度，对比度；beta：亮度

- F(R) = alpha*R + beta;
- F(G) = alpha*G + beta;
- F(B) = alpha*B + beta;

 ```
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

 ```


![色值（饱和度、亮度）调节](https://user-gold-cdn.xitu.io/2019/10/7/16da51d6005c9e80?w=1080&h=2340&f=png&s=3145299)

 #### 5. 绘制形状和文字

 注意：Scalar 四个参数分别对应 B G R A

 涉及方法
 - 画线：`line`
 - 矩形：`rectangle`
 - 椭圆：`ellipse`
 - 多边形：`fillPoly`
 - 圆：`circle`
 - 文字：`putText`

 ```

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
 ```


![绘制形状和文字](https://user-gold-cdn.xitu.io/2019/10/7/16da51dc128eceb9?w=1080&h=2340&f=png&s=3383954)

 #### 6. 三种滤波模糊

 本文介绍三种滤波：均值滤波，中值滤波，高斯滤波，对于其原理，鉴于篇幅，本文不做详细介绍，读者可自行去了解

##### 均值滤波

blur( InputArray src, OutputArray dst,
                        Size ksize, Point anchor = Point(-1,-1),
                        int borderType = BORDER_DEFAULT );

* 第一个参数，InputArray类型的src，输入图像，即源图像，填Mat类的对象即可。该函数对通道是独立处理的，且可以处理任意通道数的图片，但需要注意，待处理的图片深度应该为CV_8U, CV_16U, CV_16S, CV_32F 以及 CV_64F之一。
* 第二个参数，OutputArray类型的dst，即目标图像，需要和源图片有一样的尺寸和类型。比如可以用Mat::Clone，以源图片为模板，来初始化得到如假包换的目标图。
* 第三个参数，Size类型的 ksize，内核的大小。一般这样写Size( w,h )来表示内核的大小( 其中，w 为像素宽度， h为像素高度)。Size（3,3）就表示3x3的核大小，Size（5,5）就表示5x5的核大小
* 第四个参数，Point类型的anchor，表示锚点（即被平滑的那个点），注意他有默认值Point(-1,-1)。如果这个点坐标是负值的话，就表示取核的中心为锚点，所以默认值Point(-1,-1)表示这个锚点在核的中心。
* 第五个参数，int类型的borderType，用于推断图像外部像素的某种边界模式。有默认值BORDER_DEFAULT，我们一般不去管它。

##### 中值滤波

medianBlur( InputArray src, OutputArray dst, int ksize );

* InputArray src: 输入图像，图像为1、3、4通道的图像，当模板尺寸为3或5时，图像深度只能为CV_8U、CV_16U、CV_32F中的一个，如而对于较大孔径尺寸的图片，图像深度只能是CV_8U。
* OutputArray dst: 输出图像，尺寸和类型与输入图像一致，可以使用Mat::Clone以原图像为模板来初始化输出图像dst
* int ksize: 滤波模板的尺寸大小，必须是大于1的奇数，如3、5、7……

##### 高斯滤波

GaussianBlur( InputArray src, OutputArray dst, Size ksize,
                                double sigmaX, double sigmaY = 0,
                                int borderType = BORDER_DEFAULT );

* InputArray src: 输入图像，可以是Mat类型，图像深度为CV_8U、CV_16U、CV_16S、CV_32F、CV_64F。
* OutputArray dst: 输出图像，与输入图像有相同的类型和尺寸。
* Size ksize: 高斯内核大小，这个尺寸与前面两个滤波kernel尺寸不同，ksize.width和ksize.height可以不相同但是这两个值必须为正奇数，如果这两个值为0，他们的值将由sigma计算。
* double sigmaX: 高斯核函数在X方向上的标准偏差
* double sigmaY: 高斯核函数在Y方向上的标准偏差，如果sigmaY是0，则函数会自动将sigmaY的值设置为与sigmaX相同的值，如果sigmaX和sigmaY都是0，这两个值将由ksize.width和ksize.height计算而来。具体可以参考getGaussianKernel()函数查看具体细节。建议将size、sigmaX和sigmaY都指定出来。
* int borderType = BORDER_DEFAULT: 推断图像外部像素的某种便捷模式，有默认值BORDER_DEFAULT，如果没有特殊需要不用更改，具体可以参考borderInterpolate()函数。


 ```
 JNIEXPORT jint JNICALL
Java_com_vegen_opencvproject_ResultUtil_blur(JNIEnv *env, jobject instance, jobject bitmap) {

    Mat src;
    bitmap2Mat(env, src, bitmap);

    // 每横向 1/3 演示一种处理效果，请仔细观察

    /** 均值滤波 **/
    Size size = Size(29, 29);
    Mat mat1 = src(Rect(0, 0, src.cols / 3, src.rows));
    /*
     blur( InputArray src, OutputArray dst,
                        Size ksize, Point anchor = Point(-1,-1),
                        int borderType = BORDER_DEFAULT );
     第一个参数，InputArray类型的src，输入图像，即源图像，填Mat类的对象即可。该函数对通道是独立处理的，且可以处理任意通道数的图片，但需要注意，待处理的图片深度应该为CV_8U, CV_16U, CV_16S, CV_32F 以及 CV_64F之一。
     第二个参数，OutputArray类型的dst，即目标图像，需要和源图片有一样的尺寸和类型。比如可以用Mat::Clone，以源图片为模板，来初始化得到如假包换的目标图。
     第三个参数，Size类型的 ksize，内核的大小。一般这样写Size( w,h )来表示内核的大小( 其中，w 为像素宽度， h为像素高度)。Size（3,3）就表示3x3的核大小，Size（5,5）就表示5x5的核大小
     第四个参数，Point类型的anchor，表示锚点（即被平滑的那个点），注意他有默认值Point(-1,-1)。如果这个点坐标是负值的话，就表示取核的中心为锚点，所以默认值Point(-1,-1)表示这个锚点在核的中心。
     第五个参数，int类型的borderType，用于推断图像外部像素的某种边界模式。有默认值BORDER_DEFAULT，我们一般不去管它。
     */
    blur(mat1, mat1, size);

    line(src, Point(src.cols / 3, 0), Point(src.cols / 3, src.rows), Scalar(255, 255, 255, 255), 3, LINE_8);

    /** 中值模糊 **/
    Mat mat2 = src(Rect(src.cols / 3, 0, src.cols / 3, src.rows));
    medianBlur(mat2, mat2, 31);

    line(src, Point(2 * src.cols / 3, 0), Point(2 * src.cols / 3, src.rows), Scalar(255, 255, 255, 255), 3, LINE_8);

    /** 高斯模糊 **/
    Mat mat3 = src(Rect(2 * src.cols / 3, 0, src.cols / 3, src.rows));
    /*
     GaussianBlur( InputArray src, OutputArray dst, Size ksize,
                                double sigmaX, double sigmaY = 0,
                                int borderType = BORDER_DEFAULT );
     InputArray src: 输入图像，可以是Mat类型，图像深度为CV_8U、CV_16U、CV_16S、CV_32F、CV_64F。
     OutputArray dst: 输出图像，与输入图像有相同的类型和尺寸。
     Size ksize: 高斯内核大小，这个尺寸与前面两个滤波kernel尺寸不同，ksize.width和ksize.height可以不相同但是这两个值必须为正奇数，如果这两个值为0，他们的值将由sigma计算。
     double sigmaX: 高斯核函数在X方向上的标准偏差
     double sigmaY: 高斯核函数在Y方向上的标准偏差，如果sigmaY是0，则函数会自动将sigmaY的值设置为与sigmaX相同的值，如果sigmaX和sigmaY都是0，这两个值将由ksize.width和ksize.height计算而来。具体可以参考getGaussianKernel()函数查看具体细节。建议将size、sigmaX和sigmaY都指定出来。
     int borderType=BORDER_DEFAULT: 推断图像外部像素的某种便捷模式，有默认值BORDER_DEFAULT，如果没有特殊需要不用更改，具体可以参考borderInterpolate()函数。
     */
    GaussianBlur(mat3, mat3, Size(41, 41), 0, 0);

    mat2Bitmap(env, src, bitmap);
    return 0;
}

 ```


![滤波模糊](https://user-gold-cdn.xitu.io/2019/10/7/16da51e2a3326da3?w=1080&h=2340&f=png&s=3410892)

 #### 后话

 OpenCV 的图片处理功能还有很多，本文介绍仅常用的一些处理效果实现，文中 Demo 源码下载地址：[https://github.com/Vegen/OpenCVProject](https://github.com/Vegen/OpenCVProject)，欢迎 star。