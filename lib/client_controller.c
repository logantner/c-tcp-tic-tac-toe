#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "client_controller.h"
#include "command.h"
#include "config.h"
#include "game.h"
#include "presentation.h"


int connect_client();
trans_code get_server_cmd(int, struct command*, char*);
trans_code take_turn(int, char*, char*, int*);
trans_code move_sequence(int, char*, char*, int*);


int run_client() {
    printf("Running the client controller...\n");

    int server_sockfd = connect_client();
    if (server_sockfd < 0) {
        printf("Failed to connect client to server. Check stderr log for more info\n");
        return 1;
    } else {
        printf("Client is now connected to the server!\n");
    }

    play_game(server_sockfd);

    // char readbuf[MAX_DATA_PACKET_SIZE];
    // char* writebuf = malloc((MAX_DATA_PACKET_SIZE + 1) * sizeof(char));
    // size_t max_user_len = MAX_DATA_PACKET_SIZE;
    // ssize_t resp_len;
    // int bytes_returned;
    // int bytes_sent;

    // while (1) {
    //     bytes_returned = recv(server_sockfd, readbuf, MAX_DATA_PACKET_SIZE, 0);
    //     if (bytes_returned > 0) {
    //         printf("Message of length %d from server: '%s'\n", bytes_returned, readbuf);
    //     } else if (bytes_returned == 0) {
    //         printf("Server his ended communication\n");
    //         break;
    //     } else {
    //         printf("Connection to serve was lost\n");
    //         break;
    //     }

    //     printf("What would you like to send to the server? ");
    //     resp_len = getline(&writebuf, &max_user_len, stdin);
    //     writebuf[strcspn(writebuf, "\n")] = 0;
    //     if (resp_len == 0 || strcmp(writebuf, "quit") == 0) {
    //         printf("Goodbye!\n");
    //         break;
    //     }
    //     bytes_sent = send(server_sockfd, writebuf, strlen(writebuf), 0);

    //     // Clean out the buffers
    //     memset(writebuf, 0, strlen(writebuf));
    //     memset(readbuf, 0, strlen(readbuf));
    // }

    // free(writebuf);
    // close(server_sockfd);
    // return 0;

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

void play_game(int server_fd) {
    char name[100];
    char role[5];
    trans_code tcode;
    struct command in_cmd;
    char msg_fragment[MAX_DATA_PACKET_SIZE];
    int game_over = 0;

    printf("What is your name: ");
    fgets(name, 100, stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Thanks, %s! Requesting the server to pair you to a game.\n", name);
    tcode = send_command(server_fd, new_play_cmd(name));
    printf("Sent your name to the server, let's see what happens...\n");
    
    if (tcode != TRANS_OK) {
        printf("Unfortunately, there was a connection issue. Quitting now...\n");
        return;
    }

    // Get the WAIT command from the server
    if (get_server_cmd(server_fd, &in_cmd, msg_fragment) != TRANS_OK) {
        return;
    }
    free_cmd(in_cmd);

    // Get the BEGN command from the server
    if (get_server_cmd(server_fd, &in_cmd, msg_fragment) != TRANS_OK) {
        return;
    }

    if (strcmp(in_cmd.arg1, "X") == 0) {
        printf("You are player X, and it's your turn now\n");
        strcpy(role, "X");
        tcode = take_turn(server_fd, role, msg_fragment, &game_over);
        if (tcode == SEND_FAILED) {
            printf("Send failure: quitting\n");
            free_cmd(in_cmd);
            return;
        }
    } else if (strcmp(in_cmd.arg1, "O") == 0) {
        printf("You are player O. Waiting for your turn...\n");
        strcpy(role, "O");
    } else {
        printf("I'm not sure what role '%s' is supposed to be...\n", in_cmd.arg1);
    }

    free_cmd(in_cmd);

    // START THE GAME LOOP
    while (!game_over) {
        // Get server response
        if (get_server_cmd(server_fd, &in_cmd, msg_fragment) != TRANS_OK) {
            free_cmd(in_cmd);
            return;
        }

        if (in_cmd.code == MOVD) {
            display_board(in_cmd.arg3);
        }

        if (in_cmd.code == OVER) {
            game_over = 1;
        }

        free_cmd(in_cmd);

        if (!game_over && take_turn(server_fd, role, msg_fragment, &game_over) != TRANS_OK) {
            break;
        }
    }
    
}



trans_code take_turn(int fd, char* role, char* frag, int* game_over) {
    char resp[100];
    trans_code tcode;
    struct command in_cmd;

    printf("Would you like to MOVE, DRAW or RSGN? ");
    fgets(resp, 100, stdin);
    resp[strcspn(resp, "\n")] = 0;
    if (strcmp(resp, "MOVE") == 0) {
        tcode = move_sequence(fd, role, frag, game_over);
    } else if (strcmp(resp, "DRAW") == 0) {
        printf("And what type of draw request would you like? ");
        memset(resp, 0, strlen(resp));
        fgets(resp, 100, stdin);
        resp[strcspn(resp, "\n")] = 0;
        printf("Ok, requesting a draw from the opponent. Sit tight...\n");
        tcode = send_command(fd, new_draw_cmd(resp));
    } else if (strcmp(resp, "RSGN") == 0) {
        printf("Ok, just letting your opponent know that you are a bitch...\n");
        tcode = send_command(fd, new_rsgn_cmd());
    } else if (strcmp(resp, "WAIT") == 0) {
        printf("Ok, sending an invalid message to server\n");
        tcode = send_command(fd, new_wait_cmd());
    } else {
        printf("Invalid response.\n");
        tcode = take_turn(fd, role, frag, game_over);
    }
    return tcode;
}

trans_code move_sequence(int fd, char* role, char* frag, int* game_over) {
    char resp[100];
    trans_code tcode;
    struct command in_cmd;

    printf("Which ('R,C') space would you like to choose? ");
    fgets(resp, 100, stdin);
    resp[strcspn(resp, "\n")] = 0;
    printf("Ok, requesting server to move %s to space (%s)...", role, resp);

    tcode = send_command(fd, new_move_cmd(role, resp[0]-'0', resp[2]-'0'));

    if (tcode != TRANS_OK) {
        printf("Send failure\n");
        return tcode;
    }

    tcode = get_server_cmd(fd, &in_cmd, frag);
    if (tcode != TRANS_OK) {
        printf("Read failure\n");
        free_cmd(in_cmd);
        return tcode;
    }

    if (in_cmd.code == INVL) {
        printf("This is an illegal move. Try again\n");
        free_cmd(in_cmd);
        move_sequence(fd, role, frag, game_over);
    } else {
        if (in_cmd.code == MOVD) {
            printf("Your move was accepted. Waiting for your opponent to move...\n");
        }
        free_cmd(in_cmd);
    }

    return TRANS_OK;
}

trans_code get_server_cmd(int fd, struct command* in_cmd, char* msg_fragment) {
    char errmsg[300];
    trans_code tcode = read_command(fd, in_cmd, msg_fragment, errmsg);
    if (tcode != TRANS_OK) {
        printf("Received the following trans_code: %s\n", trans_code_to_str(tcode));
        printf("Error message: %s\n", errmsg);
        printf("Unfortunately, there was a connection issue. Quitting now...\n");
    }
    else {
        printf("Received the following message from the server:\n");
        display_cmd(*in_cmd);
    }
    return tcode;
}