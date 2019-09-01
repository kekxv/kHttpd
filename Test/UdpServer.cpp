//
// Created by caesar on 2019-08-17.
//

#include <UdpServer.h>

#ifdef ENABLE_CAMERA

#include <Camera.h>
#include <opencv2/opencv.hpp>

#endif

#include <unistd.h>
#include <Log.h>
#include <cerrno>
#include <cassert>
#include <kHttpd.h>
#include <CarNumOcr.h>

using namespace cv;
using namespace std;
using namespace kHttpdName;
#define BUF_SIZE 1024
const char *TAG = "main";

float z_x = 0.0, z_y = 0.0, z_Z = 1.0;
int width = 1400, height = 900;
#ifdef ENABLE_CAMERA
static Camera *camera = nullptr;
#endif
CarNumOcr *carNumOcr = nullptr;
VideoWriter out;

struct TrackInfo {
    unsigned short SensorDeviceID = 0;
    unsigned int TrackID = 0;
    float X = 0, Y = 0, Z = 0, Speed = 0;
    unsigned short RCS = 0, Reserved;
    unsigned char TriggerFlag, Lane, Class, CRC;
    unsigned short Footer;
};
bool TrackFlag = false;

unsigned char CheckSum(const unsigned char *data, int N) {
    unsigned char chksum = 0;
    for (int idx = 0; idx < N; idx++)
        chksum += data[idx];
    return chksum;
}

/**
 * 跟踪消息处理
 * @param rData
 */
void RunTracks(Mat img, vector<TrackInfo> trackInfos) {
    try {
        double s32X, s32Y, u32Height, u32Width;
        double OG_VIDEO_WIDTH = img.cols;
        double OG_VIDEO_HEIGHT = img.rows;
        double OG_REGION_FOUCS = OG_VIDEO_WIDTH;
        for (auto trackInfo : trackInfos) {
            //1280*720
            s32X = trackInfo.X * z_Z + z_x;
            s32Y = trackInfo.Y * z_Z + z_y;
            if (trackInfo.Speed > 0) {
                s32X += trackInfo.Speed * z_Z;
                s32Y += trackInfo.Speed * z_Z;
            }
            u32Width = u32Height = OG_VIDEO_WIDTH - trackInfo.Z * z_Z; //(200-Z)/2;
            if (u32Width < 0) {
                u32Width = u32Height = 0 - u32Width;
            }
            if (s32X < 0) s32X = 0;
            if (s32Y < 0) s32Y = 0;
            Rect rect = Rect((int) s32X, (int) s32Y, (int) u32Width, (int) u32Height);//起点；长宽
            Scalar color = Scalar(0, 255, 0);
            rectangle(img, rect, color, 2, LINE_8);

            cv::Point origin;
            origin.x = s32X;
            origin.y = s32Y + u32Height / 2;
            cv::putText(img, to_string(trackInfo.TrackID), origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 255),
                        2, 8, false);

        }
        if (out.isOpened())
            out.write(img);
    } catch (cv::Exception &e) {
        LogE(TAG, "处理数据失败:%s", e.what());
    }
    TrackFlag = false;

}

/**
 * 状态消息处理
 * @param rData
 */
void RunStatus(unsigned char *rData, UdpServer &udpServer) {
    unsigned char Alive, FrameSize, Framerate;
    unsigned short SensorDeviceID = 0;
    unsigned char nTracks;
    unsigned short verServer = 0;
    unsigned char verCore, verAnalytics, verFirmware, CRC;
    unsigned short Footer = 0;
    memcpy(&Alive, &rData[2], sizeof(Alive));
    memcpy(&FrameSize, &rData[3], sizeof(FrameSize));
    memcpy(&Framerate, &rData[4], sizeof(Framerate));
    memcpy(&SensorDeviceID, &rData[5], sizeof(SensorDeviceID));
    memcpy(&nTracks, &rData[7], sizeof(nTracks));
    memcpy(&verServer, &rData[8], sizeof(verServer));
    memcpy(&verCore, &rData[10], sizeof(verCore));
    memcpy(&verAnalytics, &rData[11], sizeof(verAnalytics));
    memcpy(&verFirmware, &rData[12], sizeof(verFirmware));
    memcpy(&CRC, &rData[13], sizeof(CRC));
    memcpy(&Footer, &rData[14], sizeof(Footer));
    if (Footer != 0xEEFF) {
        LogE(TAG, "STATUS PACKET 错误的尾部");
        return;
    }
    /*
    LogI(TAG,
         "\nAlive\t\t\t:0x%02X"
         "\nFrameSize\t\t:0x%02X"
         "\nFramerate\t\t:0x%02X"
         "\nSensorDeviceID\t\t:%d"
         "\nnTracks\t\t\t:0x%02X"
         "\nverServer\t\t:%d"
         "\nverCore\t\t\t:0x%02X"
         "\nverAnalytics\t\t:0x%02X"
         "\nverFirmware\t\t:0x%02X", Alive, FrameSize, Framerate,
         SensorDeviceID, nTracks, verServer,
         verCore,
         verAnalytics,
         verFirmware);
         */

    unsigned char mData[BUF_SIZE];
    int len;
    struct sockaddr_in client_addr{};
    vector<TrackInfo> _TrackInfo;
    for (unsigned char i = 0; i < nTracks; i++) {
        memset(mData, 0, sizeof(mData));
        len = udpServer.read((void *) mData, sizeof(mData), &client_addr);
        if (len == -1)return;
        if (len == 0)return;
        if (len < 2)return;
        unsigned int Header = 0;//= (mData[1] << 0) | (mData[0] << 8);
        memcpy(&Header, &mData[0], sizeof(unsigned short));
        if (Header != 0x2101) {
            i--;
            continue;
        }
        //3.TRACK PACKET
        if (len < 34) return;
        if (CheckSum(&mData[0], 31) != mData[31]) return;
        TrackInfo info{};
        memcpy(&info.SensorDeviceID, &mData[2], sizeof(info.SensorDeviceID));
        memcpy(&info.TrackID, &mData[3], sizeof(info.TrackID));
        memcpy(&info.X, &mData[8], sizeof(info.X));
        memcpy(&info.Y, &mData[12], sizeof(info.Y));
        memcpy(&info.Z, &mData[16], sizeof(info.Z));
        memcpy(&info.Speed, &mData[20], sizeof(info.Speed));
        memcpy(&info.RCS, &mData[24], sizeof(info.RCS));
        memcpy(&info.Reserved, &mData[26], sizeof(info.Reserved));
        memcpy(&info.TriggerFlag, &mData[28], sizeof(info.TriggerFlag));
        memcpy(&info.Lane, &mData[29], sizeof(info.Lane));
        memcpy(&info.Class, &mData[30], sizeof(info.Class));
        memcpy(&info.CRC, &mData[31], sizeof(info.CRC));
        memcpy(&info.CRC, &mData[31], sizeof(info.CRC));
        memcpy(&info.Footer, &mData[32], sizeof(info.Footer));
        if (info.Footer != 0xDEFF) {
            LogE(TAG, "TRACK PACKET 错误的尾部");
            return;
        }
        _TrackInfo.push_back(info);
    }

#ifdef ENABLE_CAMERA
    if (!TrackFlag && camera != nullptr) {
//        camera->stop();
//        camera->start();
        if (camera->isOpened()) {
            Mat img = camera->GetImage();
            if (img.empty()) {
                LogE(TAG, "图片获取失败");
            } else {
                LogI(TAG, "图片获取成功");
                resize(img, img, Size(width, height));
                TrackFlag = true;
                thread thread(RunTracks, img, _TrackInfo);
                thread.detach();
            }
        } else {
            LogE(TAG, "视频流开启失败");
        }
    }
#endif
}


void Track(Mat img, TrackInfo trackInfo) {
    try {
        double s32X, s32Y, u32Height, u32Width;
        double OG_VIDEO_WIDTH = img.cols;
        double OG_VIDEO_HEIGHT = img.rows;
        double OG_REGION_FOUCS = OG_VIDEO_WIDTH;
        //1280*720
        s32X = trackInfo.X * z_Z + z_x;
        s32Y = trackInfo.Y * z_Z + z_y;
        if (trackInfo.Speed > 0) {
            s32X += trackInfo.Speed * z_Z;
            s32Y += trackInfo.Speed * z_Z;
        }
        u32Width = u32Height = trackInfo.Z * z_Z; //(200-Z)/2;
        if (s32X < 0) s32X = 0;
        if (s32Y < 0) s32Y = 0;
        Rect rect = Rect((int) s32X, (int) s32Y, (int) u32Width, (int) u32Height);//起点；长宽
        Scalar color = Scalar(0, 255, 0);
        rectangle(img, rect, color, 2, LINE_8);

//        imwrite(string("./image")
//                + "[" + to_string(s32X) + "]"
//                + "[" + to_string(s32Y) + "]"
//                + "[" + to_string(u32Width) + "]"
//                + "[" + to_string(u32Height) + "]" + ".png", img);
//        out.write(img);
        if (out.isOpened())
            out.write(img);
    } catch (cv::Exception &e) {
        LogE(TAG, "处理数据失败:%s", e.what());
    }
    TrackFlag = false;
}

/**
 * 跟踪消息处理
 * @param rData
 */
void RunTrack(unsigned char *rData) {
    TrackInfo info{};

    memcpy(&info.SensorDeviceID, &rData[2], sizeof(info.SensorDeviceID));
    memcpy(&info.TrackID, &rData[3], sizeof(info.TrackID));
    memcpy(&info.X, &rData[8], sizeof(info.X));
    memcpy(&info.Y, &rData[12], sizeof(info.Y));
    memcpy(&info.Z, &rData[16], sizeof(info.Z));
    memcpy(&info.Speed, &rData[20], sizeof(info.Speed));
    memcpy(&info.RCS, &rData[24], sizeof(info.RCS));
    memcpy(&info.Reserved, &rData[26], sizeof(info.Reserved));
    memcpy(&info.TriggerFlag, &rData[28], sizeof(info.TriggerFlag));
    memcpy(&info.Lane, &rData[29], sizeof(info.Lane));
    memcpy(&info.Class, &rData[30], sizeof(info.Class));
    memcpy(&info.CRC, &rData[31], sizeof(info.CRC));
    memcpy(&info.CRC, &rData[31], sizeof(info.CRC));
    memcpy(&info.Footer, &rData[32], sizeof(info.Footer));
    if (info.Footer != 0xDEFF) {
        LogE(TAG, "TRACK PACKET 错误的尾部");
//        return;
    }

    LogI(TAG,
         "\nSensorDeviceID\t\t:%d"
         "\nTrackID\t\t\t:%d"
         "\nX\t\t\t:%f\nY\t\t\t:%f\nZ\t\t\t:%f"
         "\nSpeed\t\t\t:%f"
         "\nTriggerFlag\t\t:%d"
         "\nLane\t\t\t:%d"
         "\nClass\t\t\t:%d",
         info.SensorDeviceID, info.TrackID,
         info.X, info.Y, info.Z,
         info.Speed,
         info.TriggerFlag,
         info.Lane,
         info.Class);

#ifdef ENABLE_CAMERA
    if (!TrackFlag && camera != nullptr) {
//        camera->stop();
//        camera->start();
        if (camera->isOpened()) {
            Mat img = camera->GetImage();
            if (img.empty()) {
                LogE(TAG, "图片获取失败");
            } else {
                LogI(TAG, "图片获取成功");
                resize(img, img, Size(width, height));
                // imshow("img", img);
                // waitKey(0);
                TrackFlag = true;
                thread thread(Track, img, info);
                thread.detach();
            }
        } else {
            LogE(TAG, "视频流开启失败");
        }
    }
#endif
}

void read_cb(UdpServer &udpServer) {
    unsigned char rData[BUF_SIZE];
    int len;
    struct sockaddr_in client_addr{};
    memset(rData, 0, sizeof(rData));
    len = udpServer.read((void *) rData, sizeof(rData), &client_addr);

    if (len == -1) {
        LogE(TAG, "read error: %d  %s ", errno, strerror(errno));
        return;
    }
    if (len == 0) {
        LogE(TAG, "Connection Closed");
        return;
    }
    if (len < 2) {
        LogE(TAG, "read len error: %d", len);
        return;
    }

    unsigned int Header = 0;//= (rData[1] << 0) | (rData[0] << 8);
    memcpy(&Header, &rData[0], sizeof(unsigned short));
    switch (Header) {
        case 0x0101://1.GET TIME PACKET
            LogI(TAG, "接收到 GET TIME PACKET，丢弃不处理");
            break;
        case 0x1101://2.STATUS PACKET
            if (len != 16) {
                LogE(TAG, "STATUS PACKET read len error: %d", len);
                return;
            }
            if (CheckSum(&rData[0], 13) != rData[13]) {
                LogE(TAG, "STATUS PACKET CRC error");
                return;
            }
            LogD(TAG, "接收到 STATUS PACKET;准备处理");
            RunStatus(rData, udpServer);
            break;
        case 0x2101://3.TRACK PACKET
            if (len != 34) {
                LogE(TAG, "TRACK PACKET read len error: %d", len);
                return;
            }
            if (CheckSum(&rData[0], 31) != rData[31]) {
                LogE(TAG, "TRACK PACKET CRC error");
                return;
            }
            LogD(TAG, "接收到 TRACK PACKET;准备处理");
            // RunTrack(rData);
            break;
        case 0x5101://4.STATISTICS PACKET for Approaching direction
            LogI(TAG, "接收到 STATISTICS PACKET for Approaching direction，丢弃不处理");
            break;
        case 0x5201://4.STATISTICS PACKET for Leaving direction
            LogI(TAG, "接收到 STATISTICS PACKET for Leaving direction，丢弃不处理");
            break;
        case 0x6101://5.QUEUE PACKET for Approaching direction
            LogI(TAG, "接收到 QUEUE PACKET for Approaching direction，丢弃不处理");
            break;
        case 0x6201://5.QUEUE PACKET for Leaving direction
            LogI(TAG, "接收到 QUEUE PACKET for Leaving direction，丢弃不处理");
            break;
        case 0x5401://6.EVENT PACKET for Approaching direction
            LogI(TAG, "接收到 EVENT PACKET for Approaching direction，丢弃不处理");
            break;
        case 0x5501://6.EVENT PACKET for Leaving direction
            LogI(TAG, "接收到 EVENT PACKET for Leaving direction，丢弃不处理");
            break;

        case 0x2222://7.TRACK PACKET
            LogI(TAG, "接收到 TRACK PACKET，丢弃不处理");
            break;

        default:
            LogI(TAG, "接收到 未知类型包：0x%02X%02X ;丢弃不处理", rData[1], rData[0]);
            break;
    }

    // Log::D_HX("", len, buf, "read");


//    udpServer.write(buf, len, &client_addr);

}

#ifdef ENABLE_CAMERA

void TestCamera(Camera *pCamera) {
    if (pCamera != nullptr) {
        pCamera->stop();
        pCamera->start();
        while (pCamera->isOpened()) {
            Mat img = pCamera->GetImage();
            if (img.empty()) {
                LogE(TAG, "图片获取失败");
            } else {
                LogI(TAG, "图片获取成功");
                usleep(100 * 1000);
            }
        }
    }
}

#endif

void show_help() {
    const char *help = "help (http://kekxv.com)\n\n"
                       "-l <ip_addr>        interface to listen on, default is 0.0.0.0\n"
                       "-p <num>            port number to listen on, default is 10000\n"
                       "-v                  open log show\n"
                       "-c <rtsp>           open rtsp video\n"
                       "-C                  test rtsp\n"
                       "-x                  原点偏移 x\n"
                       "-y                  原点偏移 y\n"
                       "-Z                  汽车和距离的比例\n"
                       "-o <video path>     要保存的视频地址\n"
                       "-O <CarNumOcr>      enable CarNumOcr\n"
                       "-h                  print this help and exit\n"
                       "\n";
    fprintf(stderr, "%s", help);
}


int main(int argc, char *argv[]) {
    //默认参数
    string httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 9935;
    bool cTest = false;
    string outVideo;

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:hvc:CO::x::y::W::H::F:Z:o:")) != -1) {
        switch (c) {
            case 'F' :
                imshow(optarg, imread(optarg));
                waitKey(0);
                return 0;
                break;
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = (int) strtol((const char *) optarg, nullptr, 10);
                break;
            case 'x' :
                z_x = strtof((const char *) optarg, nullptr);
                break;
            case 'y' :
                z_y = strtof((const char *) optarg, nullptr);
                break;
            case 'Z' :
                z_Z = strtof((const char *) optarg, nullptr);
                break;
            case 'W' :
                width = strtol((const char *) optarg, nullptr, 10);
                break;
            case 'H' :
                height = strtol((const char *) optarg, nullptr, 10);
                break;
            case 'v' :
                kHttpdName::Log::setConsoleLevel(3);
                break;
            case 'c' :
#ifdef ENABLE_CAMERA
                camera = new Camera(optarg);
                camera->start();
#endif
                break;
            case 'C':
                cTest = true;
                break;
            case 'o':
                outVideo = optarg;
                break;
            case 'O' :
                if (carNumOcr == nullptr) {
                    if (optarg == nullptr)
                        carNumOcr = new CarNumOcr();
                    else
                        carNumOcr = new CarNumOcr(kHttpd::GetRootPath(optarg));
                }
                LogI("CarNumOcr", "开启车牌识别！");
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }
#ifdef ENABLE_CAMERA
    if (cTest) {
        TestCamera(camera);
        exit(EXIT_SUCCESS);
        return 1;
    }
#endif
    if (!outVideo.empty())
        out.open(outVideo, VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(width, height));


    UdpServer udpServer(httpd_option_listen, httpd_option_port);
    udpServer.SetCallback(read_cb);
    udpServer.Listen();
#ifdef ENABLE_CAMERA
    if (camera != nullptr) {
        camera->stop();
        delete camera;
    }
#endif
    if (out.isOpened()) {
        out.release();
    }
    if (carNumOcr != nullptr) {
        delete carNumOcr;
        carNumOcr = nullptr;
    }
    return 0;
}