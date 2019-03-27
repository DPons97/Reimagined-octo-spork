//
// Created by lcollini on 3/26/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <list>
#include "CNode.h"

/**
 *
 * This class manages client communication and
 * execute requested functions
 * msg codes:
 * 0 kill all
 * 1 background subtraction
 * 2 track
 * 3 identify
 */

CNode::CNode(int portno, char * hostname) {
    // start connection and create Logger object
    struct sockaddr_in serv_addr;
    struct hostent *server;

    log = Logger(true);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    log.WriteLog(string("Trying to connect to ").append(hostname));
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    log.WriteLog("Connected");
}

void CNode::bkgSubtraction() {
    // TODO: actual implementation
    log.WriteLog("Starting background subtraction");
    sleep(3);
    log.WriteLog("Found a changement in frame");
    sendMessage(sockfd, 11); // this code is probably going to change
    //std::terminate();
}

void CNode::trackObject() {
    // TODO: actual implementation
    log.WriteLog("Starting object tracking");
    sleep(3);
    log.WriteLog("Object is here");
    sendMessage(sockfd, 12); // this code is probably going to change
    //std::terminate();
}

void CNode::identifyObjects() {
    // TODO: actual implementation
    log.WriteLog("Starting frame identification");
    sleep(3);
    log.WriteLog("I found a cute doggo");
    sendMessage(sockfd, 13); // this code is probably going to change
    //std::terminate();
}

void CNode::sendMessage(int sockfd, int cod) {
    // send a message with the passed code to the main node/server node
    int n_write;
    char buffer[256];
    //printf("Please enter the message: ");
    bzero(buffer, 256);
    snprintf(buffer, sizeof(buffer), "%d", cod);
    n_write = write(sockfd, buffer, strlen(buffer));
    if (n_write < 0)
        error("ERROR writing to socket");

    log.WriteLog(string("Sending message ").append(buffer));
}

void CNode::listen() {
    // listen for instructions from the main node/server node
    // when asked to start a task, execute it in a new thread
    int n_read, cod;
    char buffer[256];
    list<std::thread *> threads;
    while(1){
        bzero(buffer, 256);
        n_read = read(sockfd, buffer, 255);
        if (n_read < 0)
            error("ERROR reading from socket");
        log.WriteLog(string("Received ").append(buffer));
        cod = atoi(buffer);
        auto it = threads.begin();
        while (it != threads.end()){
            if(!(*it)->joinable()){
                (*it)->~thread();
                it = threads.erase(it);
            } else {
                it++;
            }
        }
        switch (cod){
            case 0: // end connection and stop
            {
                log.WriteLog("Killing threads");
                sendMessage(sockfd, 0);
                for (std::thread * t : threads){
                    t->~thread();
                }
                close(sockfd);
                log.~Logger();
                exit(0);
            }
            case 1: // start background subtraction job
            {
                std::thread subtracter(&CNode::bkgSubtraction, this);
                threads.push_back(&subtracter);
                subtracter.detach();
                break;
            }
            case 2: // start tracking job
            {
                std::thread tracker(&CNode::trackObject,this);
                threads.push_back(&tracker);
                tracker.detach();
                break;
            }
            case 3: // start identifier job
            {
                std::thread identifier(&CNode::identifyObjects,this);
                threads.push_back(&identifier);
                identifier.detach();
                break;
            }
            default: // report unidentified cod
                log.WriteLog("Unknown cod");
        }
    }

}


void CNode::error(const char *msg)
{
    perror(msg);
    log.WriteLog(msg);
    exit(1);
}