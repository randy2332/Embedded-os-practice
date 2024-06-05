#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888

#define MAX_BUFFER_SIZE 256

int main(int argc,char *argv[]) {
    if (argc!=6){
        printf("wrong enter number");
        exit(1);
    }
    char *server_ip = argv[1];
    int port  =atoi(argv[2]);
    char *operation = argv[3];
    int amount = atoi(argv[4]);
    int times = atoi(argv[5]);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER_SIZE] = {0};
    
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    snprintf(buffer,sizeof(buffer),"%s %d %d",operation ,amount,times);
    if (send(sock,buffer,strlen(buffer),0)==-1){
        printf("send fail");
        close(sock);
        exit(EXIT_FAILURE);
    }
    close(sock);
    return 0;
}

