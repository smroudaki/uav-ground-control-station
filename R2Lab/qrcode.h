#ifndef QRCODE_H
#define QRCODE_H


#include <QDialog>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <zbar.h>

using namespace std;
using namespace cv;
using namespace zbar;

class QrCode
{
public:
    QrCode() { scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1); }

    void operation(Mat*, QString*, int*, int*, float*, float*, float*);

    Mat cap_frame_tmp;

    int width, height, location_size;

    ImageScanner scanner;

    float num1_x = 0, num1_y = 0, num2_x = 0, num2_y = 0;
};

#endif // QRCODE_H
