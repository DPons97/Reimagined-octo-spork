//
// Created by dpons on 3/24/19.
//

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H

#include "../Logger.h"
#include <vector>
#include <map>


class SNode {
    int currSocket;

    int currPort;

    Logger * log;

    // Map of current open instructions: int PID, int SOCKET
    map<int, int> instructions;

public:
    void start(int nodeSocket, int nodePort);

    int getSocket() const;

    virtual ~SNode();

    const char * toString();

protected:


private:
    bool sendMessage(int instrCode, const list<string> &args = list<string>());

    bool getAnswerCode(string& outCode, int instrSocket);

    int startInstruction(int instrCode, list<string> args = list<string>());

    void disconnect(int instrPid = 0);
};


#endif //OCTOSPORK_CONNECTION_H
