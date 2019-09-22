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

#define OG_REGION_MAXNUM 256
int OG_VIDEO_WIDTH = 1280;
int OG_VIDEO_HEIGHT = 720;
int OG_REGION_FOUCS = OG_VIDEO_WIDTH; //2952

float g_og_XOffset = 0.0;
float g_og_YOffset = 0.0;
float g_og_XOffset_Leav = 0.0;
float g_og_YOffset_Leav = 0.0;
float g_og_boxsize = 1.2;

float z_x = 0.0, z_y = 0.0, z_Z = 1.0;
#ifdef ENABLE_CAMERA
static Camera *camera = nullptr;
#endif
CarNumOcr *carNumOcr = nullptr;
static VideoWriter out;

struct TrackInfo {
    unsigned short SensorDeviceID = 0;
    unsigned int TrackID = 0;
    float X = 0, Y = 0, Z = 0, Speed = 0;
    unsigned short RCS = 0, Reserved;
    unsigned char TriggerFlag, Lane, Class, CRC;
    unsigned short Footer;
    int Width, Height;
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
        for (auto trackInfo : trackInfos) {

//            s->thickness = 3;
//            s->hsub = 1;
//            s->vsub = 1;

            if (trackInfo.X < 0) trackInfo.X = 0;
            if (trackInfo.Y < 0) trackInfo.Y = 0;
            Rect rect = Rect((int) trackInfo.X, (int) trackInfo.Y, (int) trackInfo.Width,
                             (int) trackInfo.Height);//起点；长宽
            Scalar color = Scalar(0, 255, 0);
            rectangle(img, rect, color, 2, LINE_8);

            cv::Point origin;
            origin.x = trackInfo.X;
            origin.y = trackInfo.Y + trackInfo.Height / 2;
            cv::putText(img, to_string(trackInfo.TrackID), origin,
                        cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 255),
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
        //1280*720
        if (info.Speed > 0) {
            info.X = OG_VIDEO_WIDTH / 2.0 - OG_REGION_FOUCS * info.X / info.Z + g_og_XOffset;
            info.Y = OG_VIDEO_HEIGHT / 2.0 - OG_REGION_FOUCS * info.Y / info.Z + g_og_YOffset;
        } else {
            info.X = OG_VIDEO_WIDTH / 2.0 - OG_REGION_FOUCS * info.X / info.Z + g_og_XOffset_Leav;
            info.Y = OG_VIDEO_HEIGHT / 2.0 - OG_REGION_FOUCS * info.Y / info.Z + g_og_YOffset_Leav;
        }
        info.Width = info.Height = 2.0 * OG_REGION_FOUCS * g_og_boxsize / info.Z; //(200-Z)/2;


        _TrackInfo.push_back(info);
    }

#ifdef ENABLE_CAMERA
    if (!TrackFlag && camera != nullptr) {
        if (camera->isOpened()) {
            Mat img = camera->GetImage();
            if (img.empty()) {
                LogE(TAG, "图片获取失败");
            } else {
                LogI(TAG, "图片获取成功");
                resize(img, img, Size(OG_VIDEO_WIDTH, OG_VIDEO_HEIGHT));
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
                OG_VIDEO_WIDTH = strtol((const char *) optarg, nullptr, 10);
                break;
            case 'H' :
                OG_VIDEO_HEIGHT = strtol((const char *) optarg, nullptr, 10);
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
        out.open(outVideo, VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(OG_VIDEO_WIDTH, OG_VIDEO_HEIGHT));


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