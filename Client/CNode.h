//
// Created by pitohacapito on 3/26/19.
//

#ifndef OCTOSPORK_CNODE_H
#define OCTOSPORK_CNODE_H
#include "../Logger.h"
#include <vector>

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
    vector<string> split(string str,string sep);
    vector<char *> split_char(string str,string sep);
    char * hostname;
public:
    CNode(int portno, char * hostname);
    void listen();
};


#endif //OCTOSPORK_CNODE_H
