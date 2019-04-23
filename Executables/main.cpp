#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../Logger.h"
#include <signal.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


int sockfd;


void handler (int signal_number) {
    close(sockfd);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int portno, n;
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &handler;
    sigaction (SIGTERM, &sa, NULL);


    if (argc < 3) {
        fprintf(stderr,"usage %s servername port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    Logger log;
    log = Logger("childLog",true);
    log.writeLog(string("Starting with args:").append(argv[1]).append("-").append(argv[2]));
    sleep(1);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
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
    log.writeLog(string("Trying to connect to ").append(argv[1]));
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    log.writeLog("Connected");

    sleep(1);

    char * message = const_cast<char *>("3");
    n = static_cast<int>(write(sockfd, message, 1));
    if(n<0){
        error("ERROR writing to socket");
    }
    log.writeLog(string("Sending message ").append(message));
    close(sockfd);
    return 0;
}