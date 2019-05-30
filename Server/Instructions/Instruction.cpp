#include <utility>

//
// Created by dpons on 5/19/19.
//

#include "Instruction.h"
std::mutex Instruction::instrLock;

/**
 * Default constructor
 * @param name -> This instruction's identifier
 */
Instruction::Instruction(const string &name, const map<int, int> &instructions, vector<void*> sharedMemory) : name(name),
                                                                                  instructions(instructions),
                                                                                  sharedMemory(sharedMemory) {
    nodeSocket = 0;
    nodePort = 0;
    xImgSize = 0;
    yImgSize = 0;
    log = new Logger(name, true);
}

Instruction::Instruction(const string &name, const map<int, int> &instructions, void *sharedMemory) : name(name),
                                                                                  instructions(instructions) {
    std::vector<void *> memoryVector;
    memoryVector.insert(memoryVector.begin(), sharedMemory);

    nodeSocket = 0;
    nodePort = 0;
    log = new Logger(name, true);
}

/**
 *  This instruction's starting point
 *  @param socket Socket of node which is talking to this instruction
 *  @param port Port of this instruction
 *  @param args Vector of arguments to pass to this instruction
 */
void Instruction::start(int socket, int port, std::vector<std::string> args){
    this->nodeSocket = socket;
    this->nodePort = port;

    log->writeLog(string("[").append(toString()).append("] New instruction created"));
}

/**
 *  This instruction's starting point
 *  @param socket Socket of node which is talking to this instruction
 *  @param port Port of this instruction
 */
void Instruction::start(int socket, int port) {
    start(socket, port, std::vector<std::string>());
}


/**
 * Send general purpose message to node through currSocket
 * @param instrCode
 * @param args
 * @return True if message has been sent successfully
 */
bool Instruction::sendMessage(int instrCode, const std::vector<string> &args) {
    int n;
    std::string message;

    message.assign(std::to_string(instrCode));
    if (!args.empty()) {
        // Format message as:   instrCode-arg1,arg2,arg3,...,argn
        message.append("-");
        for (const std::string &s : args) {
            message.append(s).append(",");
        }
        message.erase(message.end() - 1, message.end());
    }

    log->writeLog(string("Sending ").append(message.data()));

    n = (int) write(nodeSocket, message.data(), strlen(message.data()));
    if (n < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR writing to socket"));
        return false;
    }

    return true;
}

/**
 * Wait for next answer from client
 * @param outCode answer's instruction code
 * @param instrSocket socket from the answer is expected
 * @return True if answer is correctly read
 */
bool Instruction::getAnswerCode(string& outCode, int instrSocket) {
    int n;
    char answerBuff[10];

    bzero(answerBuff, 10);
    n = (int) read(instrSocket, answerBuff, sizeof(answerBuff));
    if (n < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR reading answer"));
        return false;
    }

    outCode = std::string(answerBuff);
    return true;
}

/**
 *  Send an instruction to this node, creating a new connection
 *      0 -> DO NOT USE: Reserved for disconnection
 *      1 -> Background subtraction
 *      2 -> Tracking
 *      3 -> Identify
 *
 * @param instrCode instruction code to send to node
 * @param args other arguments to send
 * @return PID of Client's new process if sending was successful, -1 if there was an error (see log for more info)
 */
int Instruction::startInstruction(int instrCode, std::vector<string> args) {
    instrLock.lock();
    // Find first free port
    int assignedPort = nodePort + 1;

    auto it = instructions.begin();
    while (it != instructions.end() && it->second != -1) {
        assignedPort++;
        it++;
    }
    log->writeLog(std::string("Found new port: ").append(std::to_string(assignedPort)));

    // Establish instruction connection at new port
    int sockfd, newSock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // Creating new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR opening new socket"));
    }

    // Force override old closed sockets
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        log->writeLog("setsockopt(SO_REUSEADDR) failed");
    }

    // Setting server address and port no.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) assignedPort);

    // Binding socket to server address/port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log->writeLog(std::string("[").append(toString()).append("] ERROR on binding"));
    }


    log->writeLog(std::string("[").append(toString()).append("] Ready to connect at new socket"));

    // Merge assigned port to other arguments
    args.insert( args.begin(), std::to_string(assignedPort));
    sendMessage(instrCode, args);
    // Listening for new connection at new assigned port
    listen(sockfd, 5);

    // New connection requested
    clilen = sizeof(cli_addr);
    log->writeLog(std::string("[").append(toString()).append("] Connection requested. Accepting..."));
    newSock = accept(sockfd,
                     (struct sockaddr *) &cli_addr,
                     &clilen);

    close(sockfd);

    if (newSock < 0) {
        log->writeLog(std::string("[").append(toString()).append("] Error on binding new socket"));
        instrLock.unlock();
        return -1;
    } else {
        std::string answer;

        // Get answer from client including pid
        getAnswerCode(answer, newSock);
        int clientPid = atoi(answer.data());

        instructions.insert(it, pair<int, int>(clientPid, newSock));
        log->writeLog(std::string("[").append(toString()).append("] Connection accepted. Starting job..."));
        instrLock.unlock();
        return clientPid;
    }
}

/**
 * Wait for an image to arrive from client.
 * @param bkgSocket
 * @param outMat Mat that contains the answer image
 * @return True if answer received successfully
 */
bool Instruction::getAnswerImg(int bkgSocket, cv::Mat& outMat) {
    int n;
    std::vector<unsigned char> vectBuff;
    char cmdBuff[10];

    log->writeLog("Waiting for image send message to arrive");
    // Wait for send message
    do {
        bzero(cmdBuff, sizeof(cmdBuff));
        n = (int) read(bkgSocket, cmdBuff, 7);
        if (n <= 0) {
            log->writeLog("ERROR reading command message or node disconnected");
            return false;
        }

        if (strcmp(cmdBuff, "imgstop") == 0) {
            log->writeLog("Video stream ended from client. Disconnecting...");
            return false;
        }

        log->writeLog(cmdBuff);
    } while (strcmp(cmdBuff, "imgsend") != 0);
    write(bkgSocket, "ready", 5);

    // Receive Mat cols and rows
    log->writeLog("Reading columns");
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(bkgSocket, cmdBuff, 5);
    if (n < 0) {
        log->writeLog("ERROR reading image columns");
        return false;
    }
    int cols = atoi(cmdBuff);
    xImgSize = cols;
    write(bkgSocket, "ready", 5);

    log->writeLog("Reading rows");
    bzero(cmdBuff, sizeof(cmdBuff));
    n = (int) read(bkgSocket, cmdBuff, 5);
    if (n < 0) {
        log->writeLog("ERROR reading image rows");
        return false;
    }
    int rows = atoi(cmdBuff);
    yImgSize = rows;
    write(bkgSocket, "ready", 5);

    cv::Mat inMat = cv::Mat::zeros(rows, cols, CV_8UC3);

    int imgSize = (int) (inMat.total() * inMat.elemSize());
    uchar buffer[imgSize];

    log->writeLog("Reading image");
    n = 0;
    bzero(buffer, imgSize);
    for (int i = 0; i < imgSize; i += n) {
        n = (int) recv(bkgSocket, buffer + i, imgSize - i, 0);
        if (n < 0) {
            log->writeLog("ERROR reading from socket");
            return false;
        }
        //log->writeLog(string("Received ").append(to_string(i)).append("/").append(to_string(imgSize)));
    }
    log->writeLog("Image arrived");

    inMat.data = buffer;

    outMat = inMat;
    return true;
}

/**
 * Disconnect and stop instruction
 * @param instrPid instruction's PID to stop and disconnect
 */
void Instruction::disconnect(int instrPid) {
    // Send pid to close. -1 if all connections have to be closed
    std::vector<std::string> toClose;
    toClose.push_back(std::to_string(instrPid));
    sendMessage(0,toClose);

    if (instrPid != -1) {
        close(instructions[instrPid]);

        for (auto & instruction : instructions) {
            if (instruction.first == instrPid)
                instruction.second = -1;
        }
    } else {
        // Close all instruction connections left
        for (auto & instr : instructions) {
            close(instr.second);
            instr.second = -1;
        }
    }
}

/**
 * @return [Instruction name]-[currSocket]
 */
string Instruction::toString() {
    std::string nodeString = string(name);
    return nodeString.append("-").append(std::to_string(nodeSocket));
}

/**
 * @return this node's socket
 */
int Instruction::getNodeSocket() const {
    return nodeSocket;
}

/**
 * Default destructor
 * Send disconnecting message to node and close socket. Then delete log.
 */
Instruction::~Instruction() {
    log->writeLog(std::string("[").append(toString()).append("] Disconnecting..."));
    disconnect();
    close(nodeSocket);
    delete log;
}
