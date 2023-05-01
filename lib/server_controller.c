#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include "config.h"
#include "game.h"
#include "presentation.h"
#include "server_application.h"
#include "name_set_tools.h"

int create_listening_socket(char*);
int extract_port(const struct addrinfo* const);
void display_addrinfo(struct addrinfo*);
void talk_to_client(int);

// Thread worker functions
void* execute_game_worker(void*);
void* query_player_worker(void*);

// Signal functions
void stop_server(int);
void install_handlers();

struct name_set name_set;
int is_active_server = 1;

struct clients {
    int fd1;
    int fd2;
};

struct query_player_work_data {
    int pfd;
    struct ttt_game* game;
    trans_code ret_tcode;
};


int run_server(char* ip_address, char* port) {
    printf("Starting server:\n");

    int conn_sockfd = create_listening_socket(port);
    if (conn_sockfd < 0) {
       printf("Failed to set up server listener. Check stderr for more info\n");
       return 1;
    }

    install_handlers();
    
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    int client_sockfd;
    
    int standby_clientfd = 0;
    trans_code tcode;
    name_set = new_name_set();

    while(is_active_server) {
        // Listen for and accept new connections
        client_addr_size = sizeof(client_addr);
        client_sockfd = accept(conn_sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sockfd < 0) {
            perror("accept");
            continue;
        }

        printf("Received client, assigned to fd %d\n", client_sockfd);

        if (standby_clientfd == 0) {
            standby_clientfd = client_sockfd;
        } else {
            pthread_t thid;

            struct clients* clients = malloc(sizeof(struct clients));
            clients->fd1 = client_sockfd;
            clients->fd2 = standby_clientfd;
            
            printf("Clients at sockets %d and %d have been paired to a game\n", clients->fd1, clients->fd2);

            if (pthread_create(&thid, NULL, execute_game_worker, clients)) {
                perror("pthread_create()");
            }

            standby_clientfd = 0;
        }
    }

    printf("Shutting down server...");
    if (conn_sockfd > 0) {
        close(conn_sockfd);
    }
    free_name_set(name_set);

    return 0;
}

void* execute_game_worker(void* _args) {
    // Extract info from _args
    struct clients clients = *((struct clients*)_args);
    int cfd1 = clients.fd1;
    int cfd2 = clients.fd2;
    free(_args);

    struct ttt_game game = new_game(cfd1, cfd2);

    // Initialize threads for querying player names
    pthread_t p1_thread, p2_thread;
    trans_code tcode1, tcode2;
    struct query_player_work_data p1_data = {cfd1, &game, tcode1};
    struct query_player_work_data p2_data = {cfd2, &game, tcode2};

    // Query players for names in parallel
    if (  pthread_create(&p1_thread, NULL, query_player_worker, &p1_data) || 
          pthread_create(&p2_thread, NULL, query_player_worker, &p2_data)) {
        perror("pthread_create()");
        return NULL;
    }

    // Wait until both players have finished submitting their names
    int jc1 = pthread_join(p1_thread, NULL);
    int jc2 = pthread_join(p2_thread, NULL);
    if (jc1 || jc2) {
        perror("pthread_join()");
        return NULL;
    }

    printf("Completed name collection. Current players: \n");
    display_player(game.p1);
    display_player(game.p2);

    moderate_game(game);

    post_game_cleanup(game, &name_set);
    return NULL;
}

void* query_player_worker(void* _args) {
    struct query_player_work_data* args = (struct query_player_work_data*) _args;
    printf("Inside query_player_worker for player at port %d\n", args->pfd);

    struct player* p;
    if (args->game->p1.fd == args->pfd) {
        p = &(args->game->p1);
    } else {
        p = &(args->game->p2);
    }

    args->ret_tcode = query_player_info(p, args->game, &name_set);
    return 0;
}

void stop_server(int signum) {
    is_active_server = 0;
}

void install_handlers() {
    struct sigaction act;
    act.sa_handler = stop_server;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}

int create_listening_socket(char* port) {
    struct addrinfo hint, *info_list;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags    = AI_PASSIVE;

    int error = getaddrinfo(NULL, port, &hint, &info_list);
    if (error) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    struct addrinfo* info;
    int sockfd;
    for (info = info_list; info != NULL; info = info->ai_next) {
        printf("=== Attempting to connect to the following socket:\n");
        // display_addrinfo(info);

        // create socket
        sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sockfd == -1) {
            fprintf(stderr, "failed to initialize connection socket, reattempting with another address\n");
            continue;
        };

        // bind socket to port
        if ( bind(sockfd, info->ai_addr, info->ai_addrlen) ) {
            fprintf(stderr, "failed to bind connection socket, reattempting with another address\n");
            close(sockfd);
            sockfd = -1;
            continue;
        }

        // start listening on port
        if ( listen(sockfd, SERVER_QUEUE_SIZE) ) {
            fprintf(stderr, "failed to attach listener to socket, reattempting with another address\n");
            close(sockfd);
            sockfd = -1;
            continue;
        }

        // if we got this far, we have opened the socket
        int port = extract_port(info);
        if (port == -1) {
            close(sockfd);
            sockfd = -1;
            continue;
        }

        printf("Server is now listening on port %d...\n", port);
        // display_addrinfo(info);
        break;
    }

    freeaddrinfo(info_list);

    if (sockfd < 0) {
        fprintf(stderr, "address list exhausted - aborting socket listener setup\n");
    }

    return sockfd;
}

void display_addrinfo(struct addrinfo* addr) {
    char ip_str[INET6_ADDRSTRLEN];
    char* ip_family;
    int port;

    if (addr->ai_family == AF_INET) {
        ip_family = "IPv4";
        struct sockaddr_in* sin = (struct sockaddr_in*) addr->ai_addr;
        inet_ntop(AF_INET, &(sin->sin_addr), ip_str, sizeof(ip_str));
        port = ntohs(sin->sin_port);
    } else if (addr->ai_family == AF_INET6) {
        ip_family = "IPv6";
        struct sockaddr_in6* sin = (struct sockaddr_in6*) addr->ai_addr;
        inet_ntop(AF_INET6, &(sin->sin6_addr), ip_str, sizeof(ip_str));
        port = ntohs(sin->sin6_port);
    } else {
        printf("The addrinfo family has an unexpected value: %d\n", addr->ai_family);
        return;
    }

    printf("Address info:\n");
    printf("    Family:      %s\n", ip_family);
    printf("    IP Address:  %s\n", ip_str);
    printf("    Port:        %d\n", port);
}

int extract_port(const struct addrinfo* const addr) {
    int port;
    if (addr->ai_family == AF_INET) {
        struct sockaddr_in* sin = (struct sockaddr_in*) addr->ai_addr;
        port = ntohs(sin->sin_port);
    } else if (addr->ai_family == AF_INET6) {
        struct sockaddr_in6* sin = (struct sockaddr_in6*) addr->ai_addr;
        port = ntohs(sin->sin6_port);
    } else {
        port = -1;
    }
    return port;
}