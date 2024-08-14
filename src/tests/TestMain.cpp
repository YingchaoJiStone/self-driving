#define CATCH_CONFIG_MAIN
#include "../testMe.cpp"

#include "catch.hpp"

#include <experimental/filesystem>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../calcSteeringAngle.cpp"
#include "../colorFilter.cpp"
#include "../contrastAndBrightness.cpp"
#include "../drawRect.cpp"
#include "../reduceNoise.cpp"

namespace fs = std::experimental::filesystem;

const int minH_B{105};
const int maxH_B{151};
const int minH_Y{13};
const int maxH_Y{32};
const int minS_B{80};
const int maxS_B{255};
const int minS_Y{84};
const int maxS_Y{255};
const int minV_B{41};
const int maxV_B{255};
const int minV_Y{39};
const int maxV_Y{255};

// Test contrast and brightness adjustment

TEST_CASE("Contrast and brightness adjustment test") {
    cv::Mat originalImg = cv::imread("../src/tests/testImages/test_1.png", cv::IMREAD_COLOR);
    cv::Mat img;
    if (originalImg.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        FAIL();
    }

    img = adjustContrastAndBrightness(originalImg);

    // Save the image to the output folder
    cv::imwrite("../src/tests/edgeCaseTestImages/contrast_brightness_test_1_output.png", img);

    // Check if the image is saved
    cv::Mat savedImg = cv::imread("../src/tests/edgeCaseTestImages/contrast_brightness_test_1_output.png", cv::IMREAD_COLOR);

    REQUIRE(savedImg.rows == img.rows);
    REQUIRE(savedImg.cols == img.cols);
}

// Test color filtering
TEST_CASE("Color filtering test") {
    cv::Mat originalImg = cv::imread("../src/tests/testImages/test_1.png", cv::IMREAD_COLOR);
    cv::Mat img;
    if (originalImg.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        FAIL();
    }

    img = adjustContrastAndBrightness(originalImg);

    cv::Mat imgBlue;
    cv::Mat imgYellow;
    img = colorFilter(img, originalImg, &imgBlue, &imgYellow, minH_B, maxH_B, minH_Y, maxH_Y, minS_B, maxS_B, minS_Y, maxS_Y, minV_B,
                      maxV_B, minV_Y, maxV_Y);

    // Save the image to the output folder
    cv::imwrite("../src/tests/edgeCaseTestImages/color_filter_test_1_output.png", img);

    // Check if the image is saved
    cv::Mat savedImg = cv::imread("../src/tests/edgeCaseTestImages/color_filter_test_1_output.png", cv::IMREAD_COLOR);

    REQUIRE(savedImg.rows == img.rows);
    REQUIRE(savedImg.cols == img.cols);
}

// Test noise reduction
TEST_CASE("Noise reduction test") {
    cv::Mat originalImg = cv::imread("../src/tests/testImages/test_1.png", cv::IMREAD_COLOR);
    cv::Mat img;
    if (originalImg.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        FAIL();
    }

    img = adjustContrastAndBrightness(originalImg);

    cv::Mat imgBlue;
    cv::Mat imgYellow;
    img = colorFilter(img, originalImg, &imgBlue, &imgYellow, minH_B, maxH_B, minH_Y, maxH_Y, minS_B, maxS_B, minS_Y, maxS_Y, minV_B,
                      maxV_B, minV_Y, maxV_Y);

    img = reduceNoise(img);

    // Save the image to the output folder
    cv::imwrite("../src/tests/edgeCaseTestImages/noise_reduction_test_1_output.png", img);

    // Check if the image is saved
    cv::Mat savedImg = cv::imread("../src/tests/edgeCaseTestImages/noise_reduction_test_1_output.png", cv::IMREAD_COLOR);

    REQUIRE(savedImg.rows == img.rows);
    REQUIRE(savedImg.cols == img.cols);
}

// Test contour detection
TEST_CASE("Contour detection test") {
    cv::Mat originalImg = cv::imread("../src/tests/testImages/test_1.png", cv::IMREAD_COLOR);
    cv::Mat img;
    if (originalImg.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        FAIL();
    }

    img = adjustContrastAndBrightness(originalImg);

    cv::Mat imgBlue;
    cv::Mat imgYellow;
    img = colorFilter(img, originalImg, &imgBlue, &imgYellow, minH_B, maxH_B, minH_Y, maxH_Y, minS_B, maxS_B, minS_Y, maxS_Y, minV_B,
                      maxV_B, minV_Y, maxV_Y);

    img = reduceNoise(img);

    cv::Mat canny_output;
    cv::Canny(img, canny_output, 100, 200);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(canny_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> contours_poly(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());

    for (size_t i = 0; i < contours.size(); i++) {
        cv::approxPolyDP(contours[i], contours_poly[i], 0.02 * cv::arcLength(contours[i], true), true);
        boundRect[i] = cv::boundingRect(contours_poly[i]);
    }

    cv::Mat drawing = cv::Mat::zeros(canny_output.size(), CV_8UC3);

    // Create a blue list and yellow list to store the boundingRects
    std::vector<cv::Rect> blueRects;
    std::vector<cv::Rect> yellowRects;

    int videoWidth = img.cols;
    int videoHeight = img.rows;
    int centerLineX = videoWidth / 2;
    int centerLineY = videoHeight / 2;

    int rect_n = drawRect(contours, drawing, contours_poly, boundRect, centerLineY, imgBlue, imgYellow, &blueRects, &yellowRects, &img);

    // Save the image to the output folder
    cv::imwrite("../src/tests/edgeCaseTestImages/contour_detection_test_1_output.png", drawing);

    // Check if the image is saved
    cv::Mat savedImg = cv::imread("../src/tests/edgeCaseTestImages/contour_detection_test_1_output.png", cv::IMREAD_COLOR);

    REQUIRE(savedImg.rows == drawing.rows);
    REQUIRE(savedImg.cols == drawing.cols);
}

// Test cone detection accuracy
TEST_CASE("Cone detection accuracy tests") {
    std::string folderPath = "../src/tests/testImages";

    // loop for each image
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".png") {
            // extract image and load to image object
            std::string path = entry.path().filename().string();
            cv::Mat originalImg = cv::imread(folderPath + "/" + path, cv::IMREAD_COLOR);
            cv::Mat img;
            if (originalImg.empty()) {
                std::cerr << "Error: Image not found" << std::endl;
                FAIL();
            }

            // read img metadata
            std::string img_data;
            std::string img_filename = path.substr(0, path.find("."));
            std::ifstream img_file(folderPath + "/" + img_filename + ".txt");

            if (img_file.is_open()) {
                std::getline(img_file, img_data);
                img_file.close();
            } else {
                std::cerr << "Error: Image metadata not found" << std::endl;
                FAIL();
            }

            // extract metadata values from the format "cones=xx"
            int cones = std::stoi(img_data.substr(img_data.find("=") + 1));

            img = adjustContrastAndBrightness(originalImg);

            cv::Mat imgBlue;
            cv::Mat imgYellow;
            img = colorFilter(img, originalImg, &imgBlue, &imgYellow, minH_B, maxH_B, minH_Y, maxH_Y, minS_B, maxS_B, minS_Y, maxS_Y,
                              minV_B, maxV_B, minV_Y, maxV_Y);

            img = reduceNoise(img);

            cv::Mat canny_output;
            cv::Canny(img, canny_output, 100, 200);

            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(canny_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            std::vector<std::vector<cv::Point>> contours_poly(contours.size());
            std::vector<cv::Rect> boundRect(contours.size());

            for (size_t i = 0; i < contours.size(); i++) {
                cv::approxPolyDP(contours[i], contours_poly[i], 0.02 * cv::arcLength(contours[i], true), true);
                boundRect[i] = cv::boundingRect(contours_poly[i]);
            }

            cv::Mat drawing = cv::Mat::zeros(canny_output.size(), CV_8UC3);

            // Create a blue list and yellow list to store the boundingRects
            std::vector<cv::Rect> blueRects;
            std::vector<cv::Rect> yellowRects;

            int videoHeight = img.rows;
            int centerLineY = videoHeight / 2;

            // Function 5: Draw bounding rectangles around the contours and count the important ones
            int rect_n =
                drawRect(contours, drawing, contours_poly, boundRect, centerLineY, imgBlue, imgYellow, &blueRects, &yellowRects, &img);

            std::cout << "Number of cones: " << rect_n << std::endl;

            REQUIRE(rect_n >= cones);
        }
    }
}

// Edge case test: there are no cones in the image

TEST_CASE("No cones in the image shouldn't fail or detect any cones") {
    cv::Mat originalImg = cv::imread("../src/tests/edgeCaseTestImages/no_cones_test.png", cv::IMREAD_COLOR);
    cv::Mat img;
    if (originalImg.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        FAIL();
    }

    // -------- FUNCTION 1: Contrast and Brightness Adjustment --------
    img = adjustContrastAndBrightness(originalImg);
    // ----------------------------------------------------------------

    // -------- FUNCTION 2: Color Filtering --------
    // Declare an image to store values for blue, and an image to store values for
    // yellow
    cv::Mat imgBlue, imgYellow;

    cv::Mat outputImage = colorFilter(img, originalImg, &imgBlue, &imgYellow, minH_B, maxH_B, minH_Y, maxH_Y, minS_B, maxS_B, minS_Y,
                                      maxS_Y, minV_B, maxV_B, minV_Y, maxV_Y);

    // Function 3: Apply morphological operations (erosion and dilation) to reduce
    // noise
    outputImage = reduceNoise(outputImage);
    // ----------------------------------------------------------------

    // Function 4: Detect contours
    cv::Mat canny_output;
    cv::Canny(outputImage, canny_output, 100, 200);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(canny_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> contours_poly(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());

    for (size_t i = 0; i < contours.size(); i++) {
        cv::approxPolyDP(contours[i], contours_poly[i], 0.02 * cv::arcLength(contours[i], true), true);
        // if (cv::contourArea(contours_poly[i]) > 100 && cv::contourArea(contours_poly[i]) < 720) {
        boundRect[i] = cv::boundingRect(contours_poly[i]);
        //}
    }

    // ----------------------------------------------------------------

    cv::Mat drawing = cv::Mat::zeros(canny_output.size(), CV_8UC3);

    // Create a blue list and yellow list to store the boundingRects
    std::vector<cv::Rect> blueRects;
    std::vector<cv::Rect> yellowRects;

    int videoWidth = img.cols;
    int videoHeight = img.rows;
    int centerLineX = videoWidth / 2;
    int centerLineY = videoHeight / 2;

    // Function 5: Draw bounding rectangles around the contours and count the
    // important ones

    int rect_n = drawRect(contours, drawing, contours_poly, boundRect, centerLineY, imgBlue, imgYellow, &blueRects, &yellowRects, &img);

    std::cout << "Number of cones: " << rect_n << std::endl;

    REQUIRE(rect_n == 0);

    // ----------------------------------------------------------------

    // Function 6: Calculate the steering angle
    double groundSteering = 0;
    bool blueOnLeft = false;

    double diffRatio = 0;

    double blueDistanceRatio = 20.0;
    double yellowDistanceRatio = 20.0;

    // Pass zero for gsr.groundSteering() to test the case where it is not provided
    calcSteeringAngle(blueRects, yellowRects, centerLineX, &img, &diffRatio, 0, &groundSteering, &blueDistanceRatio, &yellowDistanceRatio,
                      &blueOnLeft);
    // ----------------------------------------------------------------

    // Print mapped diffRatio to console
    std::cout << "DiffRatio: " << diffRatio << ", Ground Steering Request: " << groundSteering << std::endl;

    // Print distance ratio
    std::cout << "Blue distance ratio: " << blueDistanceRatio << std::endl;
    std::cout << "Yellow distance ratio: " << yellowDistanceRatio << std::endl;

    REQUIRE(groundSteering == 0);
}

