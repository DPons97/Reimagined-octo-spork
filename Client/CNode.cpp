//
// Created by pitohacapito on 3/26/19.
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

void error(const char *msg)
{
    perror(msg);
    exit(0);
}


void CNode::bkgSubtraction() {

}

void CNode::identifyObjects() {

}

void CNode::trackObject() {

}

CNode::CNode(int portno, char * hostname) {
    struct sockaddr_in serv_addr;
    struct hostent *server;


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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    log = Logger();
}

void CNode::listen() {
    int n_read, cod;
    char buffer[256];
    list<std::thread *> threads;
    while(1){
        bzero(buffer, 256);
        n_read = read(sockfd, buffer, 255);
        if (n_read < 0)
            error("ERROR reading from socket");
        log.WriteLog(strcat("Received ",  buffer));
        cod = atoi(buffer);

        switch (cod){
            case 0:
            {
                std::thread subtracter(&CNode::bkgSubtraction, this);
                threads.push_back(&subtracter);
                break;
            }
            case 1:
            {
                std::thread tracker(&CNode::trackObject,this);
                threads.push_back(&tracker);
                break;
            }
            case 2:
            {
                std::thread identifier(&CNode::identifyObjects,this);
                threads.push_back(&identifier);
                break;
            }
            default:
                log.WriteLog("Unknown cod");
        }
    }

}
