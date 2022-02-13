#ifndef DIALOG_H
#define DIALOG_H


#include "savenewface.h"

#include <QDialog>
#include <QDesktopWidget>
#include <QTimer>
#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCategoryAxis>
#include <QGridLayout>

// xml
#include <qxmlstream.h>
#include <QDir>
#include <QFile>
#include <QTime>
#include <QFileDialog>
#include <QDebug>

#include "circlecolorrec.h"
#include "qrcode.h"
#include "motion.h"
#include "facerec.h"
#include "ocr.h"


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0, bool run_once = false);
    ~Dialog();

    // Initialization:                  // ---> also "on_btnResetFaceRecSettings_clicked()" slot
    float screenScale = 33;
    int skip_frames = 10;
    double rec_threshold = 0.52;
    double tracking_quality_threshold = 5;
    string ip_firstPart = "http://admin:910209545@", ip_lastPart = "/video.cgi?.mjpg";

    int second_window_width = 840, second_window_height = 175;

    VideoCapture cap;

    QTimer *mainCam_timer, *circleColorRec_timer, *qrCode_timer, *motion_timer, *faceRec_timer, *ocr_timer, *refresher_timer, *serialWrite_timer;

    bool needToInit = true;

    int lstQrCodeIndexCounter = 0;

    string ip, firstIpValue, secondIpValue, thirdIpValue, forthIpValue;

    QString getCode;
    int getPointsSize_motion = 0;
    float getRateX_motion = 0, getRateY_motion = 0, getRateX_qrCode = 0, getRateY_qrCode = 0;
    int getX_circleColorRec = -1, getY_circleColorRec = -1, getX_qrCode = -1, getY_qrCode = -1;

    void load_R2Lab_logo();
    void load_backgroundImages();
    void clear_backgroundImages();

    void set_cap();
    bool set_cap_check, is_cap_open = true;

    void start_timers();
    void stop_timers();

    void set_ipVisibility(bool);

    int skip_frames_counter = 0;
    double tracking_quality_sum = 0;
    bool resetPBarTrackingQuality;

    vector<string> getLabels;
    int getTrackersSize;

    void thingsToDo_after_loadingFaceDatabase();

    void show_save_window();

    QSerialPort serial;
    QByteArray ba, ba_log, recieve;
    int send_counter = 0, serial_counter = 0, ba_sum, temp;
    bool go_on_length = false, go_on_allocation = false;
    void allocate();

    list<int> show_priority;

    // ############################## chart ##############################

    // Chart variables
    vector<float> x_rate_average, y_rate_average, circleCenter_xVect, circleCenter_yVect, heightVect;
    QGridLayout *gridLayout_xChart, *gridlayout_yChart, *gridlayout_zChart;
    QFont labelsFont, font;
    QLinearGradient backgroundGradient;

    // velocity_x
    int pointCounter_velocity_x = 0, axisX_counter1_velocity_x = 20, loopCounter_velocity_x = 0, axisX_counter2_velocity_x = 1000;
    // velocity_y
    int pointCounter_velocity_y = 0, axisX_counter1_velocity_y = 20, loopCounter_velocity_y = 0, axisX_counter2_velocity_y = 1000;

    // place_x
    int pointCounter_place_x = 0, axisX_counter1_place_x = 20, loopCounter_place_x = 0, axisX_counter2_place_x = 1000;
    // place_y
    int pointCounter_place_y = 0, axisX_counter1_place_y = 20, loopCounter_place_y = 0, axisX_counter2_place_y = 1000;

    // height
    int pointCounter_height = 0, axisX_counter1_height = 20, loopCounter_height = 0, axisX_counter2_height = 1000 ;

    QtCharts::QLineSeries *velocitySeries_x, *velocitySeries_y, *heightSeries, *placeSeries_x, *placeSeries_y;
    QtCharts::QChart *chart_x, *chart_y, *chart_z;
    QtCharts::QCategoryAxis *axisX_chartX, *axisY_velocity_chartX, *axisY_place_chartX, *axisX_chartY, *axisY_velocity_chartY, *axisY_place_chartY, *axisX_chartZ, *axisY_height;

    void x_chartDisplay();
    void y_chartDiplay();
    void z_chartDisplay();

    // xml

    QFile circleXmlFile;

    QXmlStreamWriter *streamWriterPlace = new QXmlStreamWriter();

    int circleXmlCounter = 0 ;

    QString circleXmlPath = QDir::currentPath() + "circle.xml" ;

    QFile motionXmlFile;

    QXmlStreamWriter *streamWriterRate = new QXmlStreamWriter();

    int motionXmlCounter = 0 ;

    QString motionXmlPath = QDir::currentPath() + "motion.xml";

    void circleXmlAppendingFunction();

    void motionXmlAppendingFunction();

    // ###################################################################

private slots:
    void on_btnEnableDefaultCamMode_clicked();

    void timer_mainCam();

    void timer_circleColorRec();

    void timer_qrCode();

    void timer_motion();

    void timer_faceRec();

    void timer_ocr();

    void timer_refresher();

    void timer_serialWrite();

    void read_data();

    void on_cmbDefaultColors_activated(int index);

    void on_btnPause_clicked();

    void on_btnDisconnect_clicked();

    void on_sliderLowH_sliderPressed();

    void on_sliderLowS_sliderPressed();

    void on_sliderLowV_sliderPressed();

    void on_sliderHighH_sliderPressed();

    void on_sliderHighS_sliderPressed();

    void on_sliderHighV_sliderPressed();

    void on_chckBoxQrCode_stateChanged();

    void on_chckBoxCircleColorRec_stateChanged();

    void on_chckBoxFaceRec_stateChanged();

    void on_chckBoxMotion_stateChanged();

    void on_chckBoxOcr_stateChanged();

    void on_btnClear_clicked();

    void on_btnResetFaceRecSettings_clicked();

    void on_btnMotion_clicked();

    void on_sbDistance_valueChanged(int arg1);

    void on_cmbCapMode_activated(int index);

    void on_txtFirstIpValue_textChanged();

    void on_txtSecondIpValue_textChanged();

    void on_txtThirdIpValue_textChanged();

    void on_txtForthIpValue_textChanged();

    void on_btnSetIp_clicked();

    void on_hSliderRecThreshold_valueChanged(int);

    void on_hSliderTrackingQualityThreshold_valueChanged(int);

    void on_sbSkipFrames_valueChanged(int);

    void on_chckBoxLoadDatabase_stateChanged(int);

    void on_btnSaveByImagesAdd_clicked();

    void on_btnSaveByTakingPhoto_clicked();

    void on_btnDeleteFromDatabase_clicked();

    void onEnteredTextSent(const QString&);

    void on_dsbPRoll_valueChanged(double arg1);

    void on_dsbIRoll_valueChanged(double arg1);

    void on_dsbDRoll_valueChanged(double arg1);

    void on_dsbPPitch_valueChanged(double arg1);

    void on_dsbIPitch_valueChanged(double arg1);

    void on_dsbDPitch_valueChanged(double arg1);

    void on_dsbPAlt_valueChanged(double arg1);

    void on_dsbIAlt_valueChanged(double arg1);

    void on_dsbDAlt_valueChanged(double arg1);

    void on_sbHeight_valueChanged(int arg1);

    void on_sbReserve1_1_valueChanged(int arg1);

    void on_sbReserve1_2_valueChanged(int arg1);

    void on_sbReserve1_3_valueChanged(int arg1);

    void on_sbReserve2_1_valueChanged(int arg1);

    void on_sbReserve2_2_valueChanged(int arg1);

    void on_sbReserve2_3_valueChanged(int arg1);

    void on_btnRead_clicked();

    void on_btnWrite_clicked();

    void on_btnComConnect_clicked();

    void on_btnComDisconnect_clicked();

    void on_btnComRefresh_clicked();

    void on_btnClearTxtRecievedData_clicked();

    void on_btnFreezeTxtRecievedData_clicked();

    void on_chckBoxControl_stateChanged();
    
    void on_chckBoxLog_stateChanged();

    // xml
    void on_btnBrowseCircleXmlLocation_clicked();
    void on_btnCircleCompLog_clicked();
    void on_btnDeleteCircleLog_clicked();

    void on_btnBrowseMotionXmlLocation_clicked();
    void on_btnMotionCompLog_clicked();
    void on_btnDeleteMotionLog_clicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
