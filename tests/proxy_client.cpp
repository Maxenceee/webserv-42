#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");

        int pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            
            // Define target server address and port
            struct sockaddr_in target_addr;
            target_addr.sin_family = AF_INET;
            target_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            target_addr.sin_port = htons(3005);

            int target_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (target_sockfd < 0) 
                error("ERROR opening target socket");

            if (connect(target_sockfd, (struct sockaddr *) &target_addr, sizeof(target_addr)) < 0) 
                error("ERROR connecting to target");

            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                n = read(newsockfd, buffer, BUFFER_SIZE);
                if (n < 0) 
                    error("ERROR reading from socket");
                if (n == 0)
                    break;

                n = write(target_sockfd, buffer, n);
                if (n < 0) 
                    error("ERROR writing to target socket");

                memset(buffer, 0, BUFFER_SIZE);
                n = read(target_sockfd, buffer, BUFFER_SIZE);
                if (n < 0) 
                    error("ERROR reading from target socket");

                n = write(newsockfd, buffer, n);
                if (n < 0) 
                    error("ERROR writing to socket");
            }
            close(newsockfd);
            close(target_sockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    } 
    close(sockfd);
    return 0; 
}
