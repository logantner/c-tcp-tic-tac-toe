#ifndef SERVER_APPLICATION_H
#define SERVER_APPLICATION_H

#include "presentation.h"
#include "game.h"

void set_up_game(char*, char*);

trans_code process_new_player(int, struct ttt_game*);
trans_code moderate_game(struct ttt_game);
void post_game_cleanup(struct ttt_game);

#endif