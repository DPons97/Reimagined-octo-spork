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
#include <wait.h>
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
    this->hostname = hostname;
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
    int status;
    char buffer[256];
    list<int> pids;
    int childPid, deadPid;
    std::string msg;
    std::vector<std::string> sep_msg;
    std::vector<char *> args;
    while(1){
        bzero(buffer, 256);
        n_read = static_cast<int>(read(sockfd, buffer, 255));
        if (n_read < 0)
            error("ERROR reading from socket");
        log.WriteLog(string("Received ").append(buffer));

        msg = std::string(buffer);
        sep_msg = split(msg, "-");
        cod = std::atoi(sep_msg[0].c_str());

        if (sep_msg.size()>1){
            args = split_char(sep_msg[1], ",");
            args.insert(args.begin(), hostname);
        }
        args.push_back(NULL);
        
        deadPid = waitpid(-1, &status,WNOHANG);
        printf("waitpid res: %d", deadPid);
        if (!deadPid)pids.remove(deadPid);
            
        
        switch (cod){   
            case 0: // end connection and stop
            {
                for(auto &pid:pids){
                    kill(pid, SIGTERM);
                }
                log.WriteLog("EXIT MESSAGE RECEIVED");
                log.~Logger();
                exit(0);
            }
            case 1: // start background subtraction job
            {
                childPid = fork();
                if (!childPid) pids.push_back(childPid);

                else {
                    char * name = const_cast<char *>("ChildExample");
                    args.insert(args.begin(), name);
                    for (auto &arg : args) {
                        if(arg!= NULL)
                        log.WriteLog(arg);
                    }
                    execvp("../Executables/ChildExample", args.data());
                }
                break;
            }
            case 2: // start tracking job
            {
                childPid = fork();
                if (!childPid) pids.push_back(childPid);

                else{
                    char * name = const_cast<char *>("ChildExample");
                    args.insert(args.begin(), name);
                    for (auto &arg : args) {
                        log.WriteLog(arg);
                    }
                    execvp("../Executables/ChildExample2", args.data());
                }
                break;
            }
            case 3: // start identifier job
            {
                childPid = fork();
                if (!childPid) pids.push_back(childPid);

                else {
                    char * name = const_cast<char *>("ChildExample");
                    args.insert(args.begin(), name);
                    for (auto &arg : args) {
                        log.WriteLog(arg);
                    }
                    execvp("../Executables/ChildExample3", args.data());
                }
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

std::vector<std::string> CNode::split(std::string str,std::string sep){
    char* cstr=const_cast<char*>(str.c_str());
    char* current;
    std::vector<std::string> arr;
    current=strtok(cstr,sep.c_str());
    while(current!=NULL){
        arr.push_back(current);
        current=strtok(NULL,sep.c_str());
    }
    return arr;
}

std::vector<char *> CNode::split_char(std::string str,std::string sep){
    char* cstr=const_cast<char*>(str.c_str());
    char* current;
    std::vector<char *> arr;
    current=strtok(cstr,sep.c_str());
    while(current!=NULL){
        arr.push_back(current);
        current=strtok(NULL,sep.c_str());
    }
    return arr;
}


