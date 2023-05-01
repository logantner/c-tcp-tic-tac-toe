#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <signal.h>

#include "config.h"
#include "game.h"
#include "presentation.h"
#include "server_application.h"

int create_listening_socket(char*);
int extract_port(const struct addrinfo* const);
void display_addrinfo(struct addrinfo*);
void talk_to_client(int);

void stop_server(int);
void install_handlers();
int is_active_server = 1;


int run_server(char* ip_address, char* port) {
    printf("Starting server:\n");

    int conn_sockfd = create_listening_socket(port);
    if (conn_sockfd < 0) {
       printf("Failed to set up server listener. Check stderr for more info\n");
       return 1;
    }

    // install_handlers();

    // Listen and accept new connections
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    int client_sockfd;

    struct ttt_game game = new_game();
    trans_code tcode;

    while(is_active_server) {

        client_addr_size = sizeof(client_addr);
        client_sockfd = accept(conn_sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sockfd < 0) {
            perror("accept");
            continue;
        }

        printf("Received client with fd value %d\n", client_sockfd);

        /////////////////////
        // Modularize this //
        /////////////////////

        tcode = process_new_player(client_sockfd, &game);
        if (tcode != TRANS_OK) {
            printf("There were problems with the client at socket %d. Disconnecting...", client_sockfd);
            close(client_sockfd);
        }

        printf("The current game has %d players\n", num_players(game));

        if (num_players(game) == 2) {
            printf("Players at ports %d and %d have been matched to a game.\n", game.p1.fd, game.p2.fd);
            tcode = moderate_game(game);
            printf("Game has conclude. Performing post-game cleanup\n");
            post_game_cleanup(game);
            game = new_game();
            printf("Cleanup complete. Free to wait for new players to join\n");
        }

        /////////////////////
        /////////////////////
        /////////////////////
    }

    printf("Shutting down server...");
    if (conn_sockfd > 0) {
        close(conn_sockfd);
    }

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