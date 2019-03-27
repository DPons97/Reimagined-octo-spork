//
// Created by dpons on 3/24/19.
//

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H


#include "../Logger.h"

class SNode {
    const int socket;

    Logger log;

    char buffer[256];


public:
    SNode(int socket);

    void start();

    int getSocket() const;

    virtual ~SNode();

    const char * toString();

protected:


private:
    bool sendInstruction(int instrCode);

    bool getAnswerCode(int *outCode);

};


#endif //OCTOSPORK_CONNECTION_H
