//
// Created by pitohacapito on 3/26/19.
//

#ifndef OCTOSPORK_CNODE_H
#define OCTOSPORK_CNODE_H
#include "../Logger.h"
#include <vector>
#include <map>
#include <list>

using namespace std;

typedef struct {
    int pid;
    int socket;
} child;

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

    list<child*> children;

    map<int, string> execNames;

    void sendMessage(int sockfd, int cod);

    void error(const char * msg);

    vector<string> split(string str,string sep);

    vector<char *> split_char(string str,string sep);

    int newSocket(int port);

    child* findChild(int pid);

    void killChild(child * toKill);

    void cleanChild(child * toKill);

    void readCodeFile();

};


#endif //OCTOSPORK_CNODE_H
