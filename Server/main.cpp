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

int newSocket(int portno);

void waitForConnection(int socket);


// Utility functions

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

using namespace std;

// Global variables

list<int> pids;      // Fork pids


int main(int argc, char *argv[]) {
    int newSock;

    // Socket port must be provided through command line
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    // Create new socket @ ConnectionHandler's port (defined by the user)
    int sockfd = newSocket(atoi(argv[1]));
    thread connectionHandler(waitForConnection, sockfd);

    // TODO: Close connections
    return 0;
}

int newSocket (int portno) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // Creating new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Setting server address and port no.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) portno);

    // Binding socket to server address/port
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    return sockfd;
}

void waitForConnection(int socket) {
    int newSock;
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    while (true) {
        // Listening to new connection
        listen(socket,5);

        // New connection requested
        clilen = sizeof(cli_addr);
        newSock = accept(socket,
                         (struct sockaddr *) &cli_addr,
                         &clilen);

        if (newSock < 0) {
            error("ERROR on accept");
            break;
        }

        // Create new Node object with new socket and start new process
        int newPid = fork();

        if (newPid < 0) {
            error("ERROR on fork");
        } else if (newPid == 0) {
            close(socket);
            auto * newNode = new SNode(newSock);

            exit(0);
        } else {
            pids.push_back(newPid);
            close(newSock);
        }

        // Delete pids of terminated forks
        // Declare iterator
        auto it = pids.begin();
        while (it != pids.end()) {
            // Remove this pid from array
            if (waitpid(*it, nullptr, WNOHANG)) it = pids.erase(it);
            else it++;
        }
    }
}
