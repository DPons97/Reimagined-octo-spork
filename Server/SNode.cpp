//
// Created by dpons on 3/24/19.
//

#include <strings.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "SNode.h"


SNode::SNode(int socket) : socket(socket) {
    printf("[%s] Connection established\n", toString());
    start();
}

void SNode::start(){
    int answer, i;

    i = 1;
    do {
        sendInstruction(i);
        printf("Sending %d\n", i);

        getAnswerCode(&answer);
        printf("Received %d\n", answer);
        i++;
    } while (i < 4); //&& answer == (i - 1 + 10));

    sendInstruction(0);

    do {
        getAnswerCode(&answer);
    } while (answer != 0);
}

/*
 * Send an instruction to this node, if it's not busy.
 * Return [true] if sending was successful, [false] if there was an error (see .mainLog)
 *      0 -> Close connection
 *      1 -> Background subtraction
 *      2 -> Tracking
 *      3 -> Identify
 */
bool SNode::sendInstruction(int instrCode) {
    int n;
    bzero(buffer, 256);
    sprintf(buffer, "%d", instrCode);

    n = (int) write(socket, buffer, strlen(buffer));
    if (n < 0) {
        log.WriteLog("ERROR writing to socket");
        return false;
    }

    return true;
}

bool SNode::getAnswerCode(int *outCode) {
    int n;
    char answerBuff[10];

    bzero(answerBuff, 10);
    n = (int) read(socket, answerBuff, sizeof(answerBuff));
    if (n < 0) {
        log.WriteLog("ERROR reading answer");
        return false;
    }

    *outCode = atoi(answerBuff);

    return true;
}

int SNode::getSocket() const {
    return socket;
}

const char * SNode::toString() {
    string nodeString;
    return nodeString.assign("SNode-").append(std::to_string(socket)).data();
}

/*
 * Send disconnect command to node
 */
SNode::~SNode() {
    printf("%s disconnecting...\n", toString());
    sendInstruction(0);
    close(socket);
}