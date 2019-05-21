//
// Created by dpons on 3/24/19.
//

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H

#include "../Logger.h"
#include <vector>
#include <map>
#include "darknetCPP/DarknetCalculator.h"
#include "Instructions/Instruction.h"

#include <opencv2/core/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/highgui/highgui.hpp>

class SNode : public Instruction {

public:
    SNode(const string &name, const map<int, int> &instructions);

    void start(int nodeSocket, int nodePort) override;

protected:


private:
    // Methods
};


#endif //OCTOSPORK_CONNECTION_H
