//
// Created by luca collini on 5/2/19.
//

#include <sstream>
#include <iostream>
#include <chrono>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include "../Logger.h"
#include <signal.h>

#define FRAME_NAME "../Executables/resources/frames/frame"
#define FRAME_FILE "../Executables/resources/curr_frame.txt"

using namespace cv;
using namespace dnn;
using namespace std;

int sockfd;
long int currFrame;
clock_t lastTime = 0;
Logger * mylog;
vector<string> classes;
float confThreshold = 0.5; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
int inpWidth = 416;  // Width of network's input image
int inpHeight = 416; // Height of network's input image

string nextImg();
void initCurrFrame();
void saveCurrFrame();
// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& out);
// Get the names of the output layers
vector<String> getOutputsNames(const Net& net);
// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

void handler (int signal_number) {
    saveCurrFrame();
    delete mylog;
    exit(signal_number);
}


int main(int argc, char** argv) {
    // init job
    // TODO: get object to track from parametes
    initCurrFrame();
    mylog = new Logger(string("node_tracker_").append(to_string(currFrame)),true);
    signal(SIGTERM, handler);
    sockfd = atoi(argv[1]);
    mylog->writeLog(string("New background subtraction job on socket ").append(to_string(sockfd)));
    mylog->writeLog(string("Starting from frame ").append(to_string(currFrame)));

    // Give the configuration and weight files for the model
    String modelConfiguration = "../Server/darknet/cfg/yolov3-tiny.cfg";
    String modelWeights = "../Server/darknet/yolov3-tiny.cfg";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    Mat frame, blob;

    while(true) //find better condition
    {
        frame = imread(nextImg().data(), CV_LOAD_IMAGE_COLOR);
        if (frame.empty()) {
            mylog->writeLog("ERROR OPENING FRAME");
            break;
        }
        // Create a 4D blob from a frame.
        blobFromImage(frame, blob, 1/255.0, cvSize(inpWidth, inpHeight), Scalar(0,0,0), true, false);

        //Sets the input to the network
        net.setInput(blob);

        // Runs the forward pass to get output of the output layers
        vector<Mat> outs;
        net.forward(outs, getOutputsNames(net));

        // Remove the bounding boxes with low confidence
        // TODO from here remove image processing. just look for object requested, get center x and y and area covered.
        postprocess(frame, outs);
    }


    return 0;
}

string nextImg(){
    double ellapsedTime = ((double) (clock() - lastTime)/CLOCKS_PER_SEC );
    //mylog->writeLog(string("ellapsed time: ").append(to_string(ellapsedTime)));
    double toSleep = (0.16667-ellapsedTime);
    //mylog->writeLog(string("toSleep: ").append(to_string(toSleep)));
    if (toSleep > 0 ) {
        usleep(toSleep * 1000000);
        ellapsedTime = ((double) (clock() - lastTime)/CLOCKS_PER_SEC ) + toSleep;
        //mylog->writeLog(string("ellapsed time: ").append(to_string(ellapsedTime)));
    }
    if(lastTime != 0) currFrame = currFrame + ellapsedTime * 30;
    lastTime = clock();
    string to_ret = string(FRAME_NAME).append(to_string(currFrame)).append(".jpg");
    mylog->writeLog(to_ret);
    return to_ret;
}

void initCurrFrame(){
    FILE *f = fopen(FRAME_FILE, "r");
    fscanf(f, "%ld", &currFrame);
    fclose(f);
}

void saveCurrFrame(){
    FILE *f = fopen(FRAME_FILE, "w");
    fprintf(f, "%ld", currFrame);
    mylog->writeLog(string("Saving current frame number ").append(to_string(currFrame)));
    fclose(f);
}

// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& outs)
{
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold)
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        drawPred(classIds[idx], confidences[idx], box.x, box.y,
                 box.x + box.width, box.y + box.height, frame);
    }
}

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

    //Get the label for the class name and its confidence
    string label = format("%.2f", conf);
    if (!classes.empty())
    {
        CV_Assert(classId < (int)classes.size());
        label = classes[classId] + ":" + label;
    }

    //Display the label at the top of the bounding box
    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    rectangle(frame, Point(left, top - round(1.5*labelSize.height)), Point(left + round(1.5*labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0),1);
}

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net)
{
    static vector<String> names;
    if (names.empty())
    {
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        vector<int> outLayers = net.getUnconnectedOutLayers();

        //get the names of all the layers in the network
        vector<String> layersNames = net.getLayerNames();

        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

