//
// Created by dpons on 5/19/19.
//

#pragma once

#ifndef OCTOSPORK_PLANIMETRY_H
#define OCTOSPORK_PLANIMETRY_H


#include "../Logger.h"
#include "Instructions/Instruction.h"

/**
 * Single node structure
 */
typedef struct NODE {
    Instruction * thisNode;
    int ID;
    int x;
    int z;
    int theta;
    NODE * up;
    NODE * bottom;
    NODE * left;
    NODE * right;
} Node;

/**
 * Data structure that represents how nodes are distributed throughout the territory
 */
class Planimetry {

public:
    Logger * log;

    vector<Node *> planimetry;

    Planimetry();

    void addNode(int ID, int x, int z, int theta, Instruction *toAdd, int up, int bottom, int left, int right);

    void removeNode(int socket);

    Node * getNode(Instruction * toFind);

    Node *getNode(int toFind);

    Node *getNodeBySocket(int toFind);

    ~Planimetry();

protected:


private:

    void freeNode(Node * toFree);

    void removeNulls(int ID, int up, int bottom, int left, int right);
};


#endif //OCTOSPORK_PLANIMETRY_H
