//
// Created by dpons on 3/24/19.
//
#include <strings.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <thread>

#include "SNode.h"

/**
 *  Represents a single node
 *  instructions can be sent and answers received
 *
 */
void SNode::start(int nodeSocket, int nodePort){
    this->currSocket = nodeSocket;
    this->currPort = nodePort;

    log = new Logger("nodeServer", true);
    log->writeLog(string("[").append(toString()).append("] New node created"));

    /* Type of object to detect
        1 -> person
        3 -> car
        4 -> motorbike
    */
    vector<int> objectToDetect = {1};
    thread * newThread = new thread(&SNode::backgroundSubtraction, this, std::ref(objectToDetect));
    newThread->join();
}

/**
 *  Send an instruction to this node, creating a new connection
 *      0 -> DO NOT USE: Reserved for disconnection
 *      1 -> Background subtraction
 *      2 -> Tracking
 *      3 -> Identify
 *
 * @param instrCode instruction code to send to node
 * @param args other arguments to send
 * @return PID of Client's new process if sending was successful, -1 if there was an error (see log for more info)
 */
int SNode::startInstruction(int instrCode, std::vector<std::string> args) {
    // Find first free port
    int assignedPort = currPort + 1;

    auto it = instructions.begin();
    while (it != instructions.end() && it->second != -1) {
        assignedPort++;
        it++;
    }
    log->writeLog(std::string("Found new port: ").append(std::to_string(assignedPort)));

    // Establish instruction connection at new port
    int sockfd, newSock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // Creating new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR opening new socket"));
    }

    // Force override old closed sockets
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        log->writeLog("setsockopt(SO_REUSEADDR) failed");
    }

    // Setting server address and port no.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) assignedPort);

    // Binding socket to server address/port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR on binding"));
    }


    log->writeLog(std::string("[").append(toString()).append("] Ready to connect at new socket"));

    // Merge assigned port to other arguments
    args.insert( args.begin(), std::to_string(assignedPort));
    sendMessage(instrCode, args);
    // Listening for new connection at new assigned port
    listen(sockfd, 5);

    // New connection requested
    clilen = sizeof(cli_addr);
    log->writeLog(std::string("[").append(toString()).append("] Connection requested. Accepting..."));
    newSock = accept(sockfd,
                     (struct sockaddr *) &cli_addr,
                     &clilen);

    close(sockfd);

    if (newSock < 0) {
        log->writeLog(std::string("[").append(toString()).append("] Error on binding new socket"));
        return -1;
    } else {
        std::string answer;

        // Get answer from client including pid
        getAnswerCode(answer, newSock);
        int clientPid = atoi(answer.data());

        instructions.insert(it, pair<int, int>(clientPid, newSock));
        return clientPid;
    }
}

/**
 * Send general purpose message to node through currSocket
 * @param instrCode
 * @param args
 * @return True if message has been sent successfully
 */
bool SNode::sendMessage(int instrCode, const std::vector<std::string> &args) {
    int n;
    std::string message;

    message.assign(std::to_string(instrCode));
    if (!args.empty()) {
        // Format message as:   instrCode-arg1,arg2,arg3,...,argn
        message.append("-");
        for (const std::string &s : args) {
            message.append(s).append(",");
        }
        message.erase(message.end() - 1, message.end());
    }

    n = (int) write(currSocket, message.data(), strlen(message.data()));
    if (n < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR writing to socket"));
        return false;
    }

    return true;
}

/**
 * Wait for next answer from client
 * @param outCode answer's instruction code
 * @param instrSocket socket from the answer is expected
 * @return True if answer is correctly read
 */
bool SNode::getAnswerCode(std::string& outCode, int instrSocket) {
    int n;
    char answerBuff[10];

    bzero(answerBuff, 10);
    n = (int) read(instrSocket, answerBuff, sizeof(answerBuff));
    if (n < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR reading answer"));
        return false;
    }

    outCode = std::string(answerBuff);
    return true;
}

/**
 * Close connection with defined node's process, and remove instrSocket from memorized instructions
 * @param instrPid to be closed. Leave empty (or -1) to close all intstructions
 */
void SNode::disconnect(int instrPid) {
    // Send pid to close. -1 if all connections have to be closed
    std::vector<std::string> toClose;
    toClose.push_back(std::to_string(instrPid));
    sendMessage(0,toClose);

    if (instrPid != -1) {
        close(instructions[instrPid]);

        for (auto & instruction : instructions) {
            if (instruction.first == instrPid)
                instruction.second = -1;
        }
    } else {
        // Close all instruction connections left
        for (auto & instr : instructions) {
            close(instr.second);
            instr.second = -1;
        }
    }
}

/**
 * @return current node's socket
 */
int SNode::getSocket() const {
    return currSocket;
}

/**
 * @return SNode-[currSocket]
 */
const char * SNode::toString() {
    std::string nodeString;
    return nodeString.assign("SNode-").append(std::to_string(currSocket)).data();
}

/**
 * Default destructor
 * Send disconnecting message to node and close socket. Then delete log.
 */
SNode::~SNode() {
    log->writeLog(std::string("[").append(toString()).append("] Disconnecting..."));
    disconnect();
    close(currSocket);
    delete log;
}

/**
 * Manage background subtraction operations
 */
void SNode::backgroundSubtraction(vector<int> toTrack) {
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

            cpp_free_detections(result, num_boxes);
            log->writeLog("Found something important! Starting tracking...");


            std::vector<std::string> objectString;
            // Iterate over every result
            for (int8_t i = 0; i < num_boxes; i++) {
                // And over every object trained
                for (int j = 0; j < labels.size(); j++) {
                    if (std::find(toTrack.begin(), toTrack.end(), j) != toTrack.end() && result[i].prob[j] >= threshold) {
                        // Send instruction to start tracking
                        objectString[0] = to_string(j);
                        tracking(j, startInstruction(2, objectString));
                    }
                }
            }

        } else {
            instructions[bkgPid] = -1;
            close(bkgSocket);
        }
    }

    // Stopped video stream or error occurred. Disconnecting last socket
    instructions.erase(bkgPid);
    close(bkgSocket);
}


/**
 * Wait for an image to arrive from client.
 * @param bkgSocket
 * @param outMat Mat that contains the answer image
 * @return True if answer received successfully
 */
bool SNode::getAnswerImg(int bkgSocket, cv::Mat& outMat) const {
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
    write(bkgSocket, "ready", 5);

    log->writeLog("Reading rows");
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(bkgSocket, cmdBuff, 5);
    if (n < 0) {
        log->writeLog("ERROR reading image rows");
        return false;
    }
    int rows = atoi(cmdBuff);
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

    /*
    static const std::string kWinName = "Sending images over the SPACE!";
    namedWindow(kWinName, cv::WINDOW_NORMAL);
    imshow(kWinName, inMat);
    cv::waitKey(0);
    */

    outMat = inMat;
    return true;
}

/**
 * Wait for a new coordinate result to arrive from client
 * @param trackingSocket
 * @param outCoords
 * @return True if answer received successfully. False if error occurred or no coordinates left
 */
bool SNode::getAnswerCoordinates(int trackingSocket, coordinate& outCoords) {
    int n;
    std::vector<unsigned char> vectBuff;
    char cmdBuff[100];
    log->writeLog("Waiting for coordinate size to arrive");

    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(trackingSocket, cmdBuff, 5);
    if (n <= 0) {
        log->writeLog("ERROR reading command message or node disconnected");
        return false;
    }

    if (strcmp(cmdBuff, "0") == 0) {
        log->writeLog("No more coordinates. Disconnecting...");
        return false;
    }

    // Read coordinates
    int coordSize = atoi(cmdBuff);
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) recv(trackingSocket, cmdBuff, coordSize, 0);
    if (n < 0) {
        log->writeLog("ERROR reading from socket");
        return false;
    }
    sscanf(cmdBuff, "{%d_%d_%d_%lf}", &outCoords.x, &outCoords.y, &outCoords.z, &outCoords.confidence);
    return true;
}

/**
 * Tracking engine
 * @param toTrack object to track
 * @param trackSocket socket to communicate with tracking starting point
 */
void SNode::tracking(int toTrack, int trackSocket) {
    // Init coords
    coordinate newCoordinate = {
            .x = 0,
            .y = 0,
            .z = 0,
            .confidence = 0.00
    };

    while (getAnswerCoordinates(trackSocket, newCoordinate)) {
        log->writeLog(string("Arrived coordinates: x = ").append(to_string(newCoordinate.x)
        .append(", y = ").append(to_string(newCoordinate.y))
        .append(", z = ").append(to_string(newCoordinate.z))
        .append(", confidence = ").append(to_string(newCoordinate.confidence))));

        // TODO: Do stuff with coordinates
    }
}