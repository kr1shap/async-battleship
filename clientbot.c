#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 1024
//

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Need port number.\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[BUF_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    printf("Connected to Async-Battleship server at port %s\n", argv[1]);

    //registration 
    int botNum=0;
    while(1) {
        snprintf(buffer, sizeof(buffer), "REG bot-%d\n", botNum);
        if (write(sockfd, buffer, strlen(buffer)) < 0) {
            perror("Write failed");
        }

        int n = read(sockfd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);  // print all server messages

            if (strstr(buffer, "WELCOME") != NULL) {
                break;
            }
        } else if (n == 0) {
            printf("Server disconnected\n");
            break;
        } else {
            perror("Read error");
            break;
        }
    }
    //main bot loop - reads messages, and sends 'bombing' message at a random arbituary rate 
    int x = 0; 
    int y = 0; 
    while(1) { 
        srand(time(NULL));
        int val = (rand() % (5 - 2 + 1)) + 2;
        // Sleep first for given arbituary time, then send bomb message
        sleep(val);
        snprintf(buffer, sizeof(buffer), "BOMB %d %d\n", x, y++);
        if (write(sockfd, buffer, strlen(buffer)) < 0) {
            perror("Write failed");
        }
        int n = read(sockfd, buffer, sizeof(buffer)-1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);
        }
        else  break; //server disconnect
        if(y > 9) { 
            y = 0; 
            x++;
        }
        if(x > 9) x = 0; 
    }
    close(sockfd);
}