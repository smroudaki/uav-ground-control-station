#include "qrcode.h"
//#include "ui_dialog.h"


void QrCode::operation(Mat *cap_frame, QString *code, int *center_x, int *center_y, float *rate_x, float *rate_y, float *screenScale)
{
    cap_frame_tmp = *cap_frame;

    width = cap_frame_tmp.cols;
    height = cap_frame_tmp.rows;

    circle(*cap_frame, Point(width / 2, height / 2), 1, Scalar(65, 255, 65), 5, LINE_AA);

    cvtColor(cap_frame_tmp, cap_frame_tmp, CV_BGR2GRAY);

    uchar *raw = (uchar *)cap_frame_tmp.data;
    Image image(width, height, "Y800", raw, width * height);

    scanner.scan(image);

    for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
    {
        vector<Point> vp;

        *code = QString::fromStdString(symbol->get_data());

        location_size = symbol->get_location_size();

        for (int i = 0; i < location_size; i++)
            vp.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));

        RotatedRect r = minAreaRect(vp);
        Point2f pts[4];
        r.points(pts);

        for (int i = 0; i < 4; i++)
            line(*cap_frame, pts[i], pts[(i + 1) % 4], Scalar(0, 255, 0), 3);

//        cout <<"Angle: " << r.angle << endl;

        *center_x = (symbol->get_location_x(3) + symbol->get_location_x(0)) / 2;
        *center_y = (symbol->get_location_y(0) + symbol->get_location_y(1)) / 2;

        num1_x = *center_x;
        num1_y = *center_y;

        *rate_x = ((num2_x - num1_x) / 3) * *screenScale * 100 / 640;
        *rate_y = ((num2_y - num1_y) / 3) * *screenScale * 100 / 480;

        num2_x = num1_x;
        num2_y = num1_y;

        circle(*cap_frame, Point(*center_x, *center_y), 1, Scalar(100, 100, 255), 3, LINE_AA);
    }

    if (image.symbol_begin() == image.symbol_end())
    {
        *code = "";

        *rate_x = 0;
        *rate_y = 0;

        *center_x = -1;
        *center_y = -1;
    }
}
