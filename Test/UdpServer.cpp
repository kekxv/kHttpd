//
// Created by caesar on 2019-08-17.
//

#include <UdpServer.h>
#include <unistd.h>
#include <Log.h>
#include <cerrno>
#include <cassert>

using namespace std;
using namespace kHttpdName;
#define BUF_SIZE 1024
const char *TAG = "main";

unsigned char CheckSum(const unsigned char *data, int N) {
    unsigned char chksum = 0;
    for (int idx = 0; idx < N; idx++)
        chksum += data[idx];
    return chksum;
}

void *s_memcpy(void *dest, void *src, unsigned int count) {
    assert((dest != nullptr) && (src != nullptr));
    if (dest == src)
        return src;
    char *d = (char *) dest;
    char *s = ((char *) src) + count - 1;

    while (count-- > 0) {
        *d++ = *s--;
    }
    return dest;
}

bool f_memcpy(float *dest, void *src) {
    assert((dest != nullptr) && (src != nullptr));
    if (dest == src)
        return false;

    float a;
    unsigned char c_save[4];
    unsigned char i;
    void *f;
    f = &a;
    memcpy(c_save, src, 4);
    for (i = 0; i < 4; i++) {
        *((unsigned char *) f + i) = c_save[i];
    }
    memcpy(dest, &a, sizeof(float));
    return true;
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
            {
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
            }
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
            {
                unsigned short SensorDeviceID = 0;
                unsigned int TrackID = 0;
                float X = 0, Y = 0, Z = 0, Speed = 0;
                unsigned short RCS = 0, Reserved;
                unsigned char TriggerFlag, Lane, Class, CRC;
                unsigned short Footer;

                memcpy(&SensorDeviceID, &rData[2], sizeof(SensorDeviceID));
                memcpy(&TrackID, &rData[3], sizeof(TrackID));
                memcpy(&X, &rData[8], sizeof(X));
                memcpy(&Y, &rData[12], sizeof(Y));
                memcpy(&Z, &rData[16], sizeof(Z));
                memcpy(&Speed, &rData[20], sizeof(Speed));
                memcpy(&RCS, &rData[24], sizeof(RCS));
                memcpy(&Reserved, &rData[26], sizeof(Reserved));
                memcpy(&TriggerFlag, &rData[28], sizeof(TriggerFlag));
                memcpy(&Lane, &rData[29], sizeof(Lane));
                memcpy(&Class, &rData[30], sizeof(Class));
                memcpy(&CRC, &rData[31], sizeof(CRC));
                memcpy(&CRC, &rData[31], sizeof(CRC));
                memcpy(&Footer, &rData[32], sizeof(Footer));
                if (Footer != 0xDEFF) {
                    LogE(TAG, "TRACK PACKET 错误的尾部");
                    return;
                }
                LogI(TAG,
                     "\nSensorDeviceID\t\t:%d"
                     "\nTrackID\t\t\t:%d"
                     "\nX\t\t\t:%f\nY\t\t\t:%f\nZ\t\t\t:%f"
                     "\nSpeed\t\t\t:%f"
                     "\nTriggerFlag\t\t:%d"
                     "\nLane\t\t\t:%d"
                     "\nClass\t\t\t:%d",
                     SensorDeviceID, TrackID,
                     X, Y, Z,
                     Speed,
                     TriggerFlag,
                     Lane,
                     Class);
            }
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

void show_help() {
    const char *help = "help (http://kekxv.com)\n\n"
                       "-l <ip_addr>        interface to listen on, default is 0.0.0.0\n"
                       "-p <num>            port number to listen on, default is 10000\n"
                       "-v                  open log show\n"
                       "-h                  print this help and exit\n"
                       "\n";
    fprintf(stderr, "%s", help);
}


int main(int argc, char *argv[]) {
    //默认参数
    string httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 10000;

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:hv")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = (int) strtol((const char *) optarg, nullptr, 10);
                break;
            case 'v' :
                kHttpdName::Log::setConsoleLevel(3);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }


    UdpServer udpServer(httpd_option_listen, httpd_option_port);
    udpServer.SetCallback(read_cb);
    udpServer.Listen();
    return 0;
}