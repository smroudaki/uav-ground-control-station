#ifndef MOTION_H
#define MOTION_H


#include <QDialog>

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Motion
{
public:
    // Initialization:
    const int MAX_COUNT = 500;
    const int UPDATE_POINTS_THRESHOLD = 25;

    void operation(Mat*, int*, float*, float*, bool*);

    Mat gray, prevGray;

    bool addRemovePt = false;

    vector<uchar> status;
    vector<float> err;

    size_t i, k;

    Point2f point;
    vector<Point2f> points[2];

    float speedX = 0, speedY = 0;
    int counter = 0;
};

#endif // MOTION_H
