#include "calcSteeringAngle.cpp"
#include "cluon-complete.hpp"
#include "colorFilter.cpp"
#include "contrastAndBrightness.cpp"
#include "drawRect.cpp"
#include "reduceNoise.cpp"

#include "opendlv-standard-message-set.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>

void drawArrows(cv::Mat *img, double groundSteering);

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ((0 == commandlineArguments.count("cid")) || (0 == commandlineArguments.count("name")) ||
        (0 == commandlineArguments.count("width")) || (0 == commandlineArguments.count("height"))) {
        std::cerr << argv[0]
                  << " attaches to a shared memory area containing an ARGB image and transform it "
                     "to HSV color space for inspection."
                  << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --name=<name of shared memory area> --width=<W> --height=<H>" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
    } else {
        // Extract the values from the command line parameters
        const std::string NAME{commandlineArguments["name"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid()) {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)."
                      << std::endl;

            // Create an OpenCV image header using the data in the shared memory.
            IplImage *iplimage{nullptr};
            CvSize size;
            size.width = WIDTH;
            size.height = HEIGHT;

            iplimage = cvCreateImageHeader(size, IPL_DEPTH_8U, 4 /* four channels: ARGB */);
            sharedMemory->lock();
            {
                iplimage->imageData = sharedMemory->data();
                iplimage->imageDataOrigin = iplimage->imageData;
            }
            sharedMemory->unlock();

            int minH_B{105};
            int maxH_B{151};
            int minH_Y{13};
            int maxH_Y{32};

            int minS_B{80};
            int maxS_B{255};
            int minS_Y{84};
            int maxS_Y{255};

            int minV_B{41};
            int maxV_B{255};
            int minV_Y{39};
            int maxV_Y{255};

            // Interface to a running OpenDaVINCI session where network messages are exchanged.
            // The instance od4 allows you to send and receive messages.
            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

            opendlv::proxy::VoltageReading vr;
            std::mutex vrMutex;

            double leftIfrValue = 0;
            double rightIfrValue = 0;
            bool isRightSensor = true;

            auto onVoltageReading = [&vr, &vrMutex, &isRightSensor, &leftIfrValue, &rightIfrValue](cluon::data::Envelope &&env) {
                std::lock_guard<std::mutex> lck(vrMutex);
                vr = cluon::extractMessage<opendlv::proxy::VoltageReading>(std::move(env));
                if (isRightSensor) {
                    rightIfrValue = vr.voltage();
                } else {
                    leftIfrValue = vr.voltage();
                }
                isRightSensor = !isRightSensor;   // Toggle the flag
            };

            od4.dataTrigger(opendlv::proxy::VoltageReading::ID(), onVoltageReading);

            static int64_t lastTimestamp = 0;
            int64_t tsInt = 0;

            while (od4.isRunning()) {
                cv::Mat originalImg;
                cv::Mat img;

                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy image into cvMat structure.
                    // Be aware of that any code between lock/unlock is blocking
                    // the camera to provide the next frame. Thus, any
                    // computationally heavy algorithms should be placed outside
                    // lock/unlock.
                    originalImg = cv::cvarrToMat(iplimage);
                }

                // Get the sampleTimePoint when the current frame was captured
                auto tsPair = sharedMemory->getTimeStamp();
                tsInt = cluon::time::toMicroseconds(tsPair.second);

                sharedMemory->unlock();

                if (tsInt > lastTimestamp) {
                    lastTimestamp = tsInt;

                    // -------- FUNCTION 1: Contrast and Brightness Adjustment --------
                    img = adjustContrastAndBrightness(originalImg);
                    // ----------------------------------------------------------------

                    // -------- FUNCTION 2: Color Filtering --------
                    // Declare an image to store values for blue, and an image to store values for
                    // yellow
                    cv::Mat imgBlue, imgYellow;

                    cv::Mat outputImage = colorFilter(img, originalImg, &imgBlue, &imgYellow, minH_B, maxH_B, minH_Y, maxH_Y, minS_B,
                                                      maxS_B, minS_Y, maxS_Y, minV_B, maxV_B, minV_Y, maxV_Y);

                    //  ----------------------------------------------------------------

                    // Function 3: Apply morphological operations (erosion and dilation) to reduce
                    // noise
                    outputImage = reduceNoise(outputImage);
                    // ----------------------------------------------------------------

                    int videoWidth = img.cols;
                    int videoHeight = img.rows;

                    int centerLineX = videoWidth / 2;
                    int centerLineY = videoHeight / 2;

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
                        boundRect[i] = cv::boundingRect(contours_poly[i]);
                    }

                    // ----------------------------------------------------------------

                    cv::Mat drawing = cv::Mat::zeros(canny_output.size(), CV_8UC3);

                    // Create a blue list and yellow list to store the boundingRects
                    std::vector<cv::Rect> blueRects;
                    std::vector<cv::Rect> yellowRects;

                    // Function 5: Draw bounding rectangles around the contours and count the
                    // important ones

                    int rect_n = drawRect(contours, drawing, contours_poly, boundRect, centerLineY, imgBlue, imgYellow, &blueRects,
                                          &yellowRects, &img);

                    // ----------------------------------------------------------------

                    // Function 6: Calculate the steering angle

                    double groundSteering = calcSteeringAngle(leftIfrValue, rightIfrValue, blueRects, yellowRects, centerLineX, &img);

                    /* ---------------------=---- PRINT ------------------------- */

                    std::cout << "group_19;" << tsInt << ";" << groundSteering << std::endl;

                    /* -------------------------- DRAW -------------------------- */

                    if (VERBOSE) {
                        std::cout << "Number of cones: " << rect_n << std::endl;
                        std::cout << " Ground Steering Request: " << groundSteering << std::endl;

                        drawArrows(&img, groundSteering);

                        cv::imshow("Color-Space Image", drawing);   // Display the combined mask
                        cv::imshow(sharedMemory->name().c_str(),
                                   originalImg);   // Display the original image
                        cv::imshow("Color Filtered Image",
                                   outputImage);   // Display the output image with highlighted colors
                        cv::waitKey(1);
                    }
                }
            }

            if (nullptr != iplimage) {
                cvReleaseImageHeader(&iplimage);
            }
        }
        retCode = 0;
    }
    return retCode;
}

void drawArrows(cv::Mat *img, double groundSteering) {
    // Draw the steering angle arrow on the video
    if (groundSteering > 0) {
        cv::arrowedLine(*img, cv::Point(260, 290), cv::Point(260 - groundSteering * 500, 290), cv::Scalar(0, 0, 255), 10);
    } else {
        cv::arrowedLine(*img, cv::Point(380, 290), cv::Point(380 - groundSteering * 500, 290), cv::Scalar(0, 0, 255), 10);
    }

    // Print the groundSteering to video
    cv::putText(*img, std::to_string(groundSteering), cv::Point(240, 360), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
}
