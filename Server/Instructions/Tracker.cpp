//
// Created by dpons on 5/19/19.
//

#include <cmath>
#include "Tracker.h"

#define PI 3.1415926
#define degToRad PI/180

Tracker::Tracker(const string &name, const map<int, int> &instructions, vector<void*> sharedMemory):Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry *>(sharedMemory[0]);
}

Tracker::Tracker(const string &name, const map<int, int> &instructions, void *sharedMemory) : Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry * >(sharedMemory);

}

/**
 * Starting point of tracker
 * @param nodeSocket socket of node
 * @param nodePort node's port
 * @param args arguments:
 *          args[0] -> object's name to track
 *          args[1] -> object's index to track
 *          args[2] -> frame size X
 *          args[3] -> frame size Y
 */
void Tracker::start(int nodeSocket, int nodePort, std::vector<std::string> args) {
    Instruction::start(nodeSocket, nodePort, args);

    xImgSize = atoi(args[2].data());
    yImgSize = atoi(args[3].data());

    std::vector<std::string> objectString;
    objectString.insert(objectString.begin(), args[1]);

    int trackingPid = startInstruction(2, objectString);
    tracking(args[0], trackingPid);
}

/**
 * Tracking engine
 * @param toTrack object to track
 * @param trackSocket socket to communicate with tracking starting point
 */
void Tracker::tracking(string toTrack, int trackPid) {
    vector<coordinate> coordinates;
    int trackSocket = instructions[trackPid];

    log->writeLog(string("Tracking: ").append(toTrack));

    bool coordsAvailable = true;
    while (coordsAvailable) {
        // Init coords
        coordinate newCoordinate = {
                .x = 0,
                .y = 0,
                .z = 0,
                .confidence = 0.00
        };

        coordsAvailable = getAnswerCoordinates(trackSocket, newCoordinate);

        log->writeLog(string("Arrived coordinates: x = ").append(to_string(newCoordinate.x)
                                                                         .append(", y = ").append(to_string(newCoordinate.y))
                                                                         .append(", z = ").append(to_string(newCoordinate.z))
                                                                         .append(", confidence = ").append(to_string(newCoordinate.confidence))));

        // Store coordinates
        coordinates.push_back(newCoordinate);
    }

    // Get last coordinate
    coordinate lastCoord = coordinates[coordinates.size() - 1];

    // Alien zone definition (tracked object has dematerialized)



    // Relative to absolute coordinates
    for (coordinate coord : coordinates) relToAbsCoords(coord);

    // Save coordinates to file
    saveCoords(toTrack, coordinates);

    instructions[trackPid] = -1;
    close(trackSocket);
}

/**
 * Wait for a new coordinate result to arrive from client
 * @param trackingSocket
 * @param outCoords
 * @return True if answer received successfully. False if error occurred or no coordinates left
 */
bool Tracker::getAnswerCoordinates(int trackingSocket, coordinate& outCoords) {
    int n;
    char cmdBuff[100];
    log->writeLog("Waiting for coordinate size to arrive");

    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(trackingSocket, cmdBuff, 2);
    if (n <= 0) {
        log->writeLog("ERROR reading size message or node disconnected");
        return false;
    }

    if (strcmp(cmdBuff, "0") == 0) {
        log->writeLog("No more coordinates. Disconnecting...");
        return false;
    }

    // Read coordinates
    int coordSize = atoi(cmdBuff);
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(trackingSocket, cmdBuff, coordSize);
    if (n < 0) {
        log->writeLog("ERROR reading from socket");
        return false;
    }
    sscanf(cmdBuff, "{%d_%d_%d_%lf}", &outCoords.x, &outCoords.y, &outCoords.z, &outCoords.confidence);
    return true;
}

/**
 * Transform coordinates from absolute to relative to this node
 * @param toTransform
 */
void Tracker::relToAbsCoords(coordinate& toTransform) {
    Node * thisNode = planimetry->getNodeBySocket(nodeSocket);

    // Translate and rotate coordinates
    toTransform.x = ((int) (toTransform.x * cos(thisNode->theta * degToRad) +
            toTransform.z * sin(thisNode->theta * degToRad))) + thisNode->x;

    toTransform.z = ((int) (toTransform.z * cos(thisNode->theta * degToRad) -
            toTransform.x * sin(thisNode->theta * degToRad))) + thisNode->z;
}

/**
 * Save coordinates to file
 */
void Tracker::saveCoords(string toTrack, std::vector<coordinate> coords) {
    auto stream = new fstream();
    string fileName;

    // Get current time
    auto CurrentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Format current time as Day/Month/DayNr_hh:dd:ss:Yr.Log
    auto FormattedTime = std::ctime(&CurrentTime);

    fileName.assign("Coordinates/tracking_").append(FormattedTime);

    // Replace spaces with underscores
    for (char &c : fileName) {
        c = c == ' ' ? '_' : c;
    }

    // Try to open Logs directory
    DIR * LogDir = opendir("Coordinates/");
    if (LogDir == nullptr) {
        // Create new dir
        mkdir("Coordinates/", 0777);
    } else closedir(LogDir);

    // Open log file
    stream->open(fileName.data(), ios::out);

    string toWrite = string("Tracking: ").append(toTrack).append("\n");
    stream->write(toWrite.data(), toWrite.length());
    for (coordinate c : coords) {
        toWrite = string("x = ")
                .append(to_string(c.x)
                                .append(", y = ").append(to_string(c.y))
                                .append(", z = ").append(to_string(c.z))
                                .append(", confidence = ").append(to_string(c.confidence)).append("\n"));

        stream->write(toWrite.data(), toWrite.length());
    }
    stream->close();
}