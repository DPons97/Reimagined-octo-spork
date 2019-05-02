

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "CNode.h"


int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr,"usage %s servername port\n", argv[0]);
        exit(0);
    }

    // create a node object and start to listen
    CNode node = CNode(atoi(argv[2]), argv[1]);
    node.listen();

    return 0;
}



