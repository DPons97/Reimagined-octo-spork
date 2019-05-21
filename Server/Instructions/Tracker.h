//
// Created by dpons on 5/19/19.
//

#ifndef OCTOSPORK_TRACKER_H
#define OCTOSPORK_TRACKER_H


#include "Instruction.h"

typedef struct {
    int x;
    int y;
    int z;
    double confidence;
} coordinate;

class Tracker : public Instruction {
public:
    Tracker(const string &name, const map<int, int> &instructions);

    void start(int nodeSocket, int nodePort, std::vector<std::string> args) override;

private:
    bool getAnswerCoordinates(int trackingSocket, coordinate& outCoords);

    void tracking(string toTrack, int trackPid);

    void relToAbsCoords(coordinate& toTransform);

    void saveCoords(string toTrack, std::vector<coordinate> coords);

};


#endif //OCTOSPORK_TRACKER_H
