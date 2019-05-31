//
// Created by dpons on 5/19/19.
//

#ifndef OCTOSPORK_TRACKER_H
#define OCTOSPORK_TRACKER_H


#include "Instruction.h"
#include "../Planimetry.h"

typedef struct {
    int x;
    int y;
    int z;
    double confidence;
} coordinate;

class Tracker : public Instruction {
public:
    Tracker(const string &name, std::map<int, int> &instructions, void * sharedMemory);

    Tracker(const string &name, std::map<int, int> &instructions, void * sharedMemory, string fileToWrite);

    void start(int nodeSocket, int nodePort, std::vector<std::string> args) override;

private:
    int xImgSize;

    int yImgSize;

    Planimetry * planimetry;

    string trackingName;

    int trackingID;

    string fileName;

    bool getAnswerCoordinates(int trackingSocket, coordinate& outCoords);

    void tracking(string toTrack, int trackPid);

    void relToAbsCoords(coordinate& toTransform);

    void saveCoords(string toTrack, std::vector<coordinate> coords);

    void keepTracking(const coordinate &lastCoord);
};


#endif //OCTOSPORK_TRACKER_H
