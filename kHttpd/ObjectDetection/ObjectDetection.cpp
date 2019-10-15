//
// Created by caesar on 2019/10/15.
//

#include <CJsonObject.hpp>
#include "ObjectDetection.h"

ObjectDetection::ObjectDetection(const string &pbPath, const string &pbtxtPath, const string &pbJson) {
    vector<uchar> weights, prototxt;
    net = cv::dnn::readNetFromTensorflow(pbPath, pbtxtPath);
    objectClass.clear();
    CJsonObject json(pbJson);
    if (json.IsArray()) {
        for (int i = 0; i < json.GetArraySize(); i++) {
            CJsonObject cjsonTmp = json[i];
            objectClass[cjsonTmp["id"].toNumber()] = cjsonTmp["display_name"].toString();
        }
    }
}

ObjectDetection::~ObjectDetection() {
    objectClass.clear();
}

bool ObjectDetection::IsReady() {
    return !net.empty() && !objectClass.empty();
}

vector<ObjectData> ObjectDetection::Detection(const string &FilePath, double confidenceThreshold) {
    Mat frame = cv::imread(FilePath);
    return Detection(frame, confidenceThreshold);
}

vector<ObjectData>
ObjectDetection::Detection(unsigned char *img, int width, int height, int step, double confidenceThreshold) {
    int nByte = height * width * step / 8;//字节计算
    int nType = CV_8UC3;
    switch (step) {
        case 8:
            nType = CV_8UC1;
            break;
        case 16:
            nType = CV_8UC2;
            break;
        case 24:
            nType = CV_8UC3;
            break;
        case 32:
            nType = CV_8UC4;
            break;
        default:
            break;
    }
    Mat frame = Mat::zeros(height, width, nType);
    memcpy(frame.data, img, nByte);
    return Detection(frame, confidenceThreshold);
}

vector<ObjectData> ObjectDetection::Detection(unsigned char *img, int size, double confidenceThreshold) {
    std::vector<uchar> data = std::vector<uchar>(img, img + size);
//    int nType = CV_8UC3;
//    switch (step) {
//        case 8:
//            nType = CV_8UC1;
//            break;
//        case 16:
//            nType = CV_8UC2;
//            break;
//        case 24:
//            nType = CV_8UC3;
//            break;
//        case 32:
//            nType = CV_8UC4;
//            break;
//        default:
//            break;
//    }
    cv::Mat frame = imdecode(data, cv::ImreadModes::IMREAD_ANYCOLOR);
    return Detection(frame, confidenceThreshold);
}

vector<ObjectData> ObjectDetection::Detection(Mat frame, double confidenceThreshold) {
    vector<ObjectData> vObjectData;
    if (IsReady() == 0)return vObjectData;
    cv::cvtColor(frame, frame, COLOR_RGBA2RGB);
    Size frame_size = frame.size();
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, Size(inWidth, inHeight), Scalar(), true, false);
    net.setInput(blob);
    Mat output = net.forward();
    Mat detectionMat(output.size[2], output.size[3], CV_32F, output.ptr<float>());

    for (int i = 0; i < detectionMat.rows; i++) {
        float confidence = detectionMat.at<float>(i, 2);

        if (confidence > confidenceThreshold) {
            ObjectData objectData;
            auto i1 = (size_t) (detectionMat.at<float>(i, 1));

            objectData.xLeftTop = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
            objectData.yLeftTop = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
            objectData.xRightBottom = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
            objectData.yRightBottom = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

            objectData.Confidence = confidence;
            objectData.Num = i1;
            objectData.label = objectClass[i1];
            vObjectData.push_back(objectData);
        }
    }

    return vObjectData;
}
