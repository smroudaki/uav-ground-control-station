#ifndef CIRCLECOLORREC_H
#define CIRCLECOLORREC_H


#include <QDialog>

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class CircleColorRec
{
public:
    void operation(Mat*, int[], int*, int*);

    Mat cap_frame_tmp;

    vector<Vec3f> circles;

    Vec3i c;

    ostringstream oss_location;
};

#endif // CIRCLECOLORREC_H
