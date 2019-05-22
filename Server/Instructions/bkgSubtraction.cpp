//
// Created by dpons on 5/19/19.
//

#include "bkgSubtraction.h"
#include "Tracker.h"

bkgSubtraction::bkgSubtraction(const string &name, const map<int, int> &instructions, vector<void*> sharedMemory):Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry *>(sharedMemory[0]);
}

bkgSubtraction::bkgSubtraction(const string &name, const map<int, int> &instructions, void *sharedMemory) : Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry * >(sharedMemory);

}

void bkgSubtraction::start(int socket, int port) {
    Instruction::start(socket, port);

    /* Type of object to detect
        1 -> person
        3 -> car
        4 -> motorbike
    */
    vector<int> objectToDetect = {0};

    backgroundSubtraction(objectToDetect);
}


/**
 * Manage background subtraction operations
 */
void bkgSubtraction::backgroundSubtraction(vector<int> toTrack) {
    float threshold = 0.5;
    int bkgSocket, bkgPid;

    // Do YOLO stuff and get results
    DarknetCalculator yoloCalculator = DarknetCalculator(threshold);

    while(true) {
        // Send start message
        bkgPid = startInstruction(1);
        bkgSocket = instructions[bkgPid];

        // Wait for relevant image
        cv::Mat image;

        if (!getAnswerImg(bkgSocket, image)) {
            log->writeLog("Stopping background subtraction process");
            break;
        }

        // Analyze results and start tracking if something is found
        int num_boxes = 0;
        int count = 0;
        detection * result = yoloCalculator.detect(image, &num_boxes);
        log->writeLog(string("Found: "));

        if (num_boxes > 0) {
            // Found something
            // TESTING PURPOSE: Iterate over predicted classes and print information.
            std::vector<std::string> labels = yoloCalculator.getLabels();
            for (int8_t i = 0; i < num_boxes; i++) {
                for (int j = 0; j < labels.size(); j++) {
                    if (result[i].prob[j] >= threshold) {
                        // More information is in each detections[i] item.
                        count++;
                        log->writeLog(string(labels[j]).append(" ").append(std::to_string(result[i].prob[j] * 100)));
                    }
                }
            }
            if (count == 0) log->writeLog(string("Nothing"));

            // Iterate over every result
            for (int8_t i = 0; i < num_boxes; i++) {
                // And over every object trained
                for (int j = 0; j < labels.size(); j++) {
                    if (std::find(toTrack.begin(), toTrack.end(), j) != toTrack.end() && result[i].prob[j] >= threshold) {
                        // Send instruction to start tracking
                        log->writeLog(string("Found ").append(labels[j]).append("! Starting tracking..."));

                        vector<string> trackingArgs;
                        trackingArgs.insert(trackingArgs.begin(), labels[j]);
                        trackingArgs.insert(trackingArgs.end(), to_string(j));
                        trackingArgs.insert(trackingArgs.end(), to_string(xImgSize));
                        trackingArgs.insert(trackingArgs.end(), to_string(yImgSize));

                        auto trackingInstr = new Tracker(string(labels[j]).append("-Tracker"), instructions, planimetry);
                        trackingInstr->start(nodeSocket, nodePort, trackingArgs);

                        // TODO Track only first object. To be fixed
                        break;
                    }
                }
            }

            cpp_free_detections(result, num_boxes);

        } else {
            instructions[bkgPid] = -1;
            close(bkgSocket);
        }
    }

    // Stopped video stream or error occurred. Disconnecting last socket
    instructions[bkgPid] = -1;
    close(bkgSocket);
}

/**
 * Wait for an image to arrive from client.
 * @param bkgSocket
 * @param outMat Mat that contains the answer image
 * @return True if answer received successfully
 */
bool bkgSubtraction::getAnswerImg(int bkgSocket, cv::Mat& outMat) {
    int n;
    std::vector<unsigned char> vectBuff;
    char cmdBuff[10];

    log->writeLog("Waiting for image send message to arrive");
    // Wait for send message
    do {
        bzero(cmdBuff, sizeof(cmdBuff));
        n = (int) read(bkgSocket, cmdBuff, 7);
        if (n <= 0) {
            log->writeLog("ERROR reading command message or node disconnected");
            return false;
        }

        if (strcmp(cmdBuff, "imgstop") == 0) {
            log->writeLog("Video stream ended from client. Disconnecting...");
            return false;
        }

        log->writeLog(cmdBuff);
    } while (strcmp(cmdBuff, "imgsend") != 0);
    write(bkgSocket, "ready", 5);

    // Receive Mat cols and rows
    log->writeLog("Reading columns");
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(bkgSocket, cmdBuff, 5);
    if (n < 0) {
        log->writeLog("ERROR reading image columns");
        return false;
    }
    int cols = atoi(cmdBuff);
    xImgSize = cols;
    write(bkgSocket, "ready", 5);

    log->writeLog("Reading rows");
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(bkgSocket, cmdBuff, 5);
    if (n < 0) {
        log->writeLog("ERROR reading image rows");
        return false;
    }
    int rows = atoi(cmdBuff);
    yImgSize = rows;
    write(bkgSocket, "ready", 5);

    cv::Mat inMat = cv::Mat::zeros(rows, cols, CV_8UC3);

    int imgSize = (int) (inMat.total() * inMat.elemSize());
    uchar buffer[imgSize];

    log->writeLog("Reading image");
    n = 0;
    bzero(buffer, imgSize);
    for (int i = 0; i < imgSize; i += n) {
        n = (int) recv(bkgSocket, buffer + i, imgSize - i, 0);
        if (n < 0) {
            log->writeLog("ERROR reading from socket");
            return false;
        }
        log->writeLog(string("Received ").append(to_string(i)).append("/").append(to_string(imgSize)));
    }

    inMat.data = buffer;

    outMat = inMat;
    return true;
}
