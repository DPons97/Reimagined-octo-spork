#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <wait.h>
#include <list>

// Additional libraries and classes
#include "../Logger.h"
#include "SNode.h"

#define DEF_PORT 51297

int newSocket(int portno);

void waitForConnection(int socket, int sockPort);

void startNode(int newSock, int port);

// Utility functions
void error(const char *msg, Logger * log)
{
    perror(msg);
    log->WriteLog(msg);
    exit(1);
}

using namespace std;

// Global variables

Logger * mainLog;

int main(int argc, char *argv[]) {
    int port;
    mainLog = new Logger(string("mainServer"), true);

    // Socket port can be provided through command line (Default is 51297)
    // Create new socket @ ConnectionHandler's port (defined by the user or default)
    int sockfd = newSocket(port = (argc>=2) ? atoi(argv[1]) : DEF_PORT);
    waitForConnection(sockfd, port);

    delete mainLog;
    return 0;
}

int newSocket (int portno) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // Creating new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket", mainLog);

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        error("setsockopt(SO_REUSEADDR) failed", mainLog);

    // Setting server address and port no.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) portno);

    // Binding socket to server address/port
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding", mainLog);

    return sockfd;
}

void waitForConnection(int socket, int sockPort) {
    int newSock;
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    while (true) {
        // Listening to new connection
        listen(socket, 5);

        // New connection requested
        clilen = sizeof(cli_addr);
        mainLog->WriteLog("Waiting for connection...\n");
        newSock = accept(socket,
                         (struct sockaddr *) &cli_addr,
                         &clilen);

        mainLog->WriteLog("Connection requested");

        if (newSock < 0) {
            error("ERROR on accept", mainLog);
            break;
        } else {
            // auto newNode = new SNode();
            // Create new Node object as a thread
            thread * newThread = new thread(&startNode, newSock, sockPort);
            newThread->detach();
        }
    }
}

void startNode(int newSock, int port) {
    auto newNode = SNode();
    newNode.start(newSock, port);
    printf("==================================>>>>>> Fatto \n");
}
