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
 * This class manages client communication and execute requested functions.
 * Instruction codes:
 * 0 kill all and disconnect
 * 1 background subtraction
 * 2 track
 * 3 identify
 */
CNode::CNode(int portno, char * hostname) {
    // start connection and create Logger object
    struct sockaddr_in serv_addr;
    struct hostent *server;

    log = new Logger("nodeClient", true);
    execNames = map<int, string>();
    this->hostname = hostname;

    readCodeFile();

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
    log->writeLog(string("Trying to connect to ").append(hostname));
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    log->writeLog("Connected");
}

/**
 * Send message to server
 * @param sockfd server socket
 * @param cod code to send
 */
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

    log->writeLog(string("Sending message ").append(buffer));
}

/**
 * Listen for new intrusctions from main node/server.
 * When asked to start a task, execute it in a new thread
 */
void CNode::listen() {
    int n_read, cod;
    int status;
    char buffer[256];
    list<int> pids;
    int childPid, deadPid;
    std::string msg;
    std::vector<std::string> sep_msg;
    std::vector<char *> args;

    while(true){
        bzero(buffer, 256);
        n_read = static_cast<int>(read(sockfd, buffer, 255));
        if (n_read < 0)
            error("ERROR reading from socket");
        log->writeLog(string("Received ").append(buffer));

        msg = std::string(buffer);
        sep_msg = split(msg, "-");
        cod = std::atoi(sep_msg[0].c_str());

        if (sep_msg.size()>1){
            args = split_char(sep_msg[1], ",");
            args.insert(args.begin(), hostname);
        }
        args.push_back(NULL);
        
        deadPid = waitpid(-1, &status,WNOHANG);
        printf("waitpid res: %d\n", deadPid);
        if (!deadPid)pids.remove(deadPid);

        if (cod == 0) {

            // end connection and stop
            for(auto &pid:pids){
                kill(pid, SIGTERM);
            }
            log->writeLog("EXIT MESSAGE RECEIVED");
            log->~Logger();
            exit(0);

        } else if (cod > 0) {

            // start background subtraction job
            childPid = fork();
            if (childPid < 0) log->writeLog("ERROR on creating process");
            else if (childPid == 0) {

                char * name = const_cast<char *>(execNames[cod].data());
                args.insert(args.begin(), name);
                for (auto arg : args) {
                    if(arg!= NULL) log->writeLog(arg);
                }

                // Build exec path
                string path = "../Executables/";
                path.append(execNames[cod]);

                execvp(path.data(), args.data());

            } else pids.push_back(childPid);

        } else log->writeLog("Unknown cod");
    }
}

/**
 * Split string by string separator
 * @param str to be split
 * @param sep separator string
 * @return vector of split string
 */
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

/**
 * Split string by string separator
 * @param str to be split
 * @param sep separator string
 * @return vector of split arrays of char
 */
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

/**
 * Read instruction codes and relative executable names from file executables.txt
 *
 */
void CNode::readCodeFile() {
    // Open executable info file
    ifstream execFile;
    int execQty = 0;

    execFile.open("../Executables/executables.txt", std::ifstream::in);
    // Read executables quantity
    execFile >> execQty;
    if (execQty == 0) return;

    // Read executables names from file
    string buffer;
    vector<string> sep_buffer;
    std::getline(execFile, buffer);
    for (int i = 0; i < execQty; ++i) {
        int code = -1;
        std::getline(execFile, buffer);

        // Split string in instruction code and executable name
        sep_buffer = split(buffer, ":");
        std::sscanf(sep_buffer[0].data(), "%d", &code);

        execNames[code] = sep_buffer[1];
    }
}

/**
 * Error handler
 * @param msg message to display
 */
void CNode::error(const char *msg)
{
    perror(msg);
    log->writeLog(msg);
    exit(1);
}

/**
 * Default destructor
 */
CNode::~CNode() {
    delete log;
}


