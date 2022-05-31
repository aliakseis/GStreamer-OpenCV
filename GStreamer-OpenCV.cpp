// GStreamer-OpenCV.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <opencv2/opencv.hpp>

#include <iostream>

int main()
{
    // The sink caps for the 'rtpjpegdepay' need to match the src caps of the 'rtpjpegpay' of the sender pipeline
    // Added 'videoconvert' at the end to convert the images into proper format for appsink, without
    // 'videoconvert' the receiver will not read the frames, even though 'videoconvert' is not present
    // in the original working pipeline
    //VideoCapture cap("udpsrc port=5000 ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,framerate=30/1 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink",
    //    CAP_GSTREAMER);

    // ksvideosrc ! video/x-raw,format=I420,width=640,height=480,framerate=30/1
    //VideoCapture cap("ksvideosrc ! video/x-raw,format=I420,width=640,height=480,framerate=30/1 ! videoconvert ! appsink",
    cv::VideoCapture cap("ksvideosrc ! video/x-raw ! videoconvert ! appsink",
        cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        std::cerr << "VideoCapture not opened" << std::endl;
        exit(-1);
    }

    while (true) {

        cv::Mat frame;

        cap.read(frame);

        cv::imshow("receiver", frame);

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    return 0;
}
