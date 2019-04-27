//
// Created by dpons on 4/23/19.
//

#include "DarknetCalculator.h"
#include <iostream>

DarknetCalculator::DarknetCalculator(float thresh) {
    // Define constants that were used when Darknet network was trained.
    cfg_data = const_cast<char *>("../Server/darknet/cfg/coco.data");
    cfg_file = const_cast<char *>("../Server/darknet/cfg/yolov3.cfg");
    weight_file = const_cast<char *>("../Server/darknet/yolov3.weights");
    names_file = const_cast<char *>("../Server/darknet/data/coco.names");
    this->thresh = thresh;

    // Load Darknet network itself.
    net = cpp_load_network(cfg_file, weight_file, 0);
    // In case of testing (predicting a class), set batch number to 1, exact the way it needs to be set in *.cfg file
    cpp_set_batch_network(net, 1);
}


/**
 * Start darknet's detection system
 * @param input image to analyze
 * @param num_boxes out number of boxes detected
 * @return detecton infos
 */
/*detection * DarknetCalculator::detect(cv::Mat input, int * num_boxes) {
    // Load image from OpenCV Mat
    int h = input.size().height;
    int w = input.size().width;
    int c = input.channels();
    image im = cpp_make_image(w, h, c);
    auto * data = (unsigned char *) input.data;
    int step = input.step;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }

    // And scale it to the parameters define in *.cfg file.
    image sized = cpp_letterbox_image(im, net->w, net->h);

    // Get actual data associated with test image.
    float *frame_data = sized.data;

    // Do prediction.
    cpp_network_predict(net, frame_data);

    // Get number of predicted classes (objects).
    lastDetection = cpp_get_network_boxes(net, im.w, im.h, thresh, hier_thresh, nullptr, 1, num_boxes);
    lastNumBoxes = *num_boxes;
    cpp_free_image(im);
    cpp_free_image(sized);
    return lastDetection;
}*/

/**
 * @return darknet classes' labels
 */
std::vector<std::string> DarknetCalculator::getLabels() {
    // Number of classes in "obj.names"
    size_t classes = 0;
    char **labels = cpp_get_labels(names_file);

    std::vector<std::string> labelsVector;
    while (labels[classes] != nullptr) {
        labelsVector.emplace_back(std::string(labels[classes]));
        classes++;
    }

    free(labels);
    return labelsVector;
}

void DarknetCalculator::freeDetection() {
    cpp_free_detections(lastDetection, lastNumBoxes);
}

/**
 * Start darknet's detection system
 * @param input image to analyze
 * @param num_boxes out number of boxes detected
 * @return detecton infos
 */
detection * DarknetCalculator::detect(cv::Mat input, int * num_boxes) {
    srand(2222222);

    float nms=.45;

    // Load image from OpenCV Mat
    int h = input.size().height;
    int w = input.size().width;
    int c = input.channels();
    image im = cpp_make_image(w, h, c);
    auto * data = (unsigned char *) input.data;
    int step = input.step;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }

    // And scale it to the parameters define in *.cfg file.
    image sized = cpp_letterbox_image(im, net->w, net->h);
    layer l = net->layers[net->n-1];

    // Get actual data associated with test image.
    float *frame_data = sized.data;
    // Do prediction.
    cpp_network_predict(net, frame_data);

    *num_boxes = 0;
    lastDetection = cpp_get_network_boxes(net, im.w, im.h, thresh, hier_thresh, nullptr, 1, num_boxes);

    if (nms) cpp_do_nms_sort(lastDetection, *num_boxes, l.classes, nms);

    cpp_free_image(im);
    cpp_free_image(sized);
    return lastDetection;
}