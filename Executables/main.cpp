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

int sockfd;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void handler (int signal_number) {
    close(sockfd);
}

int main(int argc, char *argv[]) {
    Logger log = Logger(true);
    int n;
    sockfd = atoi(argv[1]);

    char * message = const_cast<char *>("2");
    n = static_cast<int>(write(sockfd, message, 1));
    if(n<0){
        error("ERROR writing to socket");
    }
    log.writeLog(string("Sending message ").append(message));
    close(sockfd);
    return 0;
}