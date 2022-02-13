#ifndef FaceRec_H
#define FaceRec_H


#include "dialog.h"

#include <QDialog>
#include <QApplication>
#include <QCoreApplication>

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/opencv.h>
#include <dlib/string.h>
#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/dir_nav.h>

class FaceRec
{
public:
    FaceRec();

    // ----------------------------------------------------------------------------------------

    /* The next bit of code defines a ResNet network.  It's basically copied
     and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
     layer with loss_metric and made the network somewhat smaller.  Go read the introductory
     dlib DNN examples to learn what all this stuff means.

     Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
     The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
     essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
     mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
     was set to 10000, the jittering you can see below in jitter_image() was used during
     training, and the training dataset consisted of about 3 million images instead of 55.
     Also, the input layer was locked to images of size 150. */

    template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual = dlib::add_prev1<block<N,BN,1,dlib::tag1<SUBNET>>>;

    template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual_down = dlib::add_prev2<dlib::avg_pool<2,2,2,2,dlib::skip1<dlib::tag2<block<N,BN,2,dlib::tag1<SUBNET>>>>>>;

    template <int N, template <typename> class BN, int stride, typename SUBNET>
    using block  = BN<dlib::con<N,3,3,1,1,dlib::relu<BN<dlib::con<N,3,3,stride,stride,SUBNET>>>>>;

    template <int N, typename SUBNET> using ares      = dlib::relu<residual<block,N,dlib::affine,SUBNET>>;
    template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block,N,dlib::affine,SUBNET>>;

    template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
    template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
    template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
    template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
    template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

    using anet_type = dlib::loss_metric<dlib::fc_no_bias<128,dlib::avg_pool_everything<
                                alevel0<
                                alevel1<
                                alevel2<
                                alevel3<
                                alevel4<
                                dlib::max_pool<3,3,2,2,dlib::relu<dlib::affine<dlib::con<32,7,7,2,2,
                                dlib::input_rgb_image_sized<150>
                                >>>>>>>>>>>>;

    // ----------------------------------------------------------------------------------------

    std::string db_path = "Database/db.csv";
    std::string face_landmarking_model = "Models/shape_predictor_68_face_landmarks.dat";
    std::string dlib_face_recognition_model = "Models/dlib_face_recognition_resnet_model_v1.dat";

    dlib::shape_predictor pose_model;
    anet_type net;

    dlib::frontal_face_detector detector;

    std::vector<dlib::rectangle> dets;

    std::vector< dlib::matrix<dlib::rgb_pixel> > faces;
    dlib::matrix<dlib::rgb_pixel> face_chip;

    std::vector< dlib::matrix<float, 0, 1> > face_descriptors;
    std::vector< dlib::matrix<float> > database_face_descriptors;
    std::vector<std::string> labels;

    std::vector<std::string> detected_person_name, detected_person_name_from_photo;
    bool is_face_matched = false, is_set_detected_person_name = false;

    dlib::correlation_tracker *tracker;
    std::vector<dlib::correlation_tracker> trackers;
    double tracking_quality, tracking_quality_sum = 0;

    dlib::matrix<dlib::rgb_pixel> img;

    std::ostringstream oss;

    cv::Rect rec, t_rec, sec_rec;

    int pos_x, pos_y;
    int center_x, center_y, t_center_x, t_center_y;

    int counter = 0;

    void input_csv_error(cv::VideoCapture);

    void find_and_replace(std::string&, std::string const, std::string const);

    void load_database(std::vector<std::string>*, cv::VideoCapture, bool*);

    void load_labels_only(std::vector<std::string>*, cv::VideoCapture, bool*);

    void recognize(std::vector< dlib::matrix<dlib::rgb_pixel> >, int, double);

    void operation(cv::Mat*, double, double, int, int, int*, double*, cv::VideoCapture);
    void btnSaveByImagesPath_function(QString, std::vector<std::string>*, cv::VideoCapture, bool*);
    void btnSaveByTakingPhoto_function(cv::Mat, std::vector<std::string>*, cv::VideoCapture, bool*);
    void btnDeleteFromDatabase_function(int, std::vector<std::string>*, cv::VideoCapture, bool*);
};

#endif // FaceRec_H
