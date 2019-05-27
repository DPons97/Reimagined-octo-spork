//
// Created by dpons on 4/23/19.
//

#ifndef OCTOSPORK_DARKNETCALCULATOR_H
#define OCTOSPORK_DARKNETCALCULATOR_H

#include "darknet.h"
#include <opencv2/core/mat.hpp>
#include <mutex>

class DarknetCalculator {

public:

    DarknetCalculator(float thresh);

    detection * detect(cv::Mat input, int * num_boxes);

    std::vector<std::string> getLabels();

    void freeDetection();


private:
    // Network mutex
    static std::mutex networkMutex;

    // Path to data file.
    char * cfg_data;

    // Path to configuration file.
    char * cfg_file;

    // Path to weight file.
    char * weight_file;

    // Path to a file describing classes names.
    char * names_file;

    // Pointer to loaded darknet network
    static network * net;

    // Define thresholds for predicted class.
    float thresh;
    float hier_thresh = 0.5;

    detection * lastDetection;
    int lastNumBoxes;


};


#endif //OCTOSPORK_DARKNETCALCULATOR_H
