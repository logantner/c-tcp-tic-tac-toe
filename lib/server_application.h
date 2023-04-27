#ifndef SERVER_APPLICATION_H
#define SERVER_APPLICATION_H

#include "pres_layer.h"

struct player {
    int fd;
    char* name;
    char role;
};

struct ttt_game {
    struct player p1; // p1 is always X
    struct player p2; // p2 is always O
    char board[9];
    int is_active;
};

void set_up_game();
trans_code moderate_game();

#endif