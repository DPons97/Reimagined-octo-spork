//
// Created by dpons on 3/24/19.
//

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H

#include "../Logger.h"
#include <vector>
#include <map>
#include "DarknetCalculator.h"

#include <opencv2/core/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/highgui/highgui.hpp>

class SNode {

public:
    int currSocket;

    int currPort;

    Logger * log;

    // Map of current open instructions: int PID, int SOCKET
    std::map<int, int> instructions;

    void start(int nodeSocket, int nodePort);

    const char * toString();

    virtual ~SNode();

protected:


private:
    // Methods
    bool sendMessage(int instrCode, const std::vector<string> &args = std::vector<string>());

    void disconnect(int instrPid = 0);
};


#endif //OCTOSPORK_CONNECTION_H
