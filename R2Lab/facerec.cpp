#include "facerec.h"
//#include "ui_dialog.h"


FaceRec::FaceRec()
{
    // The first thing we are going to do is load all our models. First, since we need to find faces in the image we will need a face detector:
    detector = dlib::get_frontal_face_detector();

    // We will also use a face landmarking model to align faces to a standard pose:
    dlib::deserialize(face_landmarking_model) >> pose_model;

    // And finally we load the DNN responsible for face recognition.
    dlib::deserialize(dlib_face_recognition_model) >> net;
}

void FaceRec::input_csv_error(cv::VideoCapture cap)
{
    std::cout << "\nException thrown!" << std::endl;
    std::cerr << "No valid input for csv file." << std::endl;

    if (cap.isOpened())
        cap.release();

    QApplication::closeAllWindows();
    QCoreApplication::quit();
}

void FaceRec::find_and_replace(std::string &input, std::string const find, std::string const replace)
{
    for(std::string::size_type i = 0; (i = input.find(find, i)) != std::string::npos; )
    {
        input.replace(i, find.length(), replace);
        i += replace.length();

        if (input[i] != '0')
            break;
    }
}

void FaceRec::load_database(std::vector<std::string> *sendingLabels, cv::VideoCapture cap, bool *resetPBarTrackingQuality)
{
    std::ifstream file(db_path, std::ios::in);

    database_face_descriptors.clear();
    labels.clear();

    if (!file)
        input_csv_error(cap);
    else
    {
        std::string line, face_descriptor, label;
        dlib::matrix<float, 1, 128> m;
        int number_of_lines = 0;

        while (getline(file, line))
        {
            std::stringstream line_tmp(line);

            for (int i = 0; i < 128; i++)
            {
                getline(line_tmp, face_descriptor, ',');
                m(0, i) = std::stod(face_descriptor);

                if (i == 127)
                {
                    getline(line_tmp, label, ',');
                    find_and_replace(label, "0", "");
                    find_and_replace(label, " ", "");
                    labels.push_back(label.replace(label.end() - 4, label.end(), ""));
                }
            }

            database_face_descriptors.push_back(m);
            number_of_lines++;
        }
    }

    file.close();

    *sendingLabels = labels;
    *resetPBarTrackingQuality = true;

    detected_person_name.clear();
    trackers.clear();
}

void FaceRec::load_labels_only(std::vector<std::string> *sendingLabels, cv::VideoCapture cap, bool *resetPBarTrackingQuality)
{
    std::ifstream file(db_path, std::ios::in);

    labels.clear();

    if (!file)
        input_csv_error(cap);
    else
    {
        std::string line, face_descriptor, label;

        while (getline(file, line))
        {
            std::stringstream line_tmp(line);
            int i = 0;

            while (getline(line_tmp, face_descriptor, ','))
            {
                if (i++ == 127)
                {
                    getline(line_tmp, label, ',');
                    find_and_replace(label, "0", "");
                    find_and_replace(label, " ", "");
                    labels.push_back(label.replace(label.end() - 4, label.end(), ""));
                }
            }
        }
    }

    file.close();

    *sendingLabels = labels;
    *resetPBarTrackingQuality = false;
}

void FaceRec::recognize(std::vector< dlib::matrix<dlib::rgb_pixel> > faces, int index, double rec_threshold)
{
    /* This call asks the DNN to convert each face image in faces into a 128D vector.
     In this 128D vector space, images from the same person will be close to each other
     but vectors from different people will be far apart. So we can use these vectors to
     identify if a pair of images are from the same person or from different people. */

    face_descriptors = net(faces);

    for (size_t i = 0; i < database_face_descriptors.size(); i++)
    {
        /* Faces are connected in the graph if they are close enough. Here we check if
         the distance between two face descriptors is less than 0.6, which is the
         decision threshold the network was trained to use. Although you can
         certainly use any other threshold you find useful. */

        if (length(face_descriptors[index] - trans(database_face_descriptors[i])) < rec_threshold)
        {
            detected_person_name.push_back(labels[i]);
            is_set_detected_person_name = true;
        }
    }

    if (!is_set_detected_person_name)
        detected_person_name.push_back("unknown");

    is_set_detected_person_name = false;
}

void FaceRec::operation(cv::Mat *im, double rec_threshold, double tracking_quality_threshold, int skip_frames, int skip_frames_counter, int *sendingTrackersSize, double *tracking_quality_sum, cv::VideoCapture cap)
{
    try
    {
        // Disable Mirror mode.
//        cv::flip(im, im, 1);

        // Change to dlib's image format. No memory is copied.
        dlib::cv_image<dlib::bgr_pixel> cimg(*im);

        if (skip_frames_counter % skip_frames == 0)
        {
            // Now tell the face detector to give us a list of bounding boxes around all the faces it can find in the image.
            dets = detector(cimg);

            if (database_face_descriptors.size() != 0)
                faces.clear();

            for (size_t i = 0; i < dets.size(); i++)
            {
                if (database_face_descriptors.size() != 0)
                {
                    /* Run the face detector on the image of our action heroes, and for each face extract a
                     copy that has been normalized to 150x150 pixels in size and appropriately rotated
                     and centered. */

                    auto shape = pose_model(cimg, dets[i]);
                    extract_image_chip(cimg, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
                    faces.push_back(face_chip);
                }

                // Covert dlib rectangle to opencv rect
                rec.x = dets[i].left();
                rec.y = dets[i].top();
                rec.width = dets[i].width();
                rec.height = dets[i].height();

                // Calculate the centerpoint.
                center_x = (0.5 * rec.width) + rec.x;
                center_y = (0.5 * rec.height) + rec.y;

                is_face_matched = false;

                for (size_t j = 0; j < trackers.size(); j++)
                {
                    t_rec.x = trackers[j].get_position().left();
                    t_rec.y = trackers[j].get_position().top();
                    t_rec.width = trackers[j].get_position().width();
                    t_rec.height = trackers[j].get_position().height();

                    t_center_x = (0.5 * t_rec.width) + t_rec.x;
                    t_center_y = (0.5 * t_rec.height) + t_rec.y;

                    // Check if the centerpoint of the face is within the rectangle of a tracker region and vice versa.
                    if ((t_center_x >= rec.x) && (t_center_x <= (rec.x + rec.width)) &&
                        (t_center_y >= rec.y) && (t_center_y <= (rec.y + rec.height)) &&
                        (center_x >= t_rec.x) && (center_x <= (t_rec.x + t_rec.width)) &&
                        (center_y >= t_rec.y) && (center_y <= (t_rec.y + t_rec.height)))
                        is_face_matched = true;
                }

                // Recognition
                if (dets.size() != 0 && is_face_matched == false)
                {
                    tracker = new dlib::correlation_tracker;
                    tracker->start_track(cimg, dlib::centered_rect(dlib::point((rec.tl().x + rec.br().x) / 2, (rec.tl().y + rec.br().y) / 2), rec.width, rec.height));
                    trackers.push_back(*tracker);

                    if (database_face_descriptors.size() != 0)
                        recognize(faces, i, rec_threshold);
                }
            }
        }

        // Update all the trackers and remove the ones for which the update indicated the quality was not good enough.
        for (size_t i = 0; i < trackers.size(); )
        {
            tracking_quality = trackers[i].update(cimg);
            *tracking_quality_sum += tracking_quality;

            t_rec.x = trackers[i].get_position().left();
            t_rec.y = trackers[i].get_position().top();
            t_rec.width = trackers[i].get_position().width();
            t_rec.height = trackers[i].get_position().height();

            t_center_x = (0.5 * t_rec.width) + t_rec.x;
            t_center_y = (0.5 * t_rec.height) + t_rec.y;

            // Delete if the quality is not good enough or the rectangle's going outside the screen.
            if (tracking_quality < tracking_quality_threshold || t_center_x >= cap.get(CV_CAP_PROP_FRAME_WIDTH) || t_center_y >= cap.get(CV_CAP_PROP_FRAME_HEIGHT) || t_center_x <= 0 || t_center_y <= 0)
            {
                trackers.erase(trackers.begin() + i);
                if (detected_person_name.size() != 0)
                    detected_person_name.erase(detected_person_name.begin() + i);

                continue;
            }

            // Draw rectangle around detected face.
            cv::rectangle(*im, t_rec, CV_RGB(0, 255, 0), 1.5);

            // Recognition position.
            pos_x = std::max(t_rec.tl().x + 10, 0);
            pos_y = std::max(t_rec.tl().y - 20, 0);

            if (database_face_descriptors.size() != 0 && detected_person_name[i] != "unknown")
            {
                sec_rec.x = pos_x - 10;
                sec_rec.y = pos_y - 30;
                sec_rec.width = t_rec.width;
                sec_rec.height = 50;

                // Show recognized person.
                cv::rectangle(*im, sec_rec, CV_RGB(155, 25, 25), -1);
                cv::putText(*im, detected_person_name[i], cv::Point(pos_x, pos_y), cv::FONT_HERSHEY_SIMPLEX, 0.8, CV_RGB(55, 255, 55), 2.0);
            }

            i++;
        }

        *sendingTrackersSize = trackers.size();
    }
    catch (std::exception& e)
    {
        std::cout << "\nException thrown!" << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

void FaceRec::btnSaveByImagesPath_function(QString imagesAdd, std::vector<std::string> *sendingLabels, cv::VideoCapture cap, bool *resetPBarTrackingQuality)
{
    try
    {
        Dialog dialog;

        load_image(img, "Images/" + imagesAdd.toStdString());

        dlib::array2d<unsigned char> img_gray;
        dlib::assign_image(img_gray, img);

        dlib::image_window win;

        faces.clear();

        for (auto face : detector(img))
        {
            auto shape = pose_model(img, face);
            extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(face_chip);

            win.set_title("New Face");
            win.set_image(img_gray);
            win.add_overlay(face, dlib::rgb_pixel(0, 255, 0));

            dialog.show_save_window();
            win.clear_overlay();
        }

        if (faces.size() != 0)
        {
            win.close_window();

            face_descriptors = net(faces);

            std::ofstream file(db_path.c_str(), std::ios::out | std::ios::app);

            counter = 0;
            for (size_t i = 0; i < detected_person_name_from_photo.size(); i++)
            {
                oss.str("");
                oss.clear();

                oss << dlib::csv << trans(face_descriptors[counter++]);
                std::string s = oss.str();

                file << s.replace(s.end() - 1, s.end(), ", ") + detected_person_name_from_photo[i] + ".jpg" << std::endl;
            }

            file.close();

            faces.clear();
            detected_person_name_from_photo.clear();

            database_face_descriptors.size() == 0 ? load_labels_only(&*sendingLabels, cap, &*resetPBarTrackingQuality) : load_database(&*sendingLabels, cap, &*resetPBarTrackingQuality);
        }
        else
            std::cerr << "No face detected." << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "\nException thrown!" << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

void FaceRec::btnSaveByTakingPhoto_function(cv::Mat im, std::vector<std::string> *sendingLabels, cv::VideoCapture cap, bool *resetPBarTrackingQuality)
{
    try
    {
        Dialog dialog;

//        cv::flip(im, im, 1);

        dlib::cv_image<dlib::bgr_pixel> cimg(im);

        dlib::array2d<unsigned char> img_gray;
        dlib::assign_image(img_gray, cimg);

        dlib::image_window win;

        faces.clear();

        for (auto face : detector(cimg))
        {
            auto shape = pose_model(cimg, face);
            extract_image_chip(cimg, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(face_chip);

            win.set_title("New Face");
            win.set_image(img_gray);
            win.add_overlay(face, dlib::rgb_pixel(255, 0, 0));

            dialog.show_save_window();
            win.clear_overlay();
        }

        if (faces.size() != 0)
        {
            win.close_window();

            face_descriptors = net(faces);

            std::ofstream file(db_path, std::ios::out | std::ios::app);

            counter = 0;
            for (size_t i = 0; i < detected_person_name_from_photo.size(); i++)
            {
                oss.str("");
                oss.clear();

                oss << dlib::csv << trans(face_descriptors[counter++]);
                std::string s = oss.str();

                file << s.replace(s.end() - 1, s.end(), ", ") + detected_person_name_from_photo[i] + ".jpg" << std::endl;

                char *x = new char[detected_person_name_from_photo[i].length()];
                std::sprintf(x, "Images/%s.jpg", detected_person_name_from_photo[i].c_str());
                cv::imwrite(x, im);
                delete[] x;
            }

            file.close();

            faces.clear();
            detected_person_name_from_photo.clear();

            database_face_descriptors.size() == 0 ? load_labels_only(&*sendingLabels, cap, &*resetPBarTrackingQuality) : load_database(&*sendingLabels, cap, &*resetPBarTrackingQuality);
        }
        else
            std::cerr << "No face detected." << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "\nException thrown!" << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

void FaceRec::btnDeleteFromDatabase_function(int cmbDatabaseLabelsIndex, std::vector<std::string> *sendingLabels, cv::VideoCapture cap, bool *resetPBarTrackingQuality)
{
    std::ifstream file(db_path, std::ios::in);

    if (!file)
        input_csv_error(cap);
    else
    {
        std::string line, s;
        int i = 1;

        while (getline(file, line))
        {
            if (i++ == cmbDatabaseLabelsIndex)
                continue;

            oss.str("");
            oss.clear();

            oss << line << '\n';
            s += oss.str();
        }

        file.close();

        std::ofstream file(db_path, std::ios::out);

        if (!file)
            input_csv_error(cap);
        else
        {
            file << s;

            file.close();

            database_face_descriptors.size() == 0 ? load_labels_only(&*sendingLabels, cap, &*resetPBarTrackingQuality) : load_database(&*sendingLabels, cap, &*resetPBarTrackingQuality);
        }
    }
}
