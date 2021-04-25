#ifndef FILE_H
#define FILE_H

#include "main.h"
#include "maps.h"
#include "play.h"

#define STARTING_GAME 0
#define CONTINUE_GAME 1
#define GAME_IN_PROGRESS 2
#define QUITTING_GAME 3

extern void	title_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player, int status);
extern SDL_bool	yesno_screen(struct game *cur_game, char *message);

#endif
