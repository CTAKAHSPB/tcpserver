#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_MAX 80
#define PORT 8080

int main(int argc, const char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(1);
    }
    else
        printf("connected to the server..\n");

    // if having args, get a message from them, ask otherwise
    char *buffer = NULL;
    if (argc > 1) {
        buffer = (char*)malloc(strlen(argv[1]) + 1);
        sprintf(buffer, "%s\n", argv[1]);
    }
    else {
        unsigned long len;
        printf("Enter a message to a server: ");
        size_t read = getline(&buffer, &len, stdin);
        if (-1 == read) {
            printf("Error reading user input...\n");
            exit(1);
        }
    }

    // write a message to a server socket
    write(sockfd, buffer, strlen(buffer)); 

    // read/print server message
    char buff[BUFFER_MAX]; 
    bzero(buff, BUFFER_MAX); 
    read(sockfd, buff, sizeof(buff));
    printf("From server: %s\n", buff); 

    // close the socket
    close(sockfd);

    // free input buffer
    free(buffer);
}
