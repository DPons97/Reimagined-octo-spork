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
#include <list>
#include <vector>
#include "../../Logger.h"
#include <signal.h>
#include <thread>

#define FRAME_NAME "../Client/Executables/resources/cam2/frame"

// meters !!
#define FOCAL_LENGTH 0.025
#define Z_CONSTANT 0.12
#define OBJECT_HEIGHT 1.80
#define PIXEL_SIZE 0.00000122

#define PIXEL_COMPRSSION_RATE 3

#define EMPTY_FRAMES_TO_STOP 5

#define FPS 30


#define FRAME_FILE "../Client/Executables/resources/curr_frame.txt"

using namespace cv;
using namespace dnn;
using namespace std;

/**
 * Client-side tracking operations
 */

typedef struct {
    int x;
    int y;
    int z;
    double confidence;
} track_point;

int sockfd;

int empty_frames; //counts frames with no object found

long int currFrame;

clock_t lastTime = 0;

Logger * mylog;

vector<string> classes;

float confThreshold = 0.5; // Confidence threshold

float nmsThreshold = 0.4;  // Non-maximum suppression threshold

int inpWidth = 256;  // Width of network's input image

int inpHeight = 256; // Height of network's input image

int track_class;

list<track_point> * track_points;

string nextImg();

void initCurrFrame();
void saveCurrFrame();
void sendTrackPoints();

// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& out);

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net);

// Estimate Z
int calcZ(int width, int height);

// Draw predictions
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

void openFrame(const string &kWinName, const Mat &frame);

void handler (int signal_number) {
    saveCurrFrame();
    mylog->writeLog(string("[nodeTracker] Exiting with signal ").append(to_string(signal_number)));
    delete mylog;
    exit(signal_number);
}


int main(int argc, char** argv) {
    // init job
    initCurrFrame();
    mylog = new Logger(string("node_tracker_").append(to_string(currFrame)),true);
    signal(SIGTERM, handler);
    sockfd = atoi(argv[1]);
    track_class = atoi(argv[2]);

    track_points = new list<track_point>;

    mylog->writeLog(string("New tracking job on socket ").append(to_string(sockfd)));
    mylog->writeLog(string("Starting from frame ").append(to_string(currFrame)));

    // Load classes
    string classesFile = "../Server/darknet/data/coco.names";
    ifstream ifs(classesFile.c_str());
    string line;

    static const string kWinName = "Tracking results";
    namedWindow(kWinName, WINDOW_NORMAL);

    while(getline(ifs, line)) classes.push_back(line);

    // Give the configuration and weight files for the model
    String modelConfiguration = "../Server/darknet/cfg/yolov3-tiny.cfg";
    String modelWeights = "../Server/darknet/yolov3-tiny.weights";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    string outputFile;
    VideoCapture cap;
    VideoWriter video;
    Mat frame, blob;

    while(true) //find better condition
    {
        frame = imread(nextImg().data(), CV_LOAD_IMAGE_COLOR);

        if (frame.empty()) {
            mylog->writeLog("FRAMES ENDED");
            break;
        }
        // Create a 4D blob from a frame.
        blobFromImage(frame, blob, 1/255.0, cvSize(inpWidth, inpHeight), Scalar(0,0,0), true, false);

        //Sets the input to the network
        net.setInput(blob);

        // Runs the forward pass to get output of the output layers
        vector<Mat> outs;
        net.forward(outs, getOutputsNames(net));
        mylog->writeLog("Frame analyzed");

        // Remove the bounding boxes with low confidence
        postprocess(frame, outs);
        thread * newThread = new thread(&openFrame, kWinName, frame);
        newThread->detach();

        if (empty_frames >= EMPTY_FRAMES_TO_STOP ) break;

    }
    sendTrackPoints();

    return 0;
}

/**
 * Open given frame in new window
 * @param kWinName
 * @param frame
 */
void openFrame(const string &kWinName, const Mat &frame) {
    putText(frame, "", Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));

    // Write the frame with the detection boxes
    imshow(kWinName, frame);
    waitKey(1000);
}

long int getTimeMs(){
    return std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

string nextImg(){
    double elapsedTime = ((double) (getTimeMs() - lastTime)/1000);
    mylog->writeLog(string("elapsed time: ").append(to_string(elapsedTime)));
    double toSleep = (1-elapsedTime);
    //mylog->writeLog(string("toSleep: ").append(to_string(toSleep)));
    if (toSleep > 0 ) {
        usleep(toSleep * 1000000);
        elapsedTime = ((double) (getTimeMs() - lastTime)/1000 );
        //mylog->writeLog(string("ellapsed time: ").append(to_string(elapsedTime)));
    }
    if (lastTime != 0) currFrame = currFrame + elapsedTime * FPS;
    lastTime = getTimeMs();
    string to_ret = string(FRAME_NAME).append(to_string(currFrame)).append(".jpg");
    mylog->writeLog(to_ret);
    return to_ret;
}


void initCurrFrame(){
    FILE *f = fopen(FRAME_FILE, "r");
    long int lastFrame;
    long int last_ms;

    fscanf(f, "%ld %ld", &lastFrame, &last_ms);

    if (last_ms > 0) {
        long int now_ms = getTimeMs();
        currFrame = lastFrame + ((double) (now_ms -last_ms)/1000)*FPS;
    } else currFrame = lastFrame;

    fclose(f);
}

void saveCurrFrame(){
    FILE *f = fopen(FRAME_FILE, "w");
    long int now_ms = getTimeMs();
    fprintf(f, "%ld %ld", currFrame, now_ms);
    mylog->writeLog(string("Saving current frame number ").append(to_string(currFrame)));
    fclose(f);
}

// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& outs) {
    bool found = false;
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    mylog->writeLog("Processing output data");
    for (const auto & out : outs)
    {
        mylog->writeLog("Scanning outputs");
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        auto* data = (float*)out.data;
        for (int j = 0; j < out.rows; ++j, data += out.cols)
        {
            Mat scores = out.row(j).colRange(5, out.cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold) {
                mylog->writeLog(classes[classIdPoint.x].append(" ").append(to_string(confidence)));

                if (classIdPoint.x == track_class) {
                    found = true;
                    empty_frames = 0;
                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;
                    int z = calcZ(width, height);
                    mylog->writeLog(string("Object found at ").append(to_string(centerX)).append("x").
                            append(to_string(centerY)).append(" with confidence of: ").
                            append(to_string(confidence*100)).append("%"));

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.emplace_back(left, top, width, height);

                    track_points->push_back((track_point){ .x = centerX, .y = centerY, .z = z, .confidence = confidence});
                }
            }
        }

        // Perform non maximum suppression to eliminate redundant overlapping boxes with
        // lower confidences
        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
        for (int idx : indices) {
            Rect box = boxes[idx];
            drawPred(classIds[idx], confidences[idx], box.x, box.y,
                     box.x + box.width, box.y + box.height, frame);
        }
    }
    if(!found)empty_frames ++;
}

int calcZ(int width , int height){
    // Formula from https://en.wikipedia.org/wiki/Pinhole_camera_model
    // maybe one day someone wants to use the width for other kind of objects...
    return OBJECT_HEIGHT*FOCAL_LENGTH*Z_CONSTANT/(height*PIXEL_SIZE*PIXEL_COMPRSSION_RATE)*100; //centimeters!!
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

void sendTrackPoints(){
    string msg;
    for(auto point: *track_points){
        msg = string("{").append(to_string(point.x)).append("_").
                append(to_string(point.y)).append("_").
                append(to_string(point.z)).append("_").
                append(to_string(point.confidence)).
                append("}");
        mylog->writeLog(string("Sending size: ").append(to_string(msg.size())));

        write(sockfd, to_string(msg.size()).data(), to_string(msg.size()).size());
        mylog->writeLog(string("Sending: ").append(msg));
        write(sockfd,msg.data(), msg.size());
    }
    write(sockfd, "0", 1);
    saveCurrFrame();
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
