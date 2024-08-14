#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

std::string getCurrentTime();

int drawRect(std::vector<std::vector<cv::Point>> contours, cv::Mat drawing, std::vector<std::vector<cv::Point>> contours_poly,
             std::vector<cv::Rect> boundRect, int centerLineY, cv::Mat imgBlue, cv::Mat imgYellow, std::vector<cv::Rect> *blueRects,
             std::vector<cv::Rect> *yellowRects, cv::Mat *img) {
    // Draw bounding rectangles around the contours
    size_t i = 0;
    for (i = 0; i < contours.size(); i++) {
        std::string currentTime = getCurrentTime();

        // Specify the position and size of the rectangle
        cv::Rect upHalf(0, 0, 640, 240);
        cv::Rect carBody(180, 380, 300, 100);

        cv::Scalar color = cv::Scalar(0, 255, 0);
        cv::drawContours(drawing, contours_poly, (int) i, color);
        cv::rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 1);

        cv::Rect boundingRect = cv::boundingRect(contours[i]);

        // Ignore the rectangles at top half and the car's body
        if (boundingRect.height > 10 && boundingRect.y > centerLineY && !upHalf.contains(boundingRect.tl()) &&
            !carBody.contains(boundingRect.tl())) {

            // The color is determined by how the pixels contained in boundingRect are
            // distributed in imgBlue and imgYellow
            cv::Mat roiBlue = imgBlue(boundingRect);
            cv::Mat roiYellow = imgYellow(boundingRect);

            // Count the total number of pixels in the blue section and the yellow
            // section
            int bluePixels = cv::countNonZero(roiBlue);
            int yellowPixels = cv::countNonZero(roiYellow);

            // Judge the color based on the number of pixels
            if (bluePixels > yellowPixels) {
                // Push boundingRect to the list
                blueRects->push_back(boundingRect);
            } else {
                // Push boundingRect to the list
                yellowRects->push_back(boundingRect);
            }
        }

        // Print rectangle position information
        cv::rectangle(*img, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 1);

        // Draw the fixed size rectangle on the image
        cv::rectangle(*img, upHalf, cv::Scalar(0, 0, 255), 3);
        cv::rectangle(*img, carBody, cv::Scalar(0, 0, 255), 3);
    }
    return blueRects->size() + yellowRects->size();
}

std::string getCurrentTime() {
    // Gets the current timestamp, expressed as a point in time on the system clock
    auto now = std::chrono::system_clock::now();

    // Convert a point in time to time t
    auto now_c = std::chrono::system_clock::to_time_t(now);

    // Gets the microsecond portion of the current time
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    // Convert the time portion to local time
    std::tm tm_now{};
    localtime_r(&now_c, &tm_now);

    // Creates a stream of strings to format the time
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(6) << std::setfill('0') << now_us.count();

    // Return a string time
    return oss.str();
}
