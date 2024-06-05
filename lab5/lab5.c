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



void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void serve_client(int connfd) {
    dup2(connfd, STDOUT_FILENO);
    close(connfd);
    printf("here\n");
    execlp("/usr/games/sl", "sl", "-l", NULL);
    handle_error("execlp");
}

int main(int argc, char *argv[]) {

    
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

    //  when child process is done, parent process recieve 'SIGCHLD'. 
    signal(SIGCHLD, handler);


    // recieve client
    while (1) {
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd < 0) {
            perror("fail to accept from client");
            continue;
        }

        pid_t childpid = fork();
        if (childpid == 0) {
            close(sockfd);
            serve_client(connfd);
            exit(EXIT_SUCCESS);
        } else if (childpid < 0) {
            handle_error("fail to fork");
        } else {
            printf("Train ID: %d\n", (int)childpid);
            close(connfd);
        }
    }

    close(sockfd);
    return 0;
}

   
