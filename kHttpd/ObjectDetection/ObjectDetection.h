//
// Created by caesar on 2019/10/15.
//

#ifndef KHTTPD_OBJECTDETECTION_H
#define KHTTPD_OBJECTDETECTION_H

#include<opencv2/opencv.hpp>
#include<opencv2/dnn.hpp>
#include <iostream>

using namespace std;
using namespace cv;


typedef struct
{
    // 信任度
    float Confidence = -1;
    // 模型序号
    int Num = -1;
    // 模型文本
    string label = {};
    // 左上角 x
    int xLeftTop{};
    // 左上角 y
    int yLeftTop{};
    // 右下角 x
    int xRightBottom{};
    // 右下角 y
    int yRightBottom{};
}ObjectData;

class ObjectDetection {
private:
    const size_t inWidth = 300;
    const size_t inHeight = 300;
    const float WHRatio = inWidth / (float) inHeight;
    dnn::Net net;
    map<int, string> objectClass;

public:
    ObjectDetection(const string &pbPath, const string &pbtxtPath, const string &pbJson);
    ~ObjectDetection();
    bool IsReady();
    vector<ObjectData> Detection(const string& FilePath, double confidenceThreshold = 0.3);
    vector<ObjectData> Detection(unsigned char img[], int width, int height, int step,double confidenceThreshold = 0.3);
    vector<ObjectData> Detection(unsigned char img[], int size, double confidenceThreshold = 0.3);
    vector<ObjectData> Detection(Mat frame,double confidenceThreshold = 0.3);
};


#endif //KHTTPD_OBJECTDETECTION_H
