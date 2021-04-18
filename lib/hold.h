#ifndef HOLD_H
#define HOLD_H

#include <SDL2/SDL.h>
#include "main.h"
#include "maps.h"
#include "play.h"

extern void		setup_hold(void);
extern void		add_hold(int x, int y, unsigned short int *loot, unsigned short int *quantity);
extern void		del_hold(int x, int y);
extern void		kill_hold(void);
extern void		dump_holders(struct worldmap *map, FILE *fp);
extern void		undump_holders(struct worldmap *map, FILE *fp);
extern void	 	open_chest(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y);
extern SDL_bool		chest_empty(int x, int y);

#endif
