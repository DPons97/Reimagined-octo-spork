//
// Created by dpons on 4/23/19.
//

#ifndef OCTOSPORK_DARKNETCALCULATOR_H
#define OCTOSPORK_DARKNETCALCULATOR_H


#include <darknet.h>
#include <opencv2/core/mat.hpp>

class DarknetCalculator {

public:

    DarknetCalculator(float thresh);

    detection * detect(cv::Mat input, int * num_boxes);

    std::vector<std::string> getLabels();


private:
    // Path to configuration file.
    static char * cfg_file;

    // Path to weight file.
    static char * weight_file;

    // Path to a file describing classes names.
    static char * names_file;

    // Pointer to loaded darknet network
    network * net;

    // Define thresholds for predicted class.
    float thresh;
    float hier_thresh = 0.5;

};


#endif //OCTOSPORK_DARKNETCALCULATOR_H
