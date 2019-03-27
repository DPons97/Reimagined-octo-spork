#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void readMessages(int sockfd);

void writeMessages(int sockfd);

void sendImage(int sockfd);

int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    //std::thread reader(readMessages, sockfd);

    //std::thread writer(writeMessages, sockfd);
    std::thread imgSender(sendImage, sockfd);
    //reader.join();
    //writer.join();
    imgSender.join();
    close(sockfd);
    return 0;
}

void writeMessages(int sockfd) {
    int n;
    char buffer[256];
    do{
        //printf("Please enter the message: ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        n = write(sockfd, buffer, strlen(buffer));
        if(strcmp(buffer, "stop\n") ==0) break;
        if (n < 0)
            error("ERROR writing to socket");
    } while(strcmp(buffer, "stop")!= 0);
    printf("writer stopped\n");
}

void readMessages(int sockfd) {
    int n;
    char buffer[256];
    do {
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
            error("ERROR reading from socket");
        printf(" %s\n", buffer);
    }while(strcmp(buffer, "stop\n") != 0);
    printf("reader stopped\n");
}

void sendImage(int sockfd){
    std::vector<uchar> buff;
    int n;
    char path[30];
    const String ext = ".jpg";

    //alert image is coming
    write(sockfd, "imgsend",7);

    //open image
    printf("Insert path to image: ");
    scanf("%s", path);
    Mat image;
    image = imread(path, CV_LOAD_IMAGE_COLOR);

    // send width
    char msg[5];
    sprintf(msg, "%d", image.cols);
    printf("Sending cols: %s\n", msg);
    write(sockfd, msg, 5);


    // send height
    sprintf(msg, "%d", image.rows);
    printf("Sending rows: %s\n", msg);
    write(sockfd, msg, 5);

    // send image
    int  imgSize = image.total()*image.elemSize();
    printf("Sending image of size: %d", imgSize);
    write(sockfd, image.data, imgSize);

    //namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    //imshow( "Display window", image );                   // Show our image inside it.
    waitKey(0);

    return;
}

