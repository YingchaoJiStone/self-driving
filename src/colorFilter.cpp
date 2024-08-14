#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

cv::Mat colorFilter(cv::Mat img, cv::Mat originalImg, cv::Mat *imgBlue, cv::Mat *imgYellow, int minH_B, int maxH_B, int minH_Y, int maxH_Y,
                    int minS_B, int maxS_B, int minS_Y, int maxS_Y, int minV_B, int maxV_B, int minV_Y, int maxV_Y) {
    cv::Mat originalHSV, imgHSV;

    cvtColor(originalImg, originalHSV, cv::COLOR_BGR2HSV);
    cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

    cv::inRange(originalHSV, cv::Scalar(minH_Y, minS_Y, minV_Y), cv::Scalar(maxH_Y, maxS_Y, maxV_Y), *imgYellow);
    cv::inRange(imgHSV, cv::Scalar(minH_B, minS_B, minV_B), cv::Scalar(maxH_B, maxS_B, maxV_B), *imgBlue);

    cv::Mat yellowCones = originalImg.clone();
    cv::Mat blueCones = img.clone();

    originalImg.setTo(cv::Scalar(0, 255, 255), *imgYellow);
    img.setTo(cv::Scalar(255, 100, 100), *imgBlue);

    // Create an output image initialized to black (all zeros)
    cv::Mat outputImage = cv::Mat::zeros(img.size(), img.type());

    yellowCones.copyTo(outputImage, *imgYellow);
    blueCones.copyTo(outputImage, *imgBlue);

    return outputImage;
}
