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
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <time.h>

//#define FRAME_NAME "/home/dpons/Documents/Programming/OctoSpork/Executables/resources/frames/frame"
#define FRAME_NAME "../Executables/resources/frames/frame"
#define FRAME_FILE "../Executables/resources/curr_frame.txt"

using namespace  cv;
using namespace std;

int sockfd;
long int currFrame;
clock_t lastTime = 0;
Logger * mylog;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

string nextImg();
bool sendImage(Mat image);
void initCurrFrame();
void saveCurrFrame();
bool waitForConfirm();

void handler (int signal_number) {
    saveCurrFrame();
}

int main(int argc, char *argv[]) {
    sockfd = atoi(argv[1]);
    mylog = new Logger("bkgSub", true);
    mylog->writeLog(string("New background subtraction job on socket ").append(to_string(sockfd)));

    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2(10, 16, false);

    initCurrFrame();
    signal(SIGTERM, handler);
    mylog->writeLog(string("Starting from frame ").append(to_string(currFrame)));

    Mat frame, fgMask;
    Scalar value;
    int count = 0;
    while (true) {
        if (currFrame > 1667) { // video stream is over
            //alert nothing was found and no more frames
            write(sockfd, "imgstop",7);
            saveCurrFrame();
            return 0;
        }
        frame = imread(nextImg().data(), CV_LOAD_IMAGE_COLOR);
        if (frame.empty()){
            mylog->writeLog("ERROR OPENING FRAME");
            break;
        }

        //update the background model
        pBackSub->apply(frame, fgMask);

        value = mean(fgMask);
        mylog->writeLog(string("mean ").append(to_string(value[0])));

        if (count > 3 && value[0] > 120) {
            mylog->writeLog(string("detected motion at frame ").append(to_string(currFrame)));
            mylog->writeLog(string("number of elaborated frames: ").append(to_string(count)));
            break;
        }
        count++;
    }
    sendImage(frame);
    saveCurrFrame();
    return 0;
}

string nextImg(){
    double ellapsedTime = ((double) (clock() - lastTime)/CLOCKS_PER_SEC );
    //mylog->writeLog(string("ellapsed time: ").append(to_string(ellapsedTime)));
    double toSleep = (0.3334-ellapsedTime);
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


/**
 * Send image to server
 * @param image to send
 * @return True if success
 */
bool sendImage(Mat image){
    vector<uchar> buff;
    char toLog[100];

    //alert image is coming
    write(sockfd, "imgsend",7);
    if (!waitForConfirm()) return false;

    // send width
    char msg[5];
    sprintf(msg, "%d", image.cols);
    sprintf(toLog, "Sending cols: %s\n", msg);
    mylog->writeLog(toLog);

    write(sockfd, msg, 5);
    if (!waitForConfirm()) return false;

    // send height
    sprintf(msg, "%d", image.rows);
    sprintf(toLog,"Sending rows: %s\n", msg);
    mylog->writeLog(toLog);

    write(sockfd, msg, 5);
    if (!waitForConfirm()) return false;

    // send image
    int  imgSize = image.total()*image.elemSize();
    sprintf(toLog,"Sending image of size: %d", imgSize);
    mylog->writeLog(toLog);

    write(sockfd, image.data, imgSize);
    return waitForConfirm();

    /*
    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "Display window", image );                   // Show our image inside it.
    waitKey(0);
    */

}

void initCurrFrame(){
    FILE *f = fopen(FRAME_FILE, "r");
    fscanf(f, "%li", &currFrame);
    fclose(f);
}

void saveCurrFrame(){
    FILE *f = fopen(FRAME_FILE, "w");
    fprintf(f, "%li", currFrame);
    fclose(f);
}

bool waitForConfirm() {
    char cmdBuff[10];
    int n = 0;

    // Wait for response
    do {
        n = (int) read(sockfd, cmdBuff, 10);
        if (n < 0) {
            mylog->writeLog("ERROR reading confirmation");
            return false;
        }
    } while (strcmp(cmdBuff, "ready") != 0);
    return true;
}