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
#include <signal.h>
#include "SNode.h"

#define DEF_PORT 51297

using namespace std;

// Global variables

Logger * mainLog;

// Function declariations

int newSocket(int portno);

void waitForConnection(int socket, int sockPort);

void startNode(int newSock, int port);

/**
 * Interrupt handler
 * @param signum signal that caused interrupt
 */
void signal_callback_handler(int signum) {
    if (signum == SIGTERM) {
        delete mainLog;
        exit(signum);
    }
}

// Utility functions
void error(const char *msg, Logger * log)
{
    perror(msg);
    log->writeLog(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int port;
    mainLog = new Logger("mainServer", true);

    signal(SIGTERM, signal_callback_handler);

    // Socket port can be provided through command line (Default is 51297)
    // Create new socket @ ConnectionHandler's port (defined by the user or default)
    int sockfd = newSocket(port = (argc>=2) ? atoi(argv[1]) : DEF_PORT);
    waitForConnection(sockfd, port);

    delete mainLog;
    return 0;
}

/**
 * Open new socket at defined port
 * @param portno port number of new socket
 * @return socket
 */
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

/**
 * Connection handler for server
 * @param socket server socket
 * @param sockPort server port
 */
void waitForConnection(int socket, int sockPort) {
    int newSock;
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    while (true) {
        // Listening to new connection
        listen(socket, 5);

        // New connection requested
        clilen = sizeof(cli_addr);
        mainLog->writeLog("Waiting for connection...");
        newSock = accept(socket,
                         (struct sockaddr *) &cli_addr,
                         &clilen);

        mainLog->writeLog("Connection requested");

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

/**
 * Start new node at connection
 * @param newSock socket of new connection
 * @param port of new connection
 */
void startNode(int newSock, int port) {
    auto newNode = SNode();
    newNode.start(newSock, port);
}
