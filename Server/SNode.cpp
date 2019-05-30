//
// Created by dpons on 3/24/19.
//
#include <strings.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <thread>
#include <dirent.h>
#include <sys/stat.h>

#include "SNode.h"
#include "Instructions/bkgSubtraction.h"
#include "Instructions/Tracker.h"

SNode::SNode(const string &name, const map<int, int> &instructions, vector<void*> sharedMemory):Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry *>(sharedMemory[0]);
}

SNode::SNode(const string &name, const map<int, int> &instructions, void *sharedMemory) : Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry * >(sharedMemory);

}

/**
 *  Represents a single node
 *  instructions can be sent and answers received
 *
 */
void SNode::start(int socket, int port) {
    Instruction::start(socket, port);

    log = new Logger("nodeServer", true);
    log->writeLog(string("[").append(toString()).append("] New node created"));

    // START ALL OPERATIONS
    auto bkgSubInstr = new bkgSubtraction("bkgSubtraction", instructions, planimetry);
    bkgSubInstr->start(socket, port);
}

/**
 * Start new tracking job from another node
 * @param fileName name of file to write coordinates into
 * @param args tracking arguments
 */
void SNode::track(const string& fileName, std::vector<std::string> args) {
    auto trackingInstr = new Tracker(args[0] + "-Tracker", instructions, planimetry, fileName);
    trackingInstr->start(nodeSocket, nodePort, args);
}

SNode::~SNode() {
    planimetry->removeNode(nodeSocket);
}
