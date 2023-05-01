#ifndef SERVER_APPLICATION_H
#define SERVER_APPLICATION_H

#include "name_set_tools.h"
#include "presentation.h"
#include "game.h"
#include "name_set_tools.h"

void set_up_game(char*, char*);

trans_code process_new_player(int, struct ttt_game*);
trans_code moderate_game(struct ttt_game);
void post_game_cleanup(struct ttt_game, struct name_set*);
trans_code query_player_info(struct player*, struct ttt_game*, struct name_set*);

#endif