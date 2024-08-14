#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

Mat reduceNoise(Mat img) {
    Mat morphKernel = getStructuringElement(MORPH_RECT, Size(5, 5));
    cv::morphologyEx(img, img, cv::MORPH_CLOSE, morphKernel);
    cv::morphologyEx(img, img, cv::MORPH_OPEN, morphKernel);
    return img;
}
