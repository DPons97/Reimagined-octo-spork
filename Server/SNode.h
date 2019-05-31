//
// Created by dpons on 3/24/19.
//

#pragma once

#ifndef OCTOSPORK_CONNECTION_H
#define OCTOSPORK_CONNECTION_H

#include "Instructions/Instruction.h"
#include <vector>
#include <map>
#include "darknetCPP/DarknetCalculator.h"
#include "Planimetry.h"


class SNode : public Instruction {

public:

    SNode(const string &name, std::map<int, int> &instructions, vector<void*> sharedMemory);

    SNode(const string &name, std::map<int, int> &instructions, void * sharedMemory);

    void start(int nodeSocket, int nodePort) override;

    void track(const string& fileName, std::vector<std::string> args);

    virtual ~SNode();

protected:


private:
    // This server node's planimetry
    Planimetry * planimetry;

    long int deltaBeforeDisconnect;

    // Methods
    void startBkgSubtraction() const;
};


#endif //OCTOSPORK_CONNECTION_H
