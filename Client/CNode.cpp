//
// Created by luca collini on 3/26/19.
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
    log = new Logger("nodeClient", true);
    execNames = map<int, string>();
    this->hostname = hostname;

    readCodeFile();
    sockfd = newSocket(portno);
}

/**
 * Send message to server
 * @param sockfd server socket
 * @param cod code to send
 */
void CNode::sendMessage(int socket, int cod) {
    // send a message with the passed code to the main node/server node
    int n_write;
    char buffer[256];
    //printf("Please enter the message: ");
    bzero(buffer, 256);
    snprintf(buffer, sizeof(buffer), "%d", cod);
    n_write = write(socket, buffer, strlen(buffer));
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
    int childPid, deadPid;
    int port;
    string msg;
    vector<string> sep_msg;
    vector<char *> args;

    while(true){
        bzero(buffer, 256);
        n_read = static_cast<int>(read(sockfd, buffer, 255));
        if (n_read < 0)
            error("ERROR reading from socket");
        log->writeLog(string("Received ").append(buffer));

        msg = string(buffer);
        sep_msg = split(msg, "-");
        cod = atoi(sep_msg[0].c_str());

        do {
            deadPid = waitpid(-1, &status, WNOHANG);
            printf("waitpid res: %d\n", deadPid);
            if (deadPid > 0) cleanChild(findChild(deadPid));
        } while(deadPid > 0);

        if (cod == 0) {
            int termCod = atoi(sep_msg[1].c_str());
            if(termCod == 0){ // if termCod is 0 kill all and exit
                // end connection and stop
                log->writeLog("EXIT MESSAGE RECEIVED");
                for(auto &child:children){
                    kill(child->pid, SIGTERM);
                    close(child->socket);
                    free(child);
                }
                log->~Logger();
                exit(0);
            }
            // otherwise kill only the child with pid == temCod
            killChild(findChild(termCod));

        } else if (cod > 0) {
            if (sep_msg.size()>1){
                args = split_char(sep_msg[1], ",");
                port = atoi(args[0]);
                args.erase(args.begin());
            }
            args.push_back(nullptr);

            int newSock = newSocket(port);
            char* arg1 = (char *) to_string(newSock).c_str();
            args.insert(args.begin(), arg1);

            childPid = fork();

            if (childPid < 0) log->writeLog("ERROR on creating process");
            else if (childPid == 0) {

                char * name = const_cast<char *>(execNames[cod].data());
                args.insert(args.begin(), name);
                for (auto arg : args) {
                    if(arg!= nullptr) log->writeLog(arg);
                }

                // Build exec path
                string path = "../Client/Executables/";
                path.append(execNames[cod]);
                write(newSock, to_string(getpid()).data(), to_string(getpid()).size());

                // TODO Testing purposes: wait key to be pressed before starting

                execvp(path.data(), args.data());

            } else {
                auto newChild = (child *) malloc(sizeof(child));
                newChild->pid = childPid;
                log->writeLog(string("created child with PID ").append(to_string(childPid)));
                newChild->socket = newSock;
                children.push_back(newChild);
            }
        } else log->writeLog("Unknown cod");
    }
}

/**
 * Split string by string separator
 * @param str to be split
 * @param sep separator string
 * @return vector of split string
 */
vector<string> CNode::split(const string& str,const string& sep){
    char* cstr=const_cast<char*>(str.c_str());
    char* current;
    vector<string> arr;
    current=strtok(cstr,sep.c_str());
    while(current!=nullptr){
        arr.emplace_back(current);
        current=strtok(nullptr,sep.c_str());
    }
    return arr;
}

/**
 * Split string by string separator
 * @param str to be split
 * @param sep separator string
 * @return vector of split arrays of char
 */
vector<char *> CNode::split_char(string str,string sep){
    char* cstr=const_cast<char*>(str.c_str());
    char* current;
    vector<char *> arr;
    current=strtok(cstr,sep.c_str());
    while(current!=nullptr){
        arr.push_back(current);
        current=strtok(nullptr,sep.c_str());
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

    execFile.open("../Client/Executables/executables.txt", ifstream::in);
    if (!execFile.is_open()) log->writeLog("Error opening executables file");

    // Read executables quantity
    execFile >> execQty;
    if (execQty == 0) return;

    // Read executables names from file
    string buffer;
    vector<string> sep_buffer;
    getline(execFile, buffer);
    for (int i = 0; i < execQty; ++i) {
        int code = -1;
        getline(execFile, buffer);

        // Split string in instruction code and executable name
        sep_buffer = split(buffer, ":");
        sscanf(sep_buffer[0].data(), "%d", &code);

        execNames[code] = sep_buffer[1];
    }
}

/**
 * Looks for the child with given pid
 * @param pid pid to look for
 * @return child with given pid, nullptr if not found
 */
child* CNode::findChild(int pid) {
    log->writeLog(string("looking for child with pid ").append(to_string(pid)));
    for(auto &child: children){
        if(child->pid == pid) {
            log->writeLog(string("child with pid ").append(to_string(pid).append(" found")));
            return child;
        }
    }
    log->writeLog(string("child with pid ").append(to_string(pid).append(" NOT found")));
    return nullptr;
}

/**
 * Kills child passed
 * @param toKill child to kill
 */
void CNode::killChild(child * toKill){
    int status;
    if(toKill == nullptr || waitpid(toKill->pid, &status, WNOHANG)!=0) return;
    log->writeLog(string("killing child with pid ").append(to_string(toKill->pid)));
    kill(toKill->pid, SIGTERM);
    cleanChild(toKill);
}

/**
 * Cleans child passed
 * @param toClean child to clean
 */
void CNode::cleanChild(child * toClean){
    if(toClean == nullptr) return;
    log->writeLog(string("cleaning child with pid ").append(to_string(toClean->pid)));
    close(toClean->socket);
    children.remove(toClean);
    free(toClean);
}

int CNode::newSocket(int port){
    int portno = port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sleep(1);
    int newSock = socket(AF_INET, SOCK_STREAM, 0);
    if (newSock < 0)
        error("ERROR opening socket");
    server = gethostbyname(hostname);
    if (server == nullptr) {
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
    if (connect(newSock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    log->writeLog("Connected");
    return newSock;
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


