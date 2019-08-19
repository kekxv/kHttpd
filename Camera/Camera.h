//
// Created by caesar on 2019-08-18.
//

#ifndef KHTTP_CAMERA_H
#define KHTTP_CAMERA_H

#include <string>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>

class Camera {
public:
    explicit Camera(int index = 0);
    explicit Camera(const std::string& videoSrc);
    ~Camera();
    bool start();
    bool isOpened();
    bool stop();
    cv::Mat GetImage();
    static long long GetTime();
private:
    cv::VideoCapture cap;
    cv::Mat LastMat;
    int VideoIndex = 0;
    std::string VideoSrc;
    bool IsExit = false;
    bool IsStart = false;

    std::mutex CameraMutex;
    std::thread CameraThread;
    void InitThread();
    static void Run(Camera* camera);
};


#endif //KHTTP_CAMERA_H
