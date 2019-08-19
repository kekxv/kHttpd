//
// Created by caesar on 2019-08-18.
//

#include "Camera.h"

#ifdef _WIN32
#include <windows.h>
#else

#include <time.h>
#include <sys/time.h>

#endif

#include <unistd.h>
#include <opencv2/opencv.hpp>

using namespace std;

long long Camera::GetTime() {
#ifdef _WIN32
    // 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    long long tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
    tt = (li.QuadPart - EPOCHFILETIME) / 10 / 1000;
    return tt;
#else
    timeval tv{};
    gettimeofday(&tv, 0);
    return (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
#endif // _WIN32
    return 0;
}

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
    auto e = GetTime() + 1000 * 3;
    while (GetImage().empty()) {
        if (GetTime() > e)break;
    };
    return IsStart;
}

bool Camera::stop() {
    if (cap.isOpened()) {
        cap.release();
    }
    IsStart = cap.isOpened();
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
        } else if (camera->IsStart) {
            camera->cap.release();
            camera->start();
        }
        usleep(50 * 1000);
    }
}
