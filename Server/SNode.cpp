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

SNode::SNode(const string &name, const map<int, int> &instructions) : Instruction(name, instructions) {}

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
    auto bkgSubInstr = new bkgSubtraction("bkgSubtraction", instructions);
    bkgSubInstr->start(socket, port);
}
