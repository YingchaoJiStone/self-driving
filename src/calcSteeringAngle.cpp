#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define MAX_CONE_LOOK_AHEAD    3       // Maximum number of cones to consider per side
#define MAX_STEERING_AMPLITUDE 0.226   // Cut off value for steering angle
#define MIN_CONE_DISTANCE_Y    3       // Minimum distance between two cones to be considered as separate cones
#define CALM_ON_STRAIGHTS      0.7     // Factor to reduce steering angle when both blue and yellow cones are detected
#define SENSOR_STEERING_WEIGHT 0.85    // Weight of sensor data in steering angle calculation

// distance between two cones. default value only used if poor vision for first few frames
double totalDistanceRatio = 40;
// stores last steering value to be used if no cones are detected next frame
double diffRatio = 0;
double buffer[3] = {0, 0, 0};   // buffer to store last 3 steering values
int bufferI = 0;

bool blueOnLeft = true;

/* ------------------ FUNCTION DECLARATIONS ------------------ */

double calcMeanRatio(int centerLineX, std::vector<cv::Rect> rects, int isRight);
double height(cv::Rect rect);
double map(double value, double in_min, double in_max, double out_min, double out_max);
void determineTrackDirection(std::vector<cv::Rect> blueRects, std::vector<cv::Rect> yellowRects);

/* ------------------ FUNCTION DEFINITIONS ------------------ */

double calcSteeringAngle(double leftIfrValue, double rightIfrValue, std::vector<cv::Rect> blueRects, std::vector<cv::Rect> yellowRects,
                         int centerLineX, cv::Mat *img) {

    // Sort bounSdingrects from the highest height to lowest in blueRects
    std::partial_sort(blueRects.begin(), blueRects.end(), blueRects.end(),
                      [](const cv::Rect &a, const cv::Rect &b) { return height(a) > height(b); });

    // Sort bounSdingrects from the highest height to lowest in yellowRects
    std::partial_sort(yellowRects.begin(), yellowRects.end(), yellowRects.end(),
                      [](const cv::Rect &a, const cv::Rect &b) { return height(a) > height(b); });

    determineTrackDirection(blueRects, yellowRects);

    double blueDistanceRatio = calcMeanRatio(centerLineX, blueRects, blueOnLeft ? 1 : -1);
    double yellowDistanceRatio = calcMeanRatio(centerLineX, yellowRects, blueOnLeft ? -1 : 1);
    double sensorSteeringWeight = 0.65;   // used when only one side is detected

    if (blueRects.size() == 0) {   // sees 1 side
        blueDistanceRatio = totalDistanceRatio - yellowDistanceRatio;
    } else if (yellowRects.size() == 0) {   // sees 1 side
        yellowDistanceRatio = totalDistanceRatio - blueDistanceRatio;
    } else {   // sees 2 sides
        totalDistanceRatio = blueDistanceRatio + yellowDistanceRatio;
    }

    if (blueRects.size() != 0 || yellowRects.size() != 0) {   // sees 1 or 2 sides
        if (blueOnLeft)
            diffRatio = blueDistanceRatio - yellowDistanceRatio;
        else
            diffRatio = yellowDistanceRatio - blueDistanceRatio;
    }

    /* ---------------- CALCULATE GROUND STEERING --------------- */

    double steeringFactor = diffRatio / totalDistanceRatio;
    double visualSteering = steeringFactor * 0.3;

    // calculate the sensor data
    double sensorSteering;
    if (visualSteering > 0) {
        sensorSteering = std::fmod(map(leftIfrValue, 0, 0.03, 0, 0.3), 0.3);
    } else {
        sensorSteering = std::fmod(map(rightIfrValue, 0, 0.03, 0, -0.3), 0.3);
    }

    if (blueRects.size() != 0 && yellowRects.size() != 0) {
        visualSteering *= CALM_ON_STRAIGHTS;
    }

    buffer[bufferI] = visualSteering;
    visualSteering = (buffer[0] + buffer[1] + buffer[2]) / 3;
    bufferI = (bufferI + 1) % 3;
    double groundSteering = sensorSteering * SENSOR_STEERING_WEIGHT + visualSteering * (1 - SENSOR_STEERING_WEIGHT);

    // calculate the sensor data
    double sensorSteering;
    if (visualSteering > 0) {
        sensorSteering = std::fmod(map(leftIfrValue, 0, 0.03, 0, 0.3), 0.3);
    } else {
        sensorSteering = std::fmod(map(rightIfrValue, 0, 0.03, 0, -0.3), 0.3);
    }

    if (blueRects.size() != 0 && yellowRects.size() != 0) {
        visualSteering *= CALM_ON_STRAIGHTS;
    }

    sensorSteeringWeight = 0.85;

    buffer[bufferI] = visualSteering;
    visualSteering = (buffer[0] + buffer[1] + buffer[2]) / 3;
    bufferI = (bufferI + 1) % 3;
    double groundSteering = sensorSteering * sensorSteeringWeight + visualSteering * (1 - sensorSteeringWeight);

    // Limit the steering angle to MAX_STEERING_AMPLITUDE
    if (groundSteering > MAX_STEERING_AMPLITUDE)
        return MAX_STEERING_AMPLITUDE;
    else if (groundSteering < -MAX_STEERING_AMPLITUDE)
        return -MAX_STEERING_AMPLITUDE;
    else
        return groundSteering;
}

void determineTrackDirection(std::vector<cv::Rect> blueRects, std::vector<cv::Rect> yellowRects) {
    // Determine if the blue cone is on the left. Use strict scoping to prevent misidentification
    if (blueRects.size() > 0) {
        if (blueRects[0].x + blueRects[0].width / 2 > 620) {   // If there is a large blue cone appearing on the far right of the frame
            blueOnLeft = false;                                // Set blueOnLeft is false
        } else if (blueRects[0].x + blueRects[0].width / 2 < 20) {   // If there is a large blue cone appearing on the far left of the frame
            blueOnLeft = true;                                       // Set blueOnLeft is true
        }
    }

    if (yellowRects.size() > 0) {
        if (yellowRects[0].x + yellowRects[0].width / 2 < 20) {   // If there is a large yellow cone appearing on the far left of the frame
            blueOnLeft = false;                                   // Set blueOnLeft is false
        } else if (yellowRects[0].x + yellowRects[0].width / 2 >
                   620) {        // If there is a large yellow cone appearing on the far right of the frame
            blueOnLeft = true;   // Set blueOnLeft is true
        }
    }
    // If the position of the cone does not meet the above conditions, blueOnleft retains the previous setting
}

double calcMeanRatio(int centerLineX, std::vector<cv::Rect> rects, int isRight) {
    int foundCones = 0;
    double meanRatio = 0;

    // loop through cones until MAX_CONE_LOOK_AHEAD cones are found
    for (int i = 0; i < rects.size() && foundCones < MAX_CONE_LOOK_AHEAD; i++) {
        if (i > 0) {
            // loops until finds cones far enought apart (less likely to detect same cone twice)
            while (height(rects[i - 1]) - height(rects[i]) < MIN_CONE_DISTANCE_Y) {
                i++;

                if (i >= rects.size())
                    return meanRatio /= foundCones;
            }
        }

        const double ratio = static_cast<double>(centerLineX - rects[i].x) * isRight / height(rects[i]);
        const int y = rects[i].y;

        // source: y.nd
        meanRatio += ratio * (-0.159236 + 0.00000502275 * y * y);   // weighted average
        foundCones++;
    }

    if (foundCones == 0)
        return 0;   // prevents division by zero
    else
        return meanRatio /= foundCones;
}

double height(cv::Rect rect) {
    const double y = rect.y + rect.height;   // measure from bottom, more accurate

    // magic numbers from manual math-ing. Supposed to approximate cone height based on y value
    // should be more accurate as the cone height is unreliable. This is a rough estimate. see height.nb for info
    return y * 0.445546 - 112.032;
}

double map(double value, double in_min, double in_max, double out_min, double out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
