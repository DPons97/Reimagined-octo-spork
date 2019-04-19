//
// Created by dpons on 3/24/19.
//

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H

#include "../Logger.h"
#include <vector>

class SNode {
    int currSocket;

    int currPort;

    Logger log;

    vector<int> instructions;

public:
    void start(int nodeSocket, int nodePort);

    int getSocket() const;

    virtual ~SNode();

    const char * toString();

protected:


private:
    bool sendMessage(int instrCode, const list<string> &args = list<string>());

    bool getAnswerCode(int *outCode, int instrSocket);

    int sendInstruction(int instrCode, list<string> args = list<string>());

    void closeInstruction(int instrSocket);
};


#endif //OCTOSPORK_CONNECTION_H
