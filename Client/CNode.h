//
// Created by pitohacapito on 3/26/19.
//

#ifndef OCTOSPORK_CNODE_H
#define OCTOSPORK_CNODE_H
#include "../Logger.h"
#include <vector>
#include <map>

using namespace std;

class CNode {

public:
    CNode(int portno, char * hostname);

    void listen();

    virtual ~CNode();

protected:

private:
    Logger * log;

    int sockfd;

    char * hostname;

    std::map<int, string> execNames;

    void sendMessage(int sockfd, int cod);

    void error(const char * msg);

    vector<string> split(string str,string sep);

    vector<char *> split_char(string str,string sep);

    void readCodeFile();

};


#endif //OCTOSPORK_CNODE_H
