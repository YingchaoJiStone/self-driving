#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

cv::Mat adjustContrastAndBrightness(cv::Mat img) {
    double alpha, beta;   // declare contrast control and brightness control

    // Adjust contrast and brightness dynamically
    double aveBrightness = cv::mean(img)[0];
    if (aveBrightness < 120) {
        alpha = 1.5;   // Increase sdsdsfsfsfsf for darker images
        beta = 50;     // Increase brightness
    } else if (aveBrightness > 180) {
        alpha = 0.8;   // Decrease contrast for brighter images
        beta = -30;    // Decrease brightness
    } else {
        alpha = 1.0;
        beta = 0;
    }
    img.convertTo(img, -1, alpha, beta);
    return img;
}
