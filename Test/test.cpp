#include<iostream>
#include<stdio.h>

#define uchar unsigned char
using namespace std;

void binary_print(uchar c) {
    for (int i = 0; i < 8; ++i) {
        if ((c << i) & 0x80)
            cout << '1';
        else
            cout << '0';
    }
    cout << ' ';
}

int main() {
    float a;
    uchar c_save[4];
    uchar i;
    void *f;
    f = &a;
    cout << "请输入一个浮点数:";
    cin >> a;
    cout << endl;
    for (i = 0; i < 4; i++) {
        c_save[i] = *((uchar *) f + i);
    }
    cout << "此浮点数在计算机内存中储存格式如下：" << endl;
    for (i = 4; i != 0; i--)
        binary_print(c_save[i - 1]);
    cout << endl;
    cout << "此浮点数在计算机内存中储存格式如下：" << endl;
    for (i = 1; i <= 4; i++)
        printf("0x%02X ", c_save[i - 1]);
    cout << endl;

    for (i = 0; i < 4; i++) {
        *((uchar *) f + i) = c_save[i];
    }
    printf("%f\n ", *(float*)f);
    return 0;
}