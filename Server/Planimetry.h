//
// Created by dpons on 5/19/19.
//

#pragma once

#ifndef OCTOSPORK_PLANIMETRY_H
#define OCTOSPORK_PLANIMETRY_H


#include "../Logger.h"
#include "Instructions/Instruction.h"

typedef struct NODE {
    Instruction * thisNode;
    int ID;
    int x;
    int z;
    int theta;
    int cpuPower;
    NODE * up;
    NODE * bottom;
    NODE * left;
    NODE * right;
} Node;

class Planimetry {

public:
    Logger * log;

    vector<Node *> planimetry;

    Planimetry();

    void addNode(int ID, int cpuPower, int x, int z, int theta, Instruction *toAdd, int up, int bottom, int left, int right);

    Node * getNode(Instruction * toFind);

    Node *getNode(int toFind);

    Node *getNodeBySocket(int toFind);

    ~Planimetry();

protected:


private:

    void freeNode(Instruction * toFree);
};


#endif //OCTOSPORK_PLANIMETRY_H
