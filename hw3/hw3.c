#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_BUFFER_SIZE 256
#define NUM_DELIVERY_PERSONS 2

typedef struct {
    int total_cookie;
    int total_cake;
    int total_tea;
    int total_boba;
    int total_fried_rice;
    int total_egg_drop_soup;
    int distance;
} Order;

typedef struct {
    int available;
    time_t next_available_time;
    pthread_mutex_t mutex;
} DeliveryPerson;

DeliveryPerson delivery_persons[NUM_DELIVERY_PERSONS];


pthread_mutex_t mutex_time1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time2 = PTHREAD_MUTEX_INITIALIZER;

void handle_error(const char *msg);
void timer_handler(int signum);
void handle_shop_list(int client_socket);
void update_item_count(Order *order, const char *item, int quantity);
char* determine_restaurant(Order *order, const char *item);
int compute_price(Order *order, const char *restaurant);
void sendtoclient(Order *order, const char *restaurant, int client_socket);
void* handleclient(void* arg);

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void timer_handler(int signum){
    static int count = 0;
    printf("time expired %d times\n", ++count);
}

void handle_shop_list(int client_socket) {
    const char *response = "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n";
    send(client_socket, response, strlen(response), 0);
}

void update_item_count(Order *order, const char *item, int quantity) {
    if (strcmp(item, "cookie") == 0) {
        order->total_cookie += quantity;
    } else if (strcmp(item, "cake") == 0) {
        order->total_cake += quantity;
    } else if (strcmp(item, "tea") == 0) {
        order->total_tea += quantity;
    } else if (strcmp(item, "boba") == 0) {
        order->total_boba += quantity;
    } else if (strcmp(item, "fried-rice") == 0) {
        order->total_fried_rice += quantity;
    } else if (strcmp(item, "Egg-drop-soup") == 0) {
        order->total_egg_drop_soup += quantity;
    }
}

char* determine_restaurant(Order *order, const char *item) {
    static char restaurant[MAX_BUFFER_SIZE] = {0};

    if (strcmp(item, "cookie") == 0 || strcmp(item, "cake") == 0) {
        strcpy(restaurant, "Dessert shop");
        order->distance = 3;
    } else if (strcmp(item, "tea") == 0 || strcmp(item, "boba") == 0) {
        strcpy(restaurant, "Beverage shop");
        order->distance = 5;
    } else if (strcmp(item, "fried-rice") == 0 || strcmp(item, "Egg-drop-soup") == 0) {
        strcpy(restaurant, "Diner");
        order->distance = 8;
    } else {
        strcpy(restaurant, "Unknown");
    }

    return restaurant;
}

int compute_price(Order *order, const char *restaurant) {
    int price = 0;
    if (strcmp(restaurant, "Dessert shop") == 0) {
        price = order->total_cookie * 60 + order->total_cake * 80;
    } else if (strcmp(restaurant, "Beverage shop") == 0) {
        price = order->total_tea * 40 + order->total_boba * 70;
    } else if (strcmp(restaurant, "Diner") == 0) {
        price = order->total_fried_rice * 120 + order->total_egg_drop_soup * 50;
    }
    return price;
}

void sendtoclient(Order *order, const char *restaurant, int client_socket) {
    char message[MAX_BUFFER_SIZE] = {0};
    int has_reserved_items = 0;

    if (strcmp(restaurant, "Dessert shop") == 0) {
        if (order->total_cookie > 0) {
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "cookie %d", order->total_cookie);
            has_reserved_items = 1;
        }
        if (order->total_cake > 0) {
            if (has_reserved_items) {
                snprintf(message + strlen(message), sizeof(message) - strlen(message), "|");
            }
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "cake %d", order->total_cake);
            has_reserved_items = 1;
        }
    } else if (strcmp(restaurant, "Beverage shop") == 0) {
        if (order->total_tea > 0) {
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "tea %d", order->total_tea);
            has_reserved_items = 1;
        }
        if (order->total_boba > 0) {
            if (has_reserved_items) {
                snprintf(message + strlen(message), sizeof(message) - strlen(message), "|");
            }
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "boba %d", order->total_boba);
            has_reserved_items = 1;
        }
    } else if (strcmp(restaurant, "Diner") == 0) {
        if (order->total_fried_rice > 0) {
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "fried_rice %d", order->total_fried_rice);
            has_reserved_items = 1;
        }
        if (order->total_egg_drop_soup > 0) {
            if (has_reserved_items) {
                snprintf(message + strlen(message), sizeof(message) - strlen(message), "|");
            }
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "Egg_drop_soup %d", order->total_egg_drop_soup);
            has_reserved_items = 1;
        }
    }

    if (has_reserved_items) {
        strcat(message, "\n");
        send(client_socket, message, MAX_BUFFER_SIZE, 0);
    }
}

void* handleclient(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[MAX_BUFFER_SIZE] = {0};
    char response[MAX_BUFFER_SIZE] = {0};
    Order order = {0};
    char restaurant[MAX_BUFFER_SIZE] = {0};
    int firstorder = 1;
    int assigned_delivery_person = -1;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));

        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break;
        }

        printf("from client : %s\n", buffer);
        if (strncmp(buffer, "order", 5) == 0) {
            char item[MAX_BUFFER_SIZE] = {0};
            int quantity = 0;
            sscanf(buffer, "order %s %d", item, &quantity);

            if (firstorder == 1) {
                strcpy(restaurant, determine_restaurant(&order, item));
                firstorder = 0;
            }

            update_item_count(&order, item, quantity);
            printf("Received order: Item: %s, Quantity: %d\n", item, quantity);
            sendtoclient(&order, restaurant, client_socket);

        } else if (strncmp(buffer, "shop list", 9) == 0) {
            handle_shop_list(client_socket);

        } else if (strncmp(buffer, "confirm", 7) == 0) {
            if (firstorder == 1) {
                snprintf(response, sizeof(response), "Please order some meals\n");
                send(client_socket, response, MAX_BUFFER_SIZE, 0);
            } else {
                int deliver = -1;
               
                int total_price = compute_price(&order, restaurant);
                int waittime=0;
                // find first deliver person 1 first
                if( (delivery_persons[0].next_available_time <= delivery_persons[1].next_available_time)){
                
                        printf("Locked the mutex for delivery person 1\n");
                	pthread_mutex_lock(&delivery_persons[0].mutex);
                        deliver = 0;
                        if ((delivery_persons[0].next_available_time + order.distance) > 30) {
                        	printf("deliver 1 exceed 30 sec \n");
		                snprintf(response, sizeof(response), "Your delivery will take a long time, do you want to wait?\n");
		                send(client_socket, response, MAX_BUFFER_SIZE, 0);
		                read(client_socket, buffer, MAX_BUFFER_SIZE);
		                if (strncmp(buffer, "No", 2) == 0) {
		                    pthread_mutex_unlock(&delivery_persons[0].mutex);
		                    break;
		                } else if (strncmp(buffer, "Yes", 3) == 0) {
		                    delivery_persons[0].next_available_time += order.distance;
		                    waittime = delivery_persons[0].next_available_time;
		                    
		                    pthread_mutex_unlock(&delivery_persons[0].mutex);
		                }
		                else {
		                    printf("other response (not yes or no)\n");
		                }
		                    
		        }
                            
                        else{
                                delivery_persons[0].next_available_time += order.distance;
                                waittime = delivery_persons[0].next_available_time;
                        	
                        }
                        pthread_mutex_unlock(&delivery_persons[0].mutex);
                        printf("unLocked the mutex for delivery person 1\n");
                    
                }
                        
                
                else { //deliver peron 2
                        printf("Locked the mutex for delivery person 2\n");
			pthread_mutex_lock(&delivery_persons[1].mutex);
                    	deliver = 1;
                        if ((delivery_persons[1].next_available_time + order.distance) > 30) {
                                printf("deliver 2 exceed 30 sec \n");
		                snprintf(response, sizeof(response), "Your delivery will take a long time, do you want to wait?\n");
		                send(client_socket, response, MAX_BUFFER_SIZE, 0);
		                read(client_socket, buffer, MAX_BUFFER_SIZE);
		                if (strncmp(buffer, "No", 2) == 0) {
		                    pthread_mutex_unlock(&delivery_persons[1].mutex);
		                    break;
		                } else if (strncmp(buffer, "Yes", 3) == 0) {
		                    delivery_persons[1].next_available_time += order.distance;
		                    waittime = delivery_persons[1].next_available_time;
		                    pthread_mutex_unlock(&delivery_persons[1].mutex);
		                  
		                    
		                    pthread_mutex_lock(&mutex_time1);
				  
		                }
		                else{
		                    printf("other response (not yes or no)\n");
		                }
		                	
                            }
                        else{
                                delivery_persons[1].next_available_time += order.distance;
                                waittime = delivery_persons[1].next_available_time;
                        	
                        }
                        pthread_mutex_unlock(&delivery_persons[1].mutex);
                        printf("unLocked the mutex for delivery person 2\n");
                }    
                //do sleep thing
               
                snprintf(response, sizeof(response), "Please wait a few minutes...\n");
                send(client_socket, response, MAX_BUFFER_SIZE, 0); 
                
               
                if(deliver==0) {
                	pthread_mutex_lock(&mutex_time1);
		        for(int i=0;i<order.distance;i++)
		        {
		            sleep(1);
		            pthread_mutex_lock(&delivery_persons[deliver].mutex);
		            delivery_persons[deliver].next_available_time--;
		            pthread_mutex_unlock(&delivery_persons[deliver].mutex);
		        }
		        pthread_mutex_unlock(&mutex_time1);
                }
                else if(deliver==1){
                	pthread_mutex_lock(&mutex_time2);
		        for(int i=0;i<order.distance;i++)
		        {
		            sleep(1);
		            pthread_mutex_lock(&delivery_persons[deliver].mutex);
		            delivery_persons[deliver].next_available_time--;
		            pthread_mutex_unlock(&delivery_persons[deliver].mutex);
		        }
		        pthread_mutex_unlock(&mutex_time2);
                }
                else {
                    printf("deliver -1\n");
                }
                
             
               
           
               
                snprintf(response, sizeof(response), "Delivery has arrived and you need to pay %d$\n", total_price);
                send(client_socket, response, MAX_BUFFER_SIZE, 0);
                
                break;
                
                
             
            }
        } else if (strncmp(buffer, "cancel", 6) == 0) {
            break;
        } else {
            snprintf(response, sizeof(response), "Unknown request\n");
            send(client_socket, response, MAX_BUFFER_SIZE, 0);
        }
    }

    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_DELIVERY_PERSONS; i++) {
        delivery_persons[i].available = 1;
        delivery_persons[i].next_available_time = 0;
        pthread_mutex_init(&delivery_persons[i].mutex, NULL);
    }
    pthread_mutex_init(&mutex_time1,NULL);
    pthread_mutex_init(&mutex_time2,NULL);
    int server_fd, *connfd;
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) handle_error("fail to create socket");

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) handle_error("fail to setsockopt");

    int port = atoi(argv[1]);
    struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(port)};
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) handle_error("fail to bind");

    if (listen(server_fd, 10) < 0) handle_error("fail to listen");
    printf("Server listening on port %d...\n", port);

    while (1) {
        connfd = malloc(sizeof(int));
        if (connfd == NULL) {
            perror("fail to allocate memory for connection socket");
            continue;
        }
        *connfd = accept(server_fd, (struct sockaddr *)&addr_cln, &sLen);
        if (*connfd < 0) {
            perror("fail to accept from client");
            free(connfd);
            continue;
        }
        printf("Client connected\n");

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handleclient, connfd) != 0) {
            perror("fail to create thread");
            close(*connfd);
            free(connfd);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_fd);
    for (int i = 0; i < NUM_DELIVERY_PERSONS; i++) {
        pthread_mutex_destroy(&delivery_persons[i].mutex);
    }
    pthread_mutex_destroy(&mutex_time1);
    pthread_mutex_destroy(&mutex_time2);
    return 0;
}

