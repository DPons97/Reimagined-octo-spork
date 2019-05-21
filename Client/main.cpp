

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "CNode.h"


void loadConfigFromFile(char * fileName);

int main(int argc, char *argv[])
{
    if (argc < 12) {
        if(argc ==2){
            loadConfigFromFile(argv[1]);
        } else {
            fprintf(stderr,"usage %s servername port cpu_power id x z theta up down left right\nor load configuration from a file %s filename", argv[0], argv[0]);
        }

    } else{

        // create a node object and start to listen
        CNode node = CNode(atoi(argv[2]), argv[1], atoi(argv[3]),atoi(argv[4]), atoi(argv[5]),
            atoi(argv[6]),atoi(argv[7]),atoi(argv[8]),atoi(argv[9]), atoi(argv[10]), atoi(argv[11]));
        node.listen();
    }



    return 0;
}

void loadConfigFromFile(char * fileName){
    FILE *f = fopen(fileName, "r");
    int portno, cpu_power, id, abs_x, abs_z, theta, upNeighbour, bottomNeighbour, leftNeighbour, rightNeighbour;
    char hostname[16];
    fscanf(f, "%s %d %d %d %d %d %d %d %d %d %d", hostname, &portno,
            &cpu_power, &id, &abs_x, &abs_z, &theta, &upNeighbour, &bottomNeighbour, &leftNeighbour, &rightNeighbour);
    fclose(f);
    CNode node = CNode(portno, hostname, cpu_power, id, abs_x, abs_z, theta,
            upNeighbour, bottomNeighbour, leftNeighbour, rightNeighbour);
    node.listen();
}



