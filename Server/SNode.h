//
// Created by dpons on 3/24/19.
//

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H


#include <mutex>
#include "../Logger.h"

typedef struct {
    int nodeSocket;
    int nodePort;
} nodeArgs;

class SNode {
    int currSocket;

    int currPort;

    Logger log;

    list<int> instructions;

    mutex instrMutex;

public:
    void operator()(nodeArgs init);

    void start();

    int getSocket() const;

    virtual ~SNode();

    const char * toString();

protected:


private:
    bool sendMessage(int instrCode, const list<string> &args = list<string>());

    bool getAnswerCode(int *outCode, int instrSocket);

    int sendInstruction(int instrCode, list<string> args = list<string>());
};


#endif //OCTOSPORK_CONNECTION_H
