#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>


#define MAX_BUFFER_SIZE 1024
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc!= 2){
    	fprintf(stderr,"usage : %s <port>\n",argv[0]);
    	exit(EXIT_FAILURE);
    }
    
    int sockfd, connfd;
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    //create socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) { 
        handle_error("fail to create socket");
    }

    //allow server在結束後立即重新綁定到相同的port上，而無需等待系統釋放port
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        handle_error("fail to setsockopt");
    }

    int port = atoi(argv[1]);
    // set the server info
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };
    // bind
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        handle_error("fail to bind");
    }
    //listen
    if (listen(sockfd, 10) < 0) {
        handle_error("fail to listen");
    }
    printf("Server listening on port %d...\n",port);
    // recieve client
    while (1) {
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd < 0) {
            perror("fail to accept from client");
            continue;
        }

       
        // Handle the new connection
        char buffer[MAX_BUFFER_SIZE] = {0};
        read(connfd, buffer, sizeof(buffer));
        printf("Received message from client: %s\n", buffer);

        // Send a response to the client
        const char *response = "Hello from server!";
        send(connfd, response, strlen(response), 0);

        // Close the socket for this client
        close(connfd);
    }
    return 0;
}
