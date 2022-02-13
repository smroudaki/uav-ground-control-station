#include "motion.h"
//#include "ui_dialog.h"


TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS, 20, 0.03);
Size subPixWinSize(10, 10), winSize(31, 31);

void Motion::operation(Mat *cap_frame, int *pointsSize, float *rateX, float *rateY, bool *needToInit)
{
    cvtColor(*cap_frame, gray, CV_BGR2GRAY);

    if (*needToInit)
    {
        goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 5, noArray(), 3, false, 0.04);
        cornerSubPix(gray, points[1], subPixWinSize, Size(-1, -1), termcrit);

        addRemovePt = false;
    }

    else if (!points[0].empty())
    {
        if (prevGray.empty())
            gray.copyTo(prevGray);

        calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 3, termcrit, 0, 0.001);

        for (i = k = 0; i < points[1].size(); i++)
        {
            if (addRemovePt)
            {
                if (norm(point - points[1][i]) <= 5)
                {
                    addRemovePt = false;
                    continue;
                }
            }

            if (!status[i])
                continue;

            if ( (float)( (points[1][i].x - points[0][i].x) / 30 * 1000 ) > 30  ||
                 (float)( (points[1][i].x - points[0][i].x) / 30 * 1000 ) < -30 ||
                 (float)( (points[1][i].y - points[0][i].y) / 30 * 1000 ) > 30  ||
                 (float)( (points[1][i].y - points[0][i].y) / 30 * 1000 ) < -30 )
            {
                speedX += (float)(points[1][i].x - points[0][i].x);
                speedY += (float)(points[1][i].y - points[0][i].y);
                counter++;
            }

            points[1][k++] = points[1][i];
            circle(*cap_frame, points[1][i], 3, Scalar(0, 0, 255), -1, 8);
        }

        // height: 70cm ---> pixel coefficient: y=1 x=1 => cm coefficient: y=10 x=9.8
        if (counter != 0)
        {
            *rateX = (speedX / counter) / 9.8;
            *rateY = (speedY / counter) / 10;
        }
        else
        {
            *rateX = 0;
            *rateY = 0;
        }

        counter = 0;
        speedX = 0;
        speedY = 0;

        points[1].resize(k);
    }

    if (addRemovePt && points[1].size() < (size_t)MAX_COUNT)
    {
        vector<Point2f> tmp;

        tmp.push_back(point);
        cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
        points[1].push_back(tmp[0]);

        addRemovePt = false;
    }

    *needToInit = false;

    *pointsSize = points[1].size();

    if (*pointsSize < UPDATE_POINTS_THRESHOLD)
        *needToInit = true;

    swap(points[1], points[0]);
    swap(prevGray, gray);
}
