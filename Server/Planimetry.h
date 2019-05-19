//
// Created by dpons on 5/19/19.
//

#ifndef OCTOSPORK_PLANIMETRY_H
#define OCTOSPORK_PLANIMETRY_H


#include "../Logger.h"
#include "SNode.h"

typedef struct NODE {
    SNode * thisNode;
    int ID;
    int x;
    int z;
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

    void addNode(int ID, int cpuPower, int x, int z, SNode *toAdd, int up, int bottom, int left, int right);

    Node * getNode(SNode * toFind);

    Node *getNode(int toFind);

    ~Planimetry();

protected:


private:
    void freeNode(SNode * toFree);
};


#endif //OCTOSPORK_PLANIMETRY_H
