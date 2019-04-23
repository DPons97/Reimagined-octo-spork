//
// Created by dpons on 3/24/19.
//

#include <strings.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <list>
#include "SNode.h"


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

    string answer;
    int i;

    i = 1;
    do {
        int instrSock = startInstruction(i);
        printf("Sending %d\n", i);

        getAnswerCode(answer, instrSock);
        printf("Received %s\n", answer.data());
        i++;
    } while (i < 4);
}

/**
 *  Send an instruction to this node, creating a new connection
 *      0 -> DO NOT USE: Reserved for disconnection
 *      1 -> Background subtraction
 *      2 -> Tracking
 *      3 -> Identify
 *
 * @param instrCode instruction code to send to node
 * @param args other arguments to send
 * @return new socket if sending was successful, -1 if there was an error (see log for more info)
 */
int SNode::startInstruction(int instrCode, list<string> args) {
    // Find first free port
    int assignedPort = currPort + 1;

    auto it = instructions.begin();
    while (it != instructions.end() && it->second != -1) {
        assignedPort++;
        it++;
    }
    log->writeLog(string("Found new port: ").append(to_string(assignedPort)));

    // Establish instruction connection at new port
    int sockfd, newSock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // Creating new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log->writeLog(string("[").append(toString()).append("] ERROR opening new socket"));
    }

    // Force override old closed sockets
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        log->writeLog("setsockopt(SO_REUSEADDR) failed");
    }

    // Setting server address and port no.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) assignedPort);

    // Binding socket to server address/port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log->writeLog(string("[").append(toString()).append("] ERROR on binding"));
    }


    log->writeLog(string("[").append(toString()).append("] Ready to connect at new socket"));

    // Merge assigned port to other arguments
    args.push_front(to_string(assignedPort));
    sendMessage(instrCode, args);
    // Listening for new connection at new assigned port
    listen(sockfd, 5);

    // New connection requested
    clilen = sizeof(cli_addr);
    log->writeLog(string("[").append(toString()).append("] Connection requested. Accepting..."));
    newSock = accept(sockfd,
                     (struct sockaddr *) &cli_addr,
                     &clilen);

    close(sockfd);

    if (newSock < 0) {
        log->writeLog(string("[").append(toString()).append("] Error on binding new socket"));
        return -1;
    } else {
        string answer;

        // Get answer from client including pid
        getAnswerCode(answer, newSock);
        int clientPid = atoi(answer.data());

        instructions.insert(it, pair<int, int>(clientPid, newSock));
        return newSock;
    }
}

/**
 * Send general purpose message to node through currSocket
 * @param instrCode
 * @param args
 * @return True if message has been sent successfully
 */
bool SNode::sendMessage(int instrCode, const list<string> &args) {
    int n;
    string message;

    message.assign(to_string(instrCode));
    if (!args.empty()) {
        // Format message as:   instrCode-arg1,arg2,arg3,...,argn
        message.append("-");
        for (const string &s : args) {
            message.append(s).append(",");
        }
        message.erase(message.end() - 1, message.end());
    }

    n = (int) write(currSocket, message.data(), strlen(message.data()));
    if (n < 0) {
        log->writeLog(string("[").append(toString()).append("] ERROR writing to socket"));
        return false;
    }

    return true;
}

/**
 * Wait for next answer from client
 * @param outCode answer's instruction code
 * @param instrSocket socket from the answer is expected
 * @return True if answer is correctly read
 */
bool SNode::getAnswerCode(string& outCode, int instrSocket) {
    int n;
    char answerBuff[10];

    bzero(answerBuff, 10);
    n = (int) read(instrSocket, answerBuff, sizeof(answerBuff));
    if (n < 0) {
        log->writeLog(string("[").append(toString()).append("] ERROR reading answer"));
        return false;
    }

    outCode = string(answerBuff);
    return true;
}

/**
 * Close connection with defined node's process, and remove instrSocket from memorized instructions
 * @param instrPid to be closed. Leave empty (or -1) to close all intstructions
 */
void SNode::disconnect(int instrPid) {
    // Send pid to close. -1 if all connections have to be closed
    list<string> toClose;
    toClose.push_back(to_string(instrPid));
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
 * @return current node's socket
 */
int SNode::getSocket() const {
    return currSocket;
}

/**
 * @return SNode-[currSocket]
 */
const char * SNode::toString() {
    string nodeString;
    return nodeString.assign("SNode-").append(std::to_string(currSocket)).data();
}

/**
 * Default destructor
 * Send disconnecting message to node and close socket. Then delete log.
 */
SNode::~SNode() {
    log->writeLog(string("[").append(toString()).append("] Disconnecting..."));
    disconnect();
    close(currSocket);
    delete log;
}