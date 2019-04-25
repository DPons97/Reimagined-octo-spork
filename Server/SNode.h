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
    int currSocket;

    int currPort;

    Logger * log;

    // Map of current open instructions: int PID, int SOCKET
    std::map<int, int> instructions;

public:
    void start(int nodeSocket, int nodePort);

    int getSocket() const;

    virtual ~SNode();

    const char * toString();

protected:


private:
    bool sendMessage(int instrCode, const std::vector<string> &args = std::vector<string>());

    bool getAnswerCode(string& outCode, int instrSocket);

    int startInstruction(int instrCode, std::vector<string> args = std::vector<string>());

    void backgroundSubtraction();

    void disconnect(int instrPid = 0);

    bool getAnswerImg(int bkgSocket, cv::Mat& outMat) const;
};


#endif //OCTOSPORK_CONNECTION_H
