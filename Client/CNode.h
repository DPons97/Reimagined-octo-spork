//
// Created by pitohacapito on 3/26/19.
//

#ifndef OCTOSPORK_CNODE_H
#define OCTOSPORK_CNODE_H
#include "../Logger.h"

using namespace std;

class CNode {
private:
    Logger log;
    int sockfd;
    void bkgSubtraction();
    void identifyObjects();
    void trackObject();
    void sendMessage(int sockfd, int cod);
    void error(const char * msg);
public:
    CNode(int portno, char * hostname);
    void listen();
};


#endif //OCTOSPORK_CNODE_H
