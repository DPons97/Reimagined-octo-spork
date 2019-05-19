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

/**
 *  Represents a single node
 *  instructions can be sent and answers received
 *
 */
void SNode::start(int nodeSocket, int nodePort){
    this->currSocket = nodeSocket;
    this->currPort = nodePort;

    log = new Logger("nodeServer", true);
    log->writeLog(string("[").append(toString()).append("] New node created"));

    // START ALL OPERATIONS
    auto bkgSubInstr = new bkgSubtraction("bkgSubtraction", instructions);
    bkgSubInstr->start(currSocket, currPort);
}

/**
 * Send general purpose message to node through currSocket
 * @param instrCode
 * @param args
 * @return True if message has been sent successfully
 */
bool SNode::sendMessage(int instrCode, const std::vector<std::string> &args) {
    int n;
    std::string message;

    message.assign(std::to_string(instrCode));
    if (!args.empty()) {
        // Format message as:   instrCode-arg1,arg2,arg3,...,argn
        message.append("-");
        for (const std::string &s : args) {
            message.append(s).append(",");
        }
        message.erase(message.end() - 1, message.end());
    }

    log->writeLog(string("Sending ").append(message.data()));

    n = (int) write(currSocket, message.data(), strlen(message.data()));
    if (n < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR writing to socket"));
        return false;
    }

    return true;
}

/**
 * Close connection with defined node's process, and remove instrSocket from memorized instructions
 * @param instrPid to be closed. Leave empty (or -1) to close all intstructions
 */
void SNode::disconnect(int instrPid) {
    // Send pid to close. -1 if all connections have to be closed
    std::vector<std::string> toClose;
    toClose.push_back(std::to_string(instrPid));
    sendMessage(0,toClose);

    if (instrPid != -1) {
        close(instructions[instrPid]);

        for (auto & instruction : instructions) {
            if (instruction.first == instrPid)
                instruction.second = -1;
        }
    } else {
        // Close all instruction connections left
        for (auto & instr : instructions) {
            close(instr.second);
            instr.second = -1;
        }
    }
}

/**
 * @return SNode-[currSocket]
 */
const char * SNode::toString() {
    std::string nodeString;
    return nodeString.assign("SNode-").append(std::to_string(currSocket)).data();
}

/**
 * Default destructor
 * Send disconnecting message to node and close socket. Then delete log.
 */
SNode::~SNode() {
    log->writeLog(std::string("[").append(toString()).append("] Disconnecting..."));
    disconnect();
    close(currSocket);
    delete log;
}