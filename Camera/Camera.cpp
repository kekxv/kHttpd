//
// Created by caesar on 2019-08-18.
//

#include "Camera.h"
#include <unistd.h>
#include <opencv2/opencv.hpp>

using namespace std;

Camera::Camera(int index) {
    this->VideoIndex = index;
    InitThread();
}

Camera::Camera(const string &videoSrc) {
    this->VideoSrc = videoSrc;
    InitThread();
}

Camera::~Camera() {
    IsExit = true;
    CameraThread.join();
    if (cap.isOpened()) {
        cap.release();
    }
}

bool Camera::start() {
    if (VideoSrc.empty())
        IsStart = cap.open(VideoIndex);
    else
        IsStart = cap.open(VideoSrc);
    return IsStart;
}

bool Camera::stop() {
    if (cap.isOpened()) {
        cap.release();
    }
    IsStart =  cap.isOpened();
    return IsStart;
}

cv::Mat Camera::GetImage() {
    cv::Mat mat;
    if (cap.isOpened()) {
        std::lock_guard<std::mutex> lk(CameraMutex);
        mat = LastMat;
    }
    return mat;
}

bool Camera::isOpened() {
    return cap.isOpened();
}

void Camera::InitThread() {
    CameraThread = thread(Run, this);
}

void Camera::Run(Camera *camera) {
    while (!camera->IsExit) {
        if (camera->cap.isOpened()) {
            std::lock_guard<std::mutex> lk(camera->CameraMutex);
            camera->cap >> camera->LastMat;
        }else if(camera->IsStart){
            camera->start();
        }
        usleep(10 * 1000);
    }
}
