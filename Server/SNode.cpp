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

SNode::SNode(const string &name, std::map<int, int> &instructions, vector<void*> sharedMemory):Instruction(
        name, instructions, sharedMemory) {
    deltaBeforeDisconnect = 5000;
    planimetry = static_cast<Planimetry *>(sharedMemory[0]);
}

SNode::SNode(const string &name, std::map<int, int> &instructions, void *sharedMemory) : Instruction(
        name, instructions, sharedMemory) {
    deltaBeforeDisconnect = 5000;
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
    startBkgSubtraction();

    // First instruction finished. Wait to close node until no instructions left for 5s
    auto lastTime = getTimeMs();
    while (getTimeMs() - lastTime < deltaBeforeDisconnect) {
        usleep(500000);

        bool keepOpen = false;

        if (instructions.empty()) continue;

        // Check there are instructions left to do
        auto it = instructions.begin();
        while (it != instructions.end()) {
            if (it->second != -1) keepOpen = true;
            it++;
        }

        if (keepOpen) lastTime = getTimeMs();
    }
}

/**
 * Start new tracking job from another node
 * @param fileName name of file to write coordinates into
 * @param args tracking arguments
 */
void SNode::track(const string& fileName, std::vector<std::string> args) {
    // Disconnect all client's instructions before keeping track
    auto it = instructions.begin();
    while (it != instructions.end()) {
        if (it->second != -1) disconnect(it->first);
        it++;
    }

    auto trackingInstr = new Tracker(args[0] + "-Tracker", instructions, planimetry, fileName);
    trackingInstr->start(nodeSocket, nodePort, args);

    thread * newThread = new thread(&SNode::startBkgSubtraction, this);
    newThread->detach();
}

void SNode::startBkgSubtraction() const {
    auto bkgSubInstr = new bkgSubtraction("bkgSubtraction", instructions, planimetry);
    bkgSubInstr->start(nodeSocket, nodePort);
}

SNode::~SNode() {
    log->writeLog(std::string("[").append(toString()).append("] Disconnecting..."));
    disconnect();
    close(nodeSocket);
    planimetry->removeNode(nodeSocket);
}
