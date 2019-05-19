

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "CNode.h"


int main(int argc, char *argv[])
{
    if (argc < 11) {
        fprintf(stderr,"usage %s servername port cpu_power id x z up down left right\n", argv[0]);
        exit(0);
    }

    // create a node object and start to listen
    CNode node = CNode(atoi(argv[2]), argv[1], atoi(argv[3]),atoi(argv[4]), atoi(argv[5]),
            atoi(argv[6]),atoi(argv[7]),atoi(argv[8]),atoi(argv[9]), atoi(argv[10]));
    node.listen();

    return 0;
}



