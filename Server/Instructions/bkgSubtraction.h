//
// Created by dpons on 5/19/19.
//

#ifndef OCTOSPORK_BKGSUBTRACTION_H
#define OCTOSPORK_BKGSUBTRACTION_H


#include "Instruction.h"

#include "../darknetCPP/DarknetCalculator.h"
#include "../Planimetry.h"

#include <opencv2/core/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/highgui/highgui.hpp>

class bkgSubtraction : public Instruction {

public:
    bkgSubtraction(const string &name, const map<int, int> &instructions, vector<void*> sharedMemory);

    bkgSubtraction(const string &name, const map<int, int> &instructions, void* sharedMemory);

    void start(int nodeSocket, int nodePort) override;

private:
    Planimetry * planimetry;

    void backgroundSubtraction(vector<int> toTrack);

};


#endif //OCTOSPORK_BKGSUBTRACTION_H
