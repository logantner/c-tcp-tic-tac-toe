#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "client_controller.h"
#include "config.h"


int connect_client();


int run_client() {
    printf("Running the client controller...\n");

    int server_sockfd = connect_client();
    if (server_sockfd < 0) {
        printf("Failed to connect client to server. Check stderr log for more info\n");
        return 1;
    } else {
        printf("Client is now connected to the server!\n");
    }

    char readbuf[MAX_DATA_PACKET_SIZE];
    char* writebuf = malloc((MAX_DATA_PACKET_SIZE + 1) * sizeof(char));
    size_t max_user_len = MAX_DATA_PACKET_SIZE;
    ssize_t resp_len;
    int bytes_returned;
    int bytes_sent;

    while (1) {
        bytes_returned = recv(server_sockfd, readbuf, MAX_DATA_PACKET_SIZE, 0);
        if (bytes_returned > 0) {
            printf("Message of length %d from server: '%s'\n", bytes_returned, readbuf);
        } else if (bytes_returned == 0) {
            printf("Server his ended communication\n");
            break;
        } else {
            printf("Connection to serve was lost\n");
            break;
        }

        printf("What would you like to send to the server? ");
        resp_len = getline(&writebuf, &max_user_len, stdin);
        writebuf[strcspn(writebuf, "\n")] = 0;
        if (resp_len == 0 || strcmp(writebuf, "quit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        bytes_sent = send(server_sockfd, writebuf, strlen(writebuf), 0);

        // Clean out the buffers
        memset(writebuf, 0, strlen(writebuf));
        memset(readbuf, 0, strlen(readbuf));
    }

    free(writebuf);
    close(server_sockfd);
    return 0;
}


int connect_client() {
    struct addrinfo hint, *info_list;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    // Get server address info
    int error = getaddrinfo(SERVER_IP, SERVER_PORT, &hint, &info_list);
    if (error) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // Attempt to connect to server
    struct addrinfo* info;
    int sockfd;
    for (info = info_list; info != NULL; info = info->ai_next) {
        // create socket
        sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sockfd == -1) {
            fprintf(stderr, "failed to initialize client socket, reattempting with another address\n");
            continue;
        };

        if (connect(sockfd, info->ai_addr, info->ai_addrlen)) {
            close(sockfd);
            sockfd = -1;
            fprintf(stderr, "failed to connect client to server, reattempting with another address\n");
            continue;
        }

        break;
    }

    if (sockfd == -1) {
        fprintf(stderr, "address list exhausted - aborting connection to server\n");
    }

    freeaddrinfo(info_list);
    return sockfd;
}