#include "dialog.h"
#include "ui_dialog.h"


CircleColorRec circlecolorrec;
QrCode qrcode;
Motion motion;
FaceRec facerec;

Dialog::Dialog(QWidget *parent, bool run_once) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    if (run_once)
    {
        set_ipVisibility(false);

        ui->btnDisconnect->setEnabled(false);
        ui->btnPause->setEnabled(false);
        ui->btnMotion->setEnabled(false);
        ui->btnSaveByTakingPhoto->setEnabled(false);

        ui->btnRead->setToolTip("msg id <font color='red'><b>0x02</b></font>");
        ui->btnWrite->setToolTip("msg id <font color='red'><b>0x01</b></font>");
        ui->chckBoxControl->setToolTip("if checked<br>msg id <font color='red'><b>0x03</b></font><br>else<br>msg id <font color='red'><b>0x00</b></font>");

        // ############################## chart ##############################

        velocitySeries_x = new QtCharts::QLineSeries();
        velocitySeries_y = new QtCharts::QLineSeries();
        heightSeries = new QtCharts::QLineSeries();
        placeSeries_x = new QtCharts::QLineSeries();
        placeSeries_y = new QtCharts::QLineSeries();

        chart_x = new QtCharts::QChart();
        chart_y = new QtCharts::QChart();
        chart_z = new QtCharts::QChart();

        axisX_chartX = new QtCharts::QCategoryAxis();
        axisY_velocity_chartX = new QtCharts::QCategoryAxis();
        axisY_place_chartX = new QtCharts::QCategoryAxis();
        axisX_chartY = new QtCharts::QCategoryAxis();
        axisY_velocity_chartY = new QtCharts::QCategoryAxis();
        axisY_place_chartY = new QtCharts::QCategoryAxis();
        axisX_chartZ = new QtCharts::QCategoryAxis();
        axisY_height = new QtCharts::QCategoryAxis();

        gridLayout_xChart = new QGridLayout(ui->chartWidget_x);
        gridlayout_yChart = new QGridLayout(ui->chartWidget_y);
        gridlayout_zChart = new QGridLayout(ui->chartWidget_z);

        x_chartDisplay();
        y_chartDiplay();
        z_chartDisplay();

        // ###################################################################

        mainCam_timer = new QTimer(this);
        circleColorRec_timer = new QTimer(this);
        qrCode_timer = new QTimer(this);
        motion_timer = new QTimer(this);
        faceRec_timer = new QTimer(this);
        ocr_timer = new QTimer(this);
        refresher_timer = new QTimer(this);
        serialWrite_timer = new QTimer(this);

        connect(qrCode_timer, SIGNAL(timeout()), this, SLOT(timer_qrCode()));
        connect(circleColorRec_timer, SIGNAL(timeout()), this, SLOT(timer_circleColorRec()));
        connect(motion_timer, SIGNAL(timeout()), this, SLOT(timer_motion()));
        connect(faceRec_timer, SIGNAL(timeout()), this, SLOT(timer_faceRec()));
        connect(mainCam_timer, SIGNAL(timeout()), this, SLOT(timer_mainCam()));
        connect(ocr_timer, SIGNAL(timeout()), this, SLOT(timer_ocr()));

        connect(serialWrite_timer, SIGNAL(timeout()), this, SLOT(timer_serialWrite()));
        connect(refresher_timer, SIGNAL(timeout()), this, SLOT(timer_refresher()));
        connect(&serial, SIGNAL(readyRead()), this, SLOT(read_data()));

        load_R2Lab_logo();
        load_backgroundImages();

        facerec.load_labels_only(&getLabels, cap, &resetPBarTrackingQuality);
        thingsToDo_after_loadingFaceDatabase();

        if (!cap.isOpened())
            cap.open(ui->cmbCapMode->currentIndex());

        on_chckBoxControl_stateChanged();
        ba_log.resize(7);

        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            ui->cmbCom->addItem(info.portName());

        cout << "Frame size: " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << " x " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
    }
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::load_R2Lab_logo()
{
    Mat R2LabLogo = imread("FormBackgrounds/R2Lab-background.jpg");

    cvtColor(R2LabLogo, R2LabLogo, COLOR_BGR2RGB);

    cv::resize(R2LabLogo, R2LabLogo, Size(900, 900));

    QImage logoImage = QImage((uchar*) R2LabLogo.data, R2LabLogo.cols, R2LabLogo.rows, R2LabLogo.step, QImage::Format_RGB888);
    ui->lblR2Lab->setPixmap(QPixmap::fromImage(logoImage));
}

void Dialog::load_backgroundImages()
{
    ui->lblDefaultCameraMode->setStyleSheet("background-image: url(FormBackgrounds/defaultmode-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    ui->lblCircleColorRec->setStyleSheet("background-image: url(FormBackgrounds/circlecolorrec-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    ui->lblQrCode->setStyleSheet("background-image: url(FormBackgrounds/qrcode-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    ui->lblMotion->setStyleSheet("background-image: url(FormBackgrounds/motion-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    ui->lblFaceRec->setStyleSheet("background-image: url(FormBackgrounds/facerecognition-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    ui->lblOcr->setStyleSheet("background-image: url(FormBackgrounds/ocr-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
}

void Dialog::clear_backgroundImages()
{
    ui->lblDefaultCameraMode->clear();
    ui->lblCircleColorRec->clear();
    ui->lblQrCode->clear();
    ui->lblMotion->clear();
    ui->lblFaceRec->clear();
    ui->lblOcr->clear();
}

void Dialog::start_timers()
{
    if (ui->btnEnableDefaultCamMode->text() == "Disable Def Mode")
        mainCam_timer->start(30);
    if (ui->chckBoxCircleColorRec->isChecked())
        circleColorRec_timer->start(30);
    if (ui->chckBoxFaceRec->isChecked())
        faceRec_timer->start(30);
    if (ui->chckBoxMotion->isChecked())
        motion_timer->start(30);
    if (ui->chckBoxQrCode->isChecked())
        qrCode_timer->start(30);
    if (ui->chckBoxOcr->isChecked())
        ocr_timer->start(30);
}

void Dialog::stop_timers()
{
    mainCam_timer->stop();
    circleColorRec_timer->stop();
    qrCode_timer->stop();
    motion_timer->stop();
    faceRec_timer->stop();
    ocr_timer->stop();
}

void Dialog::set_ipVisibility(bool state)
{
    ui->lblIp->setVisible(state);
    ui->txtFirstIpValue->setVisible(state);
    ui->txtSecondIpValue->setVisible(state);
    ui->txtThirdIpValue->setVisible(state);
    ui->txtForthIpValue->setVisible(state);
    ui->btnSetIp->setVisible(state);
}

void Dialog::thingsToDo_after_loadingFaceDatabase()
{
    ui->cmbDatabaseLabels->clear();
    ui->cmbDatabaseLabels->addItem("Select");
    for (size_t i = 0; i < getLabels.size(); i++)
        ui->cmbDatabaseLabels->addItem(QString::fromStdString(getLabels[i]));
    if (resetPBarTrackingQuality)
        ui->pBarTrackingQuality->setValue(0);
}

void Dialog::timer_mainCam()
{
    Mat cap_frame;
    cap >> cap_frame;

    QImage cap_frame_QImage = QImage((uchar*) cap_frame.data, cap_frame.cols, cap_frame.rows, cap_frame.step, QImage::Format_RGB888).rgbSwapped();
    ui->lblDefaultCameraMode->setPixmap(QPixmap::fromImage(cap_frame_QImage));
}

void Dialog::timer_circleColorRec()
{
    Mat cap_frame;
    int hsvValues[] = {ui->sliderLowH->value(), ui->sliderLowS->value(), ui->sliderLowV->value(), ui->sliderHighH->value(), ui->sliderHighS->value(), ui->sliderHighV->value()};

    cap >> cap_frame;

    circlecolorrec.operation(&cap_frame, hsvValues, &getX_circleColorRec, &getY_circleColorRec);

    // ############################## chart ##############################

    // Append to vector

    circleCenter_xVect.push_back(getX_circleColorRec);
    circleCenter_yVect.push_back(getY_circleColorRec);

    circleXmlAppendingFunction();

    // Appending place-x

    if (pointCounter_place_x >= 49)
    {
        for (loopCounter_place_x = pointCounter_place_x; loopCounter_place_x >= 0; loopCounter_place_x--)
        {
            placeSeries_x->replace( QPointF(axisX_counter2_place_x, circleCenter_xVect[loopCounter_place_x - 1]), QPointF(axisX_counter2_place_x, circleCenter_xVect[loopCounter_place_x]) );
            axisX_counter2_place_x -= 20;
        }
    }
    else
    {
      placeSeries_x->append( QPointF(axisX_counter1_place_x, circleCenter_xVect[pointCounter_place_x]) );
      axisX_counter1_place_x += 20;
    }

    // Appending place-y

    if (pointCounter_place_y >= 49)

    {
        for (loopCounter_place_y = pointCounter_place_y; loopCounter_place_y >= 0; loopCounter_place_y--)
        {
            placeSeries_y->replace( QPointF(axisX_counter2_place_y, circleCenter_yVect[loopCounter_place_y - 1]), QPointF(axisX_counter2_place_y, circleCenter_yVect[loopCounter_place_y]) );
            axisX_counter2_place_y -= 20;
        }
    }
    else
    {
      placeSeries_y->append( QPointF(axisX_counter1_place_y, circleCenter_yVect[pointCounter_place_y]) );
      axisX_counter1_place_y += 20;
    }

    pointCounter_place_x++;
    axisX_counter2_place_x = 1000;

    pointCounter_place_y++;
    axisX_counter2_place_y = 1000;

    // ###################################################################

    ui->txtCircleColorRecCoordinate->setText(QString::number(getX_circleColorRec) + "\n" + QString::number(getY_circleColorRec));

    QImage cap_frame_QImage = QImage((uchar*) cap_frame.data, cap_frame.cols, cap_frame.rows, cap_frame.step, QImage::Format_Grayscale8);
    ui->lblCircleColorRec->setPixmap(QPixmap::fromImage(cap_frame_QImage));
}

void Dialog::timer_qrCode()
{
    Mat cap_frame;
    cap >> cap_frame;

    qrcode.operation(&cap_frame, &getCode, &getX_qrCode, &getY_qrCode, &getRateX_qrCode, &getRateY_qrCode, &screenScale);

    if (!getCode.isEmpty())
    {
        ui->lstQrCode->addItem(getCode);
        ui->lstQrCode->scrollToBottom();
        if (lstQrCodeIndexCounter++ % 2 == 0)
            ui->lstQrCode->item(lstQrCodeIndexCounter - 1)->setBackgroundColor(Qt::lightGray);
    }

    ui->txtRateQrCode->setText(QString::number(getRateX_qrCode, 'f', 2) + "\n" + QString::number(getRateY_qrCode, 'f', 2));
    ui->txtQrCodeCoordinate->setText(QString::number(getX_qrCode) + "\n" + QString::number(getY_qrCode));

    QImage cap_frame_QImage = QImage((uchar*) cap_frame.data, cap_frame.cols, cap_frame.rows, cap_frame.step, QImage::Format_RGB888).rgbSwapped();
    ui->lblQrCode->setPixmap(QPixmap::fromImage(cap_frame_QImage));
}

void Dialog::timer_motion()
{
    Mat cap_frame;
    cap >> cap_frame;

    motion.operation(&cap_frame, &getPointsSize_motion, &getRateX_motion, &getRateY_motion, &needToInit);

    // ############################## chart ##############################

    // Append to vector

    x_rate_average.push_back(getRateX_motion);
    y_rate_average.push_back(getRateY_motion);

    motionXmlAppendingFunction();

    // Appending velocity-x

    if (pointCounter_velocity_x >= 49)
    {
        for (loopCounter_velocity_x = pointCounter_velocity_x; loopCounter_velocity_x >= 0; loopCounter_velocity_x--)
        {
            velocitySeries_x->replace( QPointF(axisX_counter2_velocity_x, x_rate_average[loopCounter_velocity_x - 1]), QPointF(axisX_counter2_velocity_x, x_rate_average[loopCounter_velocity_x]) );
            axisX_counter2_velocity_x -= 20;
        }
    }
    else
    {
      velocitySeries_x->append( QPointF(axisX_counter1_velocity_x, x_rate_average[pointCounter_velocity_x]) );
      axisX_counter1_velocity_x += 20;
    }

    // Appending velocity-y

    if (pointCounter_velocity_y >= 49)
    {
        for (loopCounter_velocity_y = pointCounter_velocity_y; loopCounter_velocity_y >= 0; loopCounter_velocity_y--)
        {
            velocitySeries_y->replace( QPointF(axisX_counter2_velocity_y,y_rate_average[loopCounter_velocity_y - 1]), QPointF(axisX_counter2_velocity_y, y_rate_average[loopCounter_velocity_y]) );
            axisX_counter2_velocity_y -= 20;
        }
    }
    else
    {
      velocitySeries_y->append( QPointF(axisX_counter1_velocity_y, y_rate_average[pointCounter_velocity_y]) );
      axisX_counter1_velocity_y += 20;
    }

    pointCounter_velocity_x++;
    axisX_counter2_velocity_x = 1000;

    pointCounter_velocity_y++;
    axisX_counter2_velocity_y = 1000;

    // ###################################################################

    ui->txtRateMotion->setText(QString::number(getPointsSize_motion) + "\n" + QString::number(getRateX_motion, 'f', 2) + "\n" + QString::number(getRateY_motion, 'f', 2));

    QImage cap_frame_QImage = QImage((uchar*) cap_frame.data, cap_frame.cols, cap_frame.rows, cap_frame.step, QImage::Format_RGB888).rgbSwapped();
    ui->lblMotion->setPixmap(QPixmap::fromImage(cap_frame_QImage));
}

void Dialog::timer_faceRec()
{
    Mat cap_frame;
    cap >> cap_frame;

    facerec.operation(&cap_frame, rec_threshold, tracking_quality_threshold, skip_frames, skip_frames_counter++, &getTrackersSize, &tracking_quality_sum, cap);

    if (getTrackersSize != 0)
    {
        ui->pBarTrackingQuality->setValue((int)tracking_quality_sum / getTrackersSize);
        tracking_quality_sum = 0;
    }
    else
        ui->pBarTrackingQuality->setValue(0);

    QImage cap_frame_QImage = QImage((uchar*) cap_frame.data, cap_frame.cols, cap_frame.rows, cap_frame.step, QImage::Format_RGB888).rgbSwapped();
    ui->lblFaceRec->setPixmap(QPixmap::fromImage(cap_frame_QImage));
}

void Dialog::timer_ocr()
{
    // Ocr Codes ...
}

void Dialog::timer_refresher()
{
    ba[3] = 0x00;
    refresher_timer->stop();

    Dialog::setEnabled(true);
}

void Dialog::timer_serialWrite()
{
    if (ui->chckBoxControl->isChecked())
    {
        ba_sum = 0;
        for (int i = 1; i < ba.size() - 2; i++)
            ba_sum += ba[i];

        ba[8] = ba_sum;
        ba[9] = ba[0] + ba[8];

        if (send_counter < ba.size())
            serial.write(&ba.data()[send_counter++], 1);
        else
            send_counter = 0;
    }
    else if (!ui->chckBoxControl->isChecked())
    {
        if (serial_counter < 3000)
        {
            ba_sum = 0;
            for (int i = 1; i < ba.size() - 2; i++)
                ba_sum += ba[i];

            ba[29] = ba_sum;
            ba[30] = ba[0] + ba[29];

            if (send_counter < ba.size())
                serial.write(&ba.data()[send_counter++], 1);
            else
                send_counter = 0;

            serial_counter++;
        }
        else if (serial_counter >= 3000 && serial_counter < 4000)
        {
            ba_log[0] = 0x55;
            ba_log[1] = 0x00;
            ba_log[2] = 0x00;
            ba_log[3] = 0x00;
            ba_log[4] = 0x00;
            ba_log[5] = 0x00;
            ba_log[6] = 0x55;

            if (send_counter < ba_log.size())
                serial.write(&ba_log.data()[send_counter++], 1);
            else
                send_counter = 0;

            serial_counter++;
        }
        else
        {
            serialWrite_timer->stop();
            serial_counter = 0;
        }
    }
    else if (ui->chckBoxLog->isChecked())
    {
        if (send_counter < ba_log.size())
            serial.write(&ba_log.data()[send_counter++], 1);
        else
            send_counter = 0;
    }
}

void Dialog::allocate()
{
//    uchar chck_sum_A = 0;

//    for (int i = 1; i < 12; i++)
//        chck_sum_A += recieve[i];

//    uchar chck_sum_B = chck_sum_A + (uchar)recieve[0];


//    cout << (int)(uchar)chck_sum_A << ":" << (int)(uchar)chck_sum_B << endl;

//    if (chck_sum_A == ba[(int)ba[1] + 4] && chck_sum_B == ba[(int)ba[1] + 5])
//        cout << "qoli" << endl;


    if ((uchar)recieve[11] == 0xA5 && (uchar)recieve[12] == 0xAA)
    {
        uint16_t c_x = ((uchar)recieve[4] * 256) + (uchar)recieve[5];
        uint16_t c_y = ((uchar)recieve[6] * 256) + (uchar)recieve[7];
        uint16_t height = (uchar)(recieve[8]);
        //int16_t pid_out = ( ((int16_t)recieve[9] << 8) & 0xFF ) & (int16_t)recieve[10];
        int16_t pid_out = ((uint8_t)recieve[9] * 256) + (uint8_t)recieve[10];

        ui->txtRecievedData->append(QString::number(c_x) + ", " + QString::number(c_y) + ", " + QString::number(pid_out));
        ui->txtShowHeight->setText(QString::number(height));
    }
}

//uchar recieve_int[50];
//QString output;
void Dialog::read_data()
{
    recieve.clear();
    recieve.append(serial.readAll());

    if ((uchar)recieve[0] == 0x55)
        allocate();

//    output.clear();

//    for (int i = 0; i < recieve.size(); i++)
//    {
//        recieve_int[i] = recieve[i];
//        output += QString::number(recieve_int[i]) + ", ";
//    }

//    ui->txtRecievedData->append(output);
}

void Dialog::set_cap()
{
    if ( (!cap.isOpened() && ui->cmbCapMode->currentIndex() != 2) || set_cap_check )
    {
        set_cap_check = false;

        if (cap.open(ui->cmbCapMode->currentIndex()))
        {
            start_timers();
            is_cap_open = true;
        }
        else
            is_cap_open = false;
    }
}

void Dialog::on_btnEnableDefaultCamMode_clicked()
{
    if (ui->btnEnableDefaultCamMode->text() == "Enable Def Mode")
    {
        if (!cap.isOpened() && ui->cmbCapMode->currentIndex() != 2)
            cap.open(ui->cmbCapMode->currentIndex());

        ui->btnEnableDefaultCamMode->setText("Disable Def Mode");

        ui->btnPause->setEnabled(true);

        ui->tabWidgetCamera->setCurrentIndex(0);
        show_priority.push_back(ui->tabWidgetCamera->currentIndex());

        mainCam_timer->start(30);
    }
    else if (ui->btnEnableDefaultCamMode->text() == "Disable Def Mode")
    {
        ui->btnEnableDefaultCamMode->setText("Enable Def Mode");

        mainCam_timer->stop();

        show_priority.remove(0);
        auto it = show_priority.begin();
        advance(it, show_priority.size() - 1);
        ui->tabWidgetCamera->setCurrentIndex(*it);

        if (!ui->chckBoxQrCode->isChecked() && !ui->chckBoxCircleColorRec->isChecked() && !ui->chckBoxMotion->isChecked() && !ui->chckBoxFaceRec->isChecked() && !ui->chckBoxOcr->isChecked())
        {
            cap.release();
            ui->btnPause->setEnabled(false);
        }

        ui->lblDefaultCameraMode->clear();
        ui->lblDefaultCameraMode->setStyleSheet("background-image: url(FormBackgrounds/defaultmode-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    }
}

void Dialog::on_btnPause_clicked()
{
    if (ui->btnPause->text() == "Pause")
    {
        ui->btnPause->setText("Play");

        ui->btnSaveByTakingPhoto->setEnabled(false);
        ui->btnMotion->setEnabled(false);

        stop_timers();
    }
    else if (ui->btnPause->text() == "Play")
    {
        ui->btnPause->setText("Pause");

        start_timers();
    }
}

void Dialog::on_btnDisconnect_clicked()
{
    ui->btnMotion->setEnabled(false);

    ui->cmbDefaultColors->setCurrentIndex(0);
    on_cmbDefaultColors_activated(0);

    ui->btnSaveByTakingPhoto->setEnabled(false);
    ui->pBarTrackingQuality->setValue(0);

    ui->sbDistance->setValue(0);

    //on_btnClear_clicked();
    //on_btnClearTxtRecievedData_clicked();
    //on_btnResetFaceRecSettings_clicked();

    ui->chckBoxQrCode->setCheckState(Qt::Unchecked);
    ui->chckBoxCircleColorRec->setCheckState(Qt::Unchecked);
    ui->chckBoxMotion->setCheckState(Qt::Unchecked);
    ui->chckBoxFaceRec->setCheckState(Qt::Unchecked);
    ui->chckBoxLoadDatabase->setCheckState(Qt::Unchecked);
    ui->chckBoxOcr->setCheckState(Qt::Unchecked);

    ui->btnEnableDefaultCamMode->setText("Enable Def Mode");

    ui->btnPause->setEnabled(false);
    ui->btnDisconnect->setEnabled(false);

    stop_timers();
    cap.release();

    clear_backgroundImages();
    load_backgroundImages();
}

void Dialog::on_btnMotion_clicked()
{
    needToInit = true;
}

void Dialog::on_cmbDefaultColors_activated(int index)
{
    switch (index)
    {
    case 0:
        ui->chckBoxCircleColorRec->setCheckState(Qt::Checked);

        ui->sliderLowH->setValue(0);
        ui->sliderLowS->setValue(0);
        ui->sliderLowV->setValue(0);

        ui->sliderHighH->setValue(0);
        ui->sliderHighS->setValue(0);
        ui->sliderHighV->setValue(0);

        ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");

        break;

    case 1:
        // ----------- Red -----------
        ui->chckBoxCircleColorRec->setCheckState(Qt::Checked);

        ui->sliderLowH->setValue(160);
        ui->sliderLowS->setValue(100);
        ui->sliderLowV->setValue(100);

        ui->sliderHighH->setValue(179);
        ui->sliderHighS->setValue(255);
        ui->sliderHighV->setValue(255);

        ui->cmbDefaultColors->setStyleSheet("border: 2px solid red; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");

        break;

    case 2:
        // ----------- Blue -----------
        ui->chckBoxCircleColorRec->setCheckState(Qt::Checked);

        ui->sliderLowH->setValue(110);
        ui->sliderLowS->setValue(50);
        ui->sliderLowV->setValue(50);

        ui->sliderHighH->setValue(130);
        ui->sliderHighS->setValue(255);
        ui->sliderHighV->setValue(255);

        ui->cmbDefaultColors->setStyleSheet("border: 2px solid blue; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");

        break;

    case 3:
        // ----------- Yellow -----------
        ui->chckBoxCircleColorRec->setCheckState(Qt::Checked);

        ui->sliderLowH->setValue(20);
        ui->sliderLowS->setValue(100);
        ui->sliderLowV->setValue(100);

        ui->sliderHighH->setValue(30);
        ui->sliderHighS->setValue(256);
        ui->sliderHighV->setValue(256);

        ui->cmbDefaultColors->setStyleSheet("border: 2px solid yellow; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");

        break;

    default:
        break;
    }
}

void Dialog::on_sliderLowH_sliderPressed()
{
    ui->cmbDefaultColors->setCurrentIndex(0);
    ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");
}

void Dialog::on_sliderLowS_sliderPressed()
{
    ui->cmbDefaultColors->setCurrentIndex(0);
    ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");
}

void Dialog::on_sliderLowV_sliderPressed()
{
    ui->cmbDefaultColors->setCurrentIndex(0);
    ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");
}

void Dialog::on_sliderHighH_sliderPressed()
{
    ui->cmbDefaultColors->setCurrentIndex(0);
    ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");
}

void Dialog::on_sliderHighS_sliderPressed()
{
    ui->cmbDefaultColors->setCurrentIndex(0);
    ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");
}

void Dialog::on_sliderHighV_sliderPressed()
{
    ui->cmbDefaultColors->setCurrentIndex(0);
    ui->cmbDefaultColors->setStyleSheet("border: 2px solid #000; color: #000; font-style: italic; padding-left: 15px; border-radius: 0; border-bottom: none;");
}

void Dialog::on_chckBoxQrCode_stateChanged()
{
    if (ui->chckBoxQrCode->isChecked())
    {
        if (!cap.isOpened())
            set_cap();

        if (!ui->btnDisconnect->isEnabled() || !ui->btnPause->isEnabled())
        {
            ui->btnDisconnect->setEnabled(true);
            ui->btnPause->setEnabled(true);
        }

        ui->tabWidgetCamera->setCurrentIndex(2);
        show_priority.push_back(ui->tabWidgetCamera->currentIndex());

        ui->btnMotion->setEnabled(false);

        if (is_cap_open)
            qrCode_timer->start(30);
    }
    else
    {
        qrCode_timer->stop();

        show_priority.remove(2);
        auto it = show_priority.begin();
        advance(it, show_priority.size() - 1);
        ui->tabWidgetCamera->setCurrentIndex(*it);

        getX_qrCode = getY_qrCode = -1;
        getRateX_qrCode = getRateY_qrCode = 0;
        ui->txtRateQrCode->setText("0.00\n0.00");
        ui->txtQrCodeCoordinate->setText("-1\n-1");

        ui->lblQrCode->clear();
        ui->lblQrCode->setStyleSheet("background-image: url(FormBackgrounds/qrcode-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    }

    if (!ui->chckBoxQrCode->isChecked() && !ui->chckBoxCircleColorRec->isChecked() && !ui->chckBoxMotion->isChecked() && !ui->chckBoxFaceRec->isChecked() && !ui->chckBoxOcr->isChecked() && ui->btnEnableDefaultCamMode->text() == "Enable Def Mode")
    {
        stop_timers();
        cap.release();

        set_cap_check = true;
    }
}

void Dialog::on_chckBoxCircleColorRec_stateChanged()
{
    if (ui->chckBoxCircleColorRec->isChecked())
    {
        if (!cap.isOpened())
            set_cap();

        if (!ui->btnDisconnect->isEnabled() || !ui->btnPause->isEnabled())
        {
            ui->btnDisconnect->setEnabled(true);
            ui->btnPause->setEnabled(true);
        }

        ui->tabWidgetCamera->setCurrentIndex(1);
        show_priority.push_back(ui->tabWidgetCamera->currentIndex());

        placeSeries_x->show();
        placeSeries_y->show();

        if (is_cap_open)
            circleColorRec_timer->start(30);
    }
    else
    {
        circleColorRec_timer->stop();

        show_priority.remove(1);
        auto it = show_priority.begin();
        advance(it, show_priority.size() - 1);
        ui->tabWidgetCamera->setCurrentIndex(*it);

        getX_circleColorRec = getY_circleColorRec = -1;
        ui->txtCircleColorRecCoordinate->setText("-1\n-1");

        placeSeries_x->hide();
        placeSeries_y->hide();

        ui->lblCircleColorRec->clear();
        ui->lblCircleColorRec->setStyleSheet("background-image: url(FormBackgrounds/circlecolorrec-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    }

    if (!ui->chckBoxQrCode->isChecked() && !ui->chckBoxCircleColorRec->isChecked() && !ui->chckBoxMotion->isChecked() && !ui->chckBoxFaceRec->isChecked() && !ui->chckBoxOcr->isChecked() && ui->btnEnableDefaultCamMode->text() == "Enable Def Mode")
    {
        stop_timers();
        cap.release();

        set_cap_check = true;
    }
}

void Dialog::on_chckBoxMotion_stateChanged()
{
    if (ui->chckBoxMotion->isChecked())
    {
        if (!cap.isOpened())
            set_cap();

        if (!ui->btnDisconnect->isEnabled() || !ui->btnPause->isEnabled())
        {
            ui->btnDisconnect->setEnabled(true);
            ui->btnPause->setEnabled(true);
        }

        ui->tabWidgetCamera->setCurrentIndex(3);
        show_priority.push_back(ui->tabWidgetCamera->currentIndex());

        on_btnMotion_clicked();
        ui->btnMotion->setEnabled(true);

        velocitySeries_x->show();
        velocitySeries_y->show();

        if (is_cap_open)
            motion_timer->start(30);
    }
    else
    {
        motion_timer->stop();

        show_priority.remove(3);
        auto it = show_priority.begin();
        advance(it, show_priority.size() - 1);
        ui->tabWidgetCamera->setCurrentIndex(*it);

        ui->btnMotion->setEnabled(false);

        getPointsSize_motion = getRateX_motion = getRateY_motion = 0;
        ui->txtRateMotion->setText("0\n0.00\n0.00");

        velocitySeries_x->hide();
        velocitySeries_y->hide();

        ui->lblMotion->clear();
        ui->lblMotion->setStyleSheet("background-image: url(FormBackgrounds/motion-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    }

    if (!ui->chckBoxQrCode->isChecked() && !ui->chckBoxCircleColorRec->isChecked() && !ui->chckBoxMotion->isChecked() && !ui->chckBoxFaceRec->isChecked() && !ui->chckBoxOcr->isChecked() && ui->btnEnableDefaultCamMode->text() == "Enable Def Mode")
    {
        stop_timers();
        cap.release();

        set_cap_check = true;
    }
}

void Dialog::on_chckBoxFaceRec_stateChanged()
{
    if (ui->chckBoxFaceRec->isChecked())
    {
        if (!cap.isOpened())
            set_cap();

        if (!ui->btnDisconnect->isEnabled() || !ui->btnPause->isEnabled())
        {
            ui->btnDisconnect->setEnabled(true);
            ui->btnPause->setEnabled(true);
        }

        ui->tabWidgetCamera->setCurrentIndex(4);
        show_priority.push_back(ui->tabWidgetCamera->currentIndex());

        ui->btnSaveByTakingPhoto->setEnabled(true);

        if (is_cap_open)
            faceRec_timer->start(30);
    }
    else
    {
        faceRec_timer->stop();

        show_priority.remove(4);
        auto it = show_priority.begin();
        advance(it, show_priority.size() - 1);
        ui->tabWidgetCamera->setCurrentIndex(*it);

        ui->chckBoxLoadDatabase->setCheckState(Qt::Unchecked);

        ui->btnSaveByTakingPhoto->setEnabled(false);
        ui->pBarTrackingQuality->setValue(0);
        on_btnResetFaceRecSettings_clicked();

        ui->lblFaceRec->clear();
        ui->lblFaceRec->setStyleSheet("background-image: url(FormBackgrounds/facerecognition-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    }

    if (!ui->chckBoxQrCode->isChecked() && !ui->chckBoxCircleColorRec->isChecked() && !ui->chckBoxMotion->isChecked() && !ui->chckBoxFaceRec->isChecked() && !ui->chckBoxOcr->isChecked() && ui->btnEnableDefaultCamMode->text() == "Enable Def Mode")
    {
        stop_timers();
        cap.release();

        set_cap_check = true;
    }
}

void Dialog::on_chckBoxOcr_stateChanged()
{
    if (ui->chckBoxOcr->isChecked())
    {
        if (!cap.isOpened())
            set_cap();

        if (!ui->btnDisconnect->isEnabled() || !ui->btnPause->isEnabled())
        {
            ui->btnDisconnect->setEnabled(true);
            ui->btnPause->setEnabled(true);
        }

        ui->tabWidgetCamera->setCurrentIndex(5);
        show_priority.push_back(ui->tabWidgetCamera->currentIndex());

        if (is_cap_open)
            ocr_timer->start(30);
    }
    else
    {
        ocr_timer->stop();

        show_priority.remove(5);
        auto it = show_priority.begin();
        advance(it, show_priority.size() - 1);
        ui->tabWidgetCamera->setCurrentIndex(*it);

        ui->lblOcr->clear();
        ui->lblOcr->setStyleSheet("background-image: url(FormBackgrounds/ocr-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
    }

    if (!ui->chckBoxQrCode->isChecked() && !ui->chckBoxCircleColorRec->isChecked() && !ui->chckBoxMotion->isChecked() && !ui->chckBoxFaceRec->isChecked() && !ui->chckBoxOcr->isChecked() && ui->btnEnableDefaultCamMode->text() == "Enable Def Mode")
    {
        stop_timers();
        cap.release();

        set_cap_check = true;
    }
}

void Dialog::on_sbDistance_valueChanged(int arg1)
{
    screenScale = 33 * ((float)arg1 / 40);
}

void Dialog::on_hSliderRecThreshold_valueChanged(int value)
{
    ui->lcdNumberRecThreshold->display((double)value / 100);
    rec_threshold = (double)value / 100;
}

void Dialog::on_hSliderTrackingQualityThreshold_valueChanged(int value)
{
    ui->lcdNumberTrackingQualityThreshold->display((double)value / 100);
    tracking_quality_threshold = (double)value / 100;
}

void Dialog::on_sbSkipFrames_valueChanged(int value)
{
    skip_frames = value;
    skip_frames_counter = 0;
}

void Dialog::on_txtFirstIpValue_textChanged()
{
    firstIpValue = ui->txtFirstIpValue->toPlainText().toStdString();

    ui->txtFirstIpValue->setCursorWidth(1);

    if (ui->txtFirstIpValue->toPlainText().length() == 3)
    {
        ui->txtFirstIpValue->setCursorWidth(0);
        ui->txtSecondIpValue->setFocus();
    }
}

void Dialog::on_txtSecondIpValue_textChanged()
{
    secondIpValue = ui->txtSecondIpValue->toPlainText().toStdString();

    ui->txtSecondIpValue->setCursorWidth(1);

    if (ui->txtSecondIpValue->toPlainText().length() == 3)
    {
        ui->txtSecondIpValue->setCursorWidth(0);
        ui->txtThirdIpValue->setFocus();
    }
}

void Dialog::on_txtThirdIpValue_textChanged()
{
    thirdIpValue = ui->txtThirdIpValue->toPlainText().toStdString();

    ui->txtThirdIpValue->setCursorWidth(1);

    if (ui->txtThirdIpValue->toPlainText().length() == 1)
    {
        ui->txtThirdIpValue->setCursorWidth(0);
        ui->txtForthIpValue->setFocus();
    }
}

void Dialog::on_txtForthIpValue_textChanged()
{
    forthIpValue = ui->txtForthIpValue->toPlainText().toStdString();
}

void Dialog::on_btnSetIp_clicked()
{
    if (!firstIpValue.empty() && !secondIpValue.empty() && !thirdIpValue.empty() && !forthIpValue.empty())
    {
        ip = ip_firstPart + firstIpValue + "." + secondIpValue + "." + thirdIpValue + "." + forthIpValue + ip_lastPart;
        cap.open(ip);
        set_cap();
    }
    else
        cerr << "Fill ip values !" << endl;
}

void Dialog::on_cmbCapMode_activated(int index)
{
    clear_backgroundImages();
    load_backgroundImages();

    if (cap.isOpened())
    {
        stop_timers();
        cap.release();
    }

    if (index == 2)
    {
        set_ipVisibility(true);
        ui->txtFirstIpValue->setFocus();
    }
    else
    {
        set_ipVisibility(false);
        set_cap();
    }
}

void Dialog::onEnteredTextSent(const QString &text)
{
    facerec.detected_person_name_from_photo.push_back(text.toStdString());
}

void Dialog::show_save_window()
{
    SaveNewFace *savenewface;
    savenewface = new SaveNewFace(this);
    connect(savenewface, SIGNAL(notifyEnteredTextSent(QString)), this, SLOT(onEnteredTextSent(QString)));

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - savenewface->width()) / 2;
    int y = (screenGeometry.height() - savenewface->height()) / 2;

    savenewface->move(x, y);
    savenewface->setFixedSize(second_window_width, second_window_height);
    savenewface->setModal(true);
    savenewface->exec();
}

void Dialog::on_btnSaveByImagesAdd_clicked()
{
    facerec.btnSaveByImagesPath_function(ui->txtImagesPath->toPlainText(), &getLabels, cap, &resetPBarTrackingQuality);
    thingsToDo_after_loadingFaceDatabase();
}

void Dialog::on_btnSaveByTakingPhoto_clicked()
{
    Mat cap_frame;
    cap >> cap_frame;

    facerec.btnSaveByTakingPhoto_function(cap_frame, &getLabels, cap, &resetPBarTrackingQuality);
    thingsToDo_after_loadingFaceDatabase();
}

void Dialog::on_btnDeleteFromDatabase_clicked()
{
    facerec.btnDeleteFromDatabase_function(ui->cmbDatabaseLabels->currentIndex(), &getLabels, cap, &resetPBarTrackingQuality);
    thingsToDo_after_loadingFaceDatabase();
}

void Dialog::on_chckBoxLoadDatabase_stateChanged(int state)
{
    if (state == 0)
        facerec.database_face_descriptors.clear();
    else
    {
        facerec.load_database(&getLabels, cap, &resetPBarTrackingQuality);
        thingsToDo_after_loadingFaceDatabase();
    }
}

void Dialog::on_btnClear_clicked()
{
    ui->lstQrCode->clear();

    getX_circleColorRec = getY_circleColorRec = -1;
    ui->txtCircleColorRecCoordinate->setText("-1\n-1");

    getPointsSize_motion = getRateX_motion = getRateY_motion = 0;
    ui->txtRateMotion->setText("0\n0.00\n0.00");

    getX_qrCode = getY_qrCode = -1;
    getRateX_qrCode = getRateY_qrCode = 0;
    ui->txtRateQrCode->setText("0.00\n0.00");
    ui->txtQrCodeCoordinate->setText("-1\n-1");
}

void Dialog::on_btnResetFaceRecSettings_clicked()
{
    skip_frames = 10;
    ui->sbSkipFrames->setValue(10);

    rec_threshold = 0.52;
    ui->hSliderRecThreshold->setValue(52);

    tracking_quality_threshold = 5;
    ui->hSliderTrackingQualityThreshold->setValue(500);
}

void Dialog::on_dsbPRoll_valueChanged(double arg1)
{
    temp = arg1 * 100;
    ba[4] = temp / 256;
    ba[5] = temp % 256;
}

void Dialog::on_dsbIRoll_valueChanged(double arg1)
{
    temp = arg1 * 1000;
    ba[6] = temp / 256;
    ba[7] = temp % 256;
}

void Dialog::on_dsbDRoll_valueChanged(double arg1)
{
    temp = arg1 * 100;
    ba[8] = temp / 256;
    ba[9] = temp % 256;
}

void Dialog::on_dsbPPitch_valueChanged(double arg1)
{
    temp = arg1 * 100;
    ba[10] = temp / 256;
    ba[11] = temp % 256;
}

void Dialog::on_dsbIPitch_valueChanged(double arg1)
{
    temp = arg1 * 1000;
    ba[12] = temp / 256;
    ba[13] = temp % 256;
}

void Dialog::on_dsbDPitch_valueChanged(double arg1)
{
    temp = arg1 * 100;
    ba[14] = temp / 256;
    ba[15] = temp % 256;
}

void Dialog::on_dsbPAlt_valueChanged(double arg1)
{
    temp = arg1 * 100;
    ba[16] = temp / 256;
    ba[17] = temp % 256;
}

void Dialog::on_dsbIAlt_valueChanged(double arg1)
{
    temp = arg1 * 1000;
    ba[18] = temp / 256;
    ba[19] = temp % 256;
}

void Dialog::on_dsbDAlt_valueChanged(double arg1)
{
    temp = arg1 * 100;
    ba[20] = temp / 256;
    ba[21] = temp % 256;
}

void Dialog::on_sbHeight_valueChanged(int arg1)
{
    ba[22] = arg1;
}

void Dialog::on_sbReserve1_1_valueChanged(int arg1)
{
    ba[23] = arg1;
}

void Dialog::on_sbReserve1_2_valueChanged(int arg1)
{
    ba[24] = arg1;
}

void Dialog::on_sbReserve1_3_valueChanged(int arg1)
{
    ba[25] = arg1;
}

void Dialog::on_sbReserve2_1_valueChanged(int arg1)
{
    ba[26] = arg1;
}

void Dialog::on_sbReserve2_2_valueChanged(int arg1)
{
    ba[27] = arg1;
}

void Dialog::on_sbReserve2_3_valueChanged(int arg1)
{
    ba[28] = arg1;
}

void Dialog::on_btnRead_clicked()
{
    ba[3] = 0x02;

    serialWrite_timer->start(1);
    refresher_timer->start(3000);
    
    Dialog::setEnabled(false);
}

void Dialog::on_btnWrite_clicked()
{
    ba[3] = 0x01;

    serialWrite_timer->start(1);
    refresher_timer->start(3000);

    Dialog::setEnabled(false);
}

void Dialog::on_btnComConnect_clicked()
{
    if (!serial.isOpen() && ui->cmbCom->currentIndex() > 0 && ui->cmbBaudrate->currentIndex() > 0)
    {
        serial.setPortName(ui->cmbCom->currentText());
        serial.setBaudRate(ui->cmbBaudrate->currentText().toInt());
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);

        if (serial.open(QIODevice::ReadWrite))
        {
            serial.clear();
            
            ui->pBarLoadComConnect->setValue(ui->pBarLoadComConnect->maximum() / 2);
            QThread::msleep(300);
            ui->pBarLoadComConnect->setValue(ui->pBarLoadComConnect->maximum());

            ui->pBarLoadComConnect->setFormat("connected");
            ui->cmbCom->setEnabled(false);
            ui->cmbBaudrate->setEnabled(false);
            ui->tabWidgetOthers->setCurrentIndex(2);
        }
    }
}

void Dialog::on_btnComDisconnect_clicked()
{
    if (serial.isOpen())
    {
        serial.close();

        ui->pBarLoadComConnect->setValue(ui->pBarLoadComConnect->minimum());
        ui->pBarLoadComConnect->setFormat("disconnected");

        ui->cmbCom->setEnabled(true);
        ui->cmbBaudrate->setEnabled(true);

        if (serialWrite_timer->isActive())
            serialWrite_timer->stop();
    }
}

void Dialog::on_btnComRefresh_clicked()
{
    ui->cmbCom->clear();
    ui->cmbCom->addItem("Select");
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->cmbCom->addItem(info.portName());
}

void Dialog::on_btnClearTxtRecievedData_clicked()
{
    ui->txtRecievedData->clear();
}

void Dialog::on_btnFreezeTxtRecievedData_clicked()
{
    if (ui->pBarLoadComConnect->text() == "connected")
    {
        if (ui->btnFreezeTxtRecievedData->text() == "Freeze")
        {
            ui->btnFreezeTxtRecievedData->setText("Unfreeze");
            disconnect(&serial, SIGNAL(readyRead()), this, SLOT(read_data()));
        }
        else if (ui->btnFreezeTxtRecievedData->text() == "Unfreeze")
        {
            ui->btnFreezeTxtRecievedData->setText("Freeze");
            connect(&serial, SIGNAL(readyRead()), this, SLOT(read_data()));
        }
    }
}

void Dialog::on_chckBoxControl_stateChanged()
{
    if (ui->chckBoxControl->isChecked())
    {
        ba.resize(10);
        ba[0] = 0x55;
        ba[1] = 4;
        ba[2] = 0x02;
        ba[3] = 0x03;

        if (circleColorRec_timer->isActive())
        {
            temp = getX_circleColorRec * 100;
            ba[4] = temp / 256;
            ba[5] = temp % 256;

            temp = getY_circleColorRec * 100;
            ba[6] = temp / 256;
            ba[7] = temp % 256;
        }
        else
        {
            for (int i = 4; i < ba.size() - 2; i++)
                ba[i] = 0;
        }

        ui->btnRead->setEnabled(false);
        ui->btnWrite->setEnabled(false);
    }
    else
    {
        ba.resize(31);
        ba[0] = 0x55;
        ba[1] = 25;
        ba[2] = 0x01;
        ba[3] = 0x00;

        for (int i = 4; i < ba.size() - 2; i++)
            ba[i] = 0;

        ui->btnRead->setEnabled(true);
        ui->btnWrite->setEnabled(true);
    }
}

void Dialog::on_chckBoxLog_stateChanged()
{
    ba_log[0] = 0x55;
    ba_log[1] = 0x01;
    ba_log[2] = 0x01;
    ba_log[3] = 0x04;
    ba_log[4] = 0x5A;
    ba_log[5] = 0x60;
    ba_log[6] = 0xB5;

    if (ui->chckBoxLog->isChecked())
        serialWrite_timer->start(1);
    else
        serialWrite_timer->stop();
}

void Dialog::x_chartDisplay()
{
    velocitySeries_x->setName("Velocity-X");
    placeSeries_x->setName("Place-X");

    // Appending first point

    velocitySeries_x->append(QPointF(0, 0));
    placeSeries_x->append(QPointF(0, 0));

    // Add series to chart

    chart_x->addSeries(placeSeries_x);
    chart_x->addSeries(velocitySeries_x);

    chart_x->setVisible(true);
    chart_x->legend()->setAlignment(Qt::AlignTrailing);

    // Customize series

    QPen velocitySeriesPen(QRgb(0x105C33));
    QPen translationSeriesPen(QRgb(0x95800F));

    velocitySeriesPen.setWidth(3);
    translationSeriesPen.setWidth(3);

    velocitySeries_x->setPen(velocitySeriesPen);
    placeSeries_x->setPen(translationSeriesPen);

    // Customize chart title

    font.setPixelSize(18);

    chart_x->setTitleFont(font);
    chart_x->setTitleBrush(QBrush(QRgb(0xBCA8C2)));
    chart_x->setTitle("Chart-X");

    // Customize chart background

    backgroundGradient.setStart(QPointF(0, 0));
    backgroundGradient.setFinalStop(QPointF(0, 1));
    backgroundGradient.setColorAt(0.2, QRgb(0x201523));
    backgroundGradient.setColorAt(0.5, QRgb(0x2D2130));
    backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    chart_x->setBackgroundBrush(backgroundGradient);

    // Customize axis label font

    labelsFont.setPixelSize(12);

    axisX_chartX->setLabelsFont(labelsFont);
    axisY_place_chartX->setLabelsFont(labelsFont);
    axisY_velocity_chartX->setLabelsFont(labelsFont);

    // Customize axis colors

    QPen axisPen(QRgb(0x141414));
    axisPen.setWidth(2);

    axisX_chartX->setLinePen(axisPen);
    axisY_place_chartX->setLinePen(axisPen);
    axisY_velocity_chartX->setLinePen(axisPen);

    // Customize axis label colors

    QBrush axisBrush(Qt::white);

    axisX_chartX->setLabelsBrush(axisBrush);
    axisY_place_chartX->setLabelsBrush(axisBrush);
    axisY_velocity_chartX->setLabelsBrush(axisBrush);

    // Customize grid lines and shades

    axisX_chartX->setGridLineVisible(false);
    axisY_place_chartX->setGridLineVisible(false);
    axisY_velocity_chartX->setGridLineVisible(false);

    axisX_chartX->setRange(0, 1000);
    axisY_place_chartX->setRange(-600, 600);
    axisY_velocity_chartX->setRange(-1, 1);

    axisY_place_chartX->append("200", -600);
    axisY_place_chartX->append("400", 0);
    axisY_place_chartX->append("600", 600);

    axisY_velocity_chartX->append("-1", -1);
    axisY_velocity_chartX->append("0", 0);
    axisY_velocity_chartX->append("1", 1);

    // Setting Axisx and Axisy

    chart_x->setAxisX(axisX_chartX, velocitySeries_x);
    chart_x->setAxisY(axisY_velocity_chartX, velocitySeries_x);

    chart_x->setAxisX(axisX_chartX, placeSeries_x);
    chart_x->setAxisY(axisY_place_chartX, placeSeries_x);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart_x);
    chartView->setRenderHint(QPainter::Antialiasing);
    gridLayout_xChart->addWidget(chartView, 0, 0);
}

void Dialog::y_chartDiplay()
{
    velocitySeries_y->setName("Velocity-Y");
    placeSeries_y->setName("Place-Y");

    //Appending first point

    velocitySeries_y->append(QPointF(0, 0));
    placeSeries_y->append(QPointF(0, 0));

    //Add series to chart

    chart_y->addSeries(placeSeries_y);
    chart_y->addSeries(velocitySeries_y);

    chart_y->setVisible(true);
    chart_y->legend()->setAlignment(Qt::AlignTrailing);

    // Customize series

    QPen velocitySeriesPen(QRgb(0x105C33));
    QPen translationSeriesPen(QRgb(0x95800F));

    velocitySeriesPen.setWidth(3);
    translationSeriesPen.setWidth(3);

    velocitySeries_y->setPen(velocitySeriesPen);
    placeSeries_y->setPen(translationSeriesPen);

    // Customize chart title

    font.setPixelSize(18);

    chart_y->setTitleFont(font);
    chart_y->setTitleBrush(QBrush(QRgb(0xBCA8C2)));
    chart_y->setTitle("Chart-Y");

    // Customize chart background

    backgroundGradient.setStart(QPointF(0, 0));
    backgroundGradient.setFinalStop(QPointF(0, 1));
    backgroundGradient.setColorAt(0.2, QRgb(0x201523));
    backgroundGradient.setColorAt(0.5, QRgb(0x2D2130));
    backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    chart_y->setBackgroundBrush(backgroundGradient);

    // Customize axis label font

    labelsFont.setPixelSize(12);

    axisX_chartY->setLabelsFont(labelsFont);
    axisY_place_chartY->setLabelsFont(labelsFont);
    axisY_velocity_chartY->setLabelsFont(labelsFont);

    // Customize axis colors

    QPen axisPen(QRgb(0x141414));
    axisPen.setWidth(2);

    axisX_chartY->setLinePen(axisPen);
    axisY_place_chartY->setLinePen(axisPen);
    axisY_velocity_chartY->setLinePen(axisPen);


    // Customize axis label colors

    QBrush axisBrush(Qt::white);

    axisX_chartY->setLabelsBrush(axisBrush);
    axisY_place_chartY->setLabelsBrush(axisBrush);
    axisY_velocity_chartY->setLabelsBrush(axisBrush);

    // Customize grid lines and shades

    axisX_chartY->setGridLineVisible(false);
    axisY_place_chartY->setGridLineVisible(false);
    axisY_velocity_chartY->setGridLineVisible(false);

    axisX_chartY->setRange(0, 1000);
    axisY_place_chartY->setRange(0, 600);
    axisY_velocity_chartY->setRange(-1, 1);

    axisY_place_chartY->append("200" ,200);
    axisY_place_chartY->append("400", 400);
    axisY_place_chartY->append("600", 600);

    axisY_velocity_chartY->append("-1", -1);
    axisY_velocity_chartY->append("0", 0);
    axisY_velocity_chartY->append("1", 1);

    // Setting Axisx and Axisy

    chart_y->setAxisX(axisX_chartY, velocitySeries_y);
    chart_y->setAxisY(axisY_velocity_chartY, velocitySeries_y);

    chart_y->setAxisX(axisX_chartY, placeSeries_y);
    chart_y->setAxisY(axisY_place_chartY, placeSeries_y);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart_y);
    chartView->setRenderHint(QPainter::Antialiasing);
    gridlayout_yChart->addWidget(chartView, 0, 0);
}

// xml

void Dialog::z_chartDisplay()
{
    heightSeries->setName("Height");

    // Appending first point
    heightSeries->append(QPointF(0, 0));

    // Add series to chart

    chart_z->addSeries(heightSeries);

    chart_z->setVisible(true);
    chart_z->legend()->setAlignment(Qt::AlignTrailing);

    // Customize series

    QPen heightSeriesPen(QRgb(0x105C33));
    heightSeriesPen.setWidth(3);

    heightSeries->setPen(heightSeriesPen);

    // Customize chart title

    font.setPixelSize(18);

    chart_z->setTitleFont(font);
    chart_z->setTitleBrush(QBrush(QRgb(0xBCA8C2)));
    chart_z->setTitle("Chart-Z");

    // Customize chart background

    backgroundGradient.setStart(QPointF(0, 0));

    backgroundGradient.setFinalStop(QPointF(0, 1));

    backgroundGradient.setColorAt(0.2, QRgb(0x201523));
    backgroundGradient.setColorAt(0.5,QRgb(0x2D2130));

    backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    chart_z->setBackgroundBrush(backgroundGradient);

    // Customize axis label font

    labelsFont.setPixelSize(12);

    axisX_chartZ->setLabelsFont(labelsFont);
    axisY_height->setLabelsFont(labelsFont);

    // Customize axis colors

    QPen axisPen(QRgb(0x141414));
    axisPen.setWidth(2);

    axisX_chartZ->setLinePen(axisPen);
    axisY_height->setLinePen(axisPen);

    // Customize axis label colors

    QBrush axisBrush(Qt::white);

    axisX_chartZ->setLabelsBrush(axisBrush);
    axisY_height->setLabelsBrush(axisBrush);

    // Customize gyrid lines and shades

    axisX_chartZ->setGridLineVisible(false);
    axisY_height->setGridLineVisible(false);

    axisX_chartZ->setRange(0, 1000);
    axisY_height->setRange(0, 600);

    axisY_height->append("200", 200);
    axisY_height->append("400", 400);
    axisY_height->append("600", 600);

    // Setting Axisx and Axisy

    chart_z->setAxisX(axisX_chartZ, heightSeries);
    chart_z->setAxisY(axisY_height, heightSeries);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart_z);
    chartView->setRenderHint(QPainter::Antialiasing);
    gridlayout_zChart->addWidget(chartView, 0, 0);
}

void Dialog::circleXmlAppendingFunction()
{

    if(circleXmlCounter == 0 )
    {

        circleXmlFile.setFileName(circleXmlPath);

        if(circleXmlFile.open(QIODevice::Append))
        {
            streamWriterPlace->setDevice(&circleXmlFile);
            streamWriterPlace->setAutoFormatting(true);
            streamWriterPlace->writeStartDocument();
            streamWriterPlace->writeStartElement("CirclePlace");
        }
        circleXmlFile.close();
    }

    else if (circleXmlCounter >= 0 && !circleCenter_xVect.empty() && !circleCenter_yVect.empty())

    {

        if(circleXmlFile.open(QIODevice::Append))
        {
            QString time = QTime::currentTime().toString();
            streamWriterPlace->writeStartElement("Place");
            streamWriterPlace->writeAttribute("Time",time);
            streamWriterPlace->writeTextElement("Place_x",QString::number(circleCenter_xVect[circleXmlCounter]));
            streamWriterPlace->writeTextElement("Place_y",QString::number(circleCenter_yVect[circleXmlCounter]));
            streamWriterPlace->writeEndElement();
        }

       circleXmlFile.close();
    }

    circleXmlCounter++;
}

void Dialog::motionXmlAppendingFunction()
{

    if(motionXmlCounter == 0 )
    {
        if(motionXmlFile.open(QIODevice::Append))
        {
            streamWriterRate->setDevice(&motionXmlFile);
            streamWriterRate->setAutoFormatting(true);
            streamWriterRate->writeStartDocument();
            streamWriterRate->writeStartElement("MotionRate");
        }
        motionXmlFile.close();
    }

    else if (circleXmlCounter >= 0 && !x_rate_average.empty() && !y_rate_average.empty())

    {

        if(motionXmlFile.open(QIODevice::Append))
        {
            QString time = QTime::currentTime().toString();
            streamWriterRate->writeStartElement("Rate");
            streamWriterRate->writeAttribute("Time",time);
            streamWriterRate->writeTextElement("Rate_x",QString::number(x_rate_average[motionXmlCounter],'f',2));
            streamWriterRate->writeTextElement("Rate_y",QString::number(y_rate_average[motionXmlCounter],'f',2));
            streamWriterRate->writeEndElement();
        }

       motionXmlFile.close();
    }

    motionXmlCounter++;
}

void Dialog::on_btnBrowseCircleXmlLocation_clicked()
{
   circleXmlPath = QFileDialog::getSaveFileName();
   circleXmlFile.setFileName(circleXmlPath);
   ui->lEditCircleXmlPath->setText(circleXmlPath);
}

void Dialog::on_btnCircleCompLog_clicked()
{
    if(circleXmlFile.open(QIODevice::Append))
    {
        streamWriterPlace->writeEndElement();
        streamWriterPlace->writeEndDocument();
        circleXmlFile.close();
    }

    stop_timers();

}

void Dialog::on_btnDeleteCircleLog_clicked()
{
    circleXmlFile.setFileName(circleXmlPath);
    circleXmlFile.open(QIODevice::WriteOnly);
    circleXmlFile.close();
}

void Dialog::on_btnBrowseMotionXmlLocation_clicked()
{
    motionXmlPath = QFileDialog::getSaveFileName();
    motionXmlFile.setFileName(motionXmlPath);
    ui->lEditMotionXmlPath->setText(motionXmlPath);
}

void Dialog::on_btnMotionCompLog_clicked()
{
    if(motionXmlFile.open(QIODevice::Append))
    {
        streamWriterRate->writeEndElement();
        streamWriterRate->writeEndDocument();
        motionXmlFile.close();
    }

    stop_timers();
    ui->lblMotion->clear();
    ui->lblMotion->setStyleSheet("background-image: url(FormBackgrounds/motion-background.jpg); background-position: center; background-repeat: no-repeat; border-top: none; background-color: #fff; border-radius: 5px;");
}

void Dialog::on_btnDeleteMotionLog_clicked()
{
    motionXmlFile.setFileName(circleXmlPath);
    motionXmlFile.open(QIODevice::WriteOnly);
    motionXmlFile.close();
}
