#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#define PORT 8888
#define MAX_CLIENTS 100
#define BUFFER_SIZE 256

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[MAX_CLIENTS];
int client_sockets[MAX_CLIENTS];
int num_clients = 0;
int server_fd;
int account_balance = 0;  // Shared resource: account balance

void handle_sigint(int sig) {
    printf("Shutting down server...\n");
    close(server_fd);
    // wait for thread to complete
    for (int i = 0; i < num_clients; ++i) {
        close(client_sockets[i]);
    }
    //destroy
    pthread_mutex_destroy(&mutex);
    exit(0);
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    char operation[20];
    int amount, times, bytes_read;

    // Read client request
    if ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        sscanf(buffer, "%s %d %d", operation, &amount, &times);

        for (int i = 0; i < times; ++i) {
            // Lock the mutex before accessing shared resource
            pthread_mutex_lock(&mutex);

            if (strcmp(operation, "deposit") == 0) {
                account_balance += amount;
                printf("Deposited %d\n",   account_balance);
            } else if (strcmp(operation, "withdraw") == 0) {
                if (account_balance >= amount) {
                    account_balance -= amount;
                    printf("Withdrew %d\n", account_balance);
                } else {
                    printf("Withdrawal of %d failed, insufficient funds. Current balance: %d\n", amount, account_balance);
                }
            }

            // Unlock the mutex after accessing shared resource
            pthread_mutex_unlock(&mutex);
        }
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    signal(SIGINT, handle_sigint);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&mutex);
        if (num_clients < MAX_CLIENTS) {
            client_sockets[num_clients] = client_socket;
            if (pthread_create(&threads[num_clients], NULL, handle_client, &client_sockets[num_clients]) != 0) {
                perror("pthread_create failed");
            } else {
                num_clients++;
                printf("Client connected\n");
            }
        } else {
            printf("Maximum clients reached. Connection rejected.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    return 0;
}

