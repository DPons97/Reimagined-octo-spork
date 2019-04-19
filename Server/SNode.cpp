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


void SNode::start(int nodeSocket, int nodePort){
    this->currSocket = nodeSocket;
    this->currPort = nodePort;

    log = new Logger("nodeServer", true);
    log->WriteLog(string("[").append(toString()).append("] New node created"));

    int answer, i;

    i = 1;
    do {
        int instrSock = sendInstruction(i);
        printf("Sending %d\n", i);

        getAnswerCode(&answer, instrSock);
        printf("Received %d\n", answer);
        i++;
    } while (i < 4); //&& answer == (i - 1 + 10));

    printf("Sending disconnection message\n");
    int discSock = sendMessage(0);
}

/*
 * Send an instruction to this node, creating a new connection every time
 * Return new socket if sending was successful, -1 if there was an error (see log)
 *      0 -> Close connection
 *      1 -> Background subtraction
 *      2 -> Tracking
 *      3 -> Identify
 */
int SNode::sendInstruction(int instrCode, list<string> args) {// Send instruction to client
    // Find first free port
    int assignedPort = currPort + 1;

    auto it = instructions.begin();
    while (it != instructions.end() && (*it) != -1) {
        assignedPort++;
        it++;
    }
    log->WriteLog(string("Found new port: ").append(to_string(assignedPort)));

    // Establish instruction connection at new port
    int sockfd, newSock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // Creating new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log->WriteLog(string("[").append(toString()).append("] ERROR opening new socket"));
    }

    // Force override old closed sockets
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        log->WriteLog("setsockopt(SO_REUSEADDR) failed");
    }

    // Setting server address and port no.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) assignedPort);

    // Binding socket to server address/port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log->WriteLog(string("[").append(toString()).append("] ERROR on binding"));
    }


    log->WriteLog(string("[").append(toString()).append("] Ready to connect at new socket"));

    // Merge assigned port to other arguments
    args.push_front(to_string(assignedPort));
    sendMessage(instrCode, args);
    // Listening for new connection at new assigned port
    listen(sockfd, 5);

    // New connection requested
    clilen = sizeof(cli_addr);
    log->WriteLog(string("[").append(toString()).append("] Connection requested. Accepting..."));
    newSock = accept(sockfd,
                     (struct sockaddr *) &cli_addr,
                     &clilen);

    close(sockfd);

    if (newSock < 0) {
        log->WriteLog(string("[").append(toString()).append("] Error on binding new socket"));
        return -1;
    } else {
        instructions.insert(it, newSock);
        return newSock;
    }
}

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
        log->WriteLog(string("[").append(toString()).append("] ERROR writing to socket"));
        return false;
    }

    return true;
}

/*
 * Wait for next answer from client
 * &outCode -> answer's instruction code
 */
bool SNode::getAnswerCode(int *outCode, int instrSocket) {
    int n;
    char answerBuff[10];

    bzero(answerBuff, 10);
    n = (int) read(instrSocket, answerBuff, sizeof(answerBuff));
    if (n < 0) {
        log->WriteLog(string("[").append(toString()).append("] ERROR reading answer"));
        return false;
    }

    *outCode = atoi(answerBuff);

    // TEMPORARY: Close connection on answer received
    closeInstruction(instrSocket);
    return true;
}

void SNode::closeInstruction(int instrSocket) {
    close(instrSocket);

    for (int i=0; i < instructions.size(); i++) {
        if (instructions[i] == instrSocket)
            instructions.at(i) = -1;
    }
}

int SNode::getSocket() const {
    return currSocket;
}

const char * SNode::toString() {
    string nodeString;
    return nodeString.assign("SNode-").append(std::to_string(currSocket)).data();
}

/*
 * Send disconnect command to node
 */
SNode::~SNode() {
    log->WriteLog(string("[").append(toString()).append("] Disconnecting..."));
    //sendInstruction(0);
    close(currSocket);
    delete log;
}