#include "circlecolorrec.h"
//#include "ui_dialog.h"


void CircleColorRec::operation(Mat *cap_frame, int hsvValues[], int *x, int *y)
{
    cvtColor(*cap_frame, *cap_frame, COLOR_BGR2HSV);

    inRange(*cap_frame, Scalar(hsvValues[0], hsvValues[1], hsvValues[2]), Scalar(hsvValues[3], hsvValues[4], hsvValues[5]), *cap_frame);

    //morphological opening (remove small objects from the foreground)
    erode(*cap_frame, *cap_frame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( *cap_frame, *cap_frame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    dilate( *cap_frame, *cap_frame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(*cap_frame, *cap_frame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    addWeighted(*cap_frame, 1.0, *cap_frame, 2, 0.0, *cap_frame);
    GaussianBlur(*cap_frame, *cap_frame, Size(9, 9), 2, 2.5);

    cap_frame_tmp = *cap_frame;

    HoughCircles(*cap_frame, circles, CV_HOUGH_GRADIENT, 1, cap_frame_tmp.rows/8, 120, 30, 50, 200);

    for (size_t i = 0; i < circles.size(); i++)
    {
        c = circles[i];

        Point center(c[0], c[1]);
        int radius = c[2];

        circle(*cap_frame, center, 3, Scalar(0, 0, 255), -1, 8, 0);
        circle(*cap_frame, center, radius, Scalar(0, 0, 255), 3, LINE_AA);

        oss_location.str("");
        oss_location.clear();

        oss_location << "(" << c[0] << "," << c[1] << ")";

        putText(*cap_frame, oss_location.str(), Point(c[0] - 45, c[1] - 20), FONT_ITALIC, 0.6, Scalar(0, 0, 0), 1, LINE_AA);

        *x = c[0];
        *y = c[1];
    }

    if (circles.size() == 0 || circles.size() >= 2)
    {
        *x = -1;
        *y = -1;
    }
}
