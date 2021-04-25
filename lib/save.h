#ifndef SAVE_H
#define SAVE_H

#include "main.h"
#include "maps.h"
#include "play.h"

extern SDL_bool	*check_savefiles(void);
extern SDL_bool	 save_all(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save);
extern void	 load_all(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save);

#endif
