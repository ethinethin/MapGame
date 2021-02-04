#ifndef LOOT_H
#define LOOT_H

#include "main.h"
#include "maps.h"
#include "play.h"

#define UNSTACKABLE 0
#define STACKABLE 1

struct loot {
	char *name;
	short int sprite;
	char stackable;
	char passable;
};

extern short int	 get_loot_sprite(short int id);
extern char		 is_loot_passable(short int id);
extern char	 	*get_loot_name(short int id);
extern char 		 is_loot_stackable(short int id);
extern void		 pickup_item(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void		 throw_item(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void		 move_cursor(struct game *cur_game, char dir);
extern void		 move_cursor_click(struct game *cur_game, int x);
extern void		 swap_item(struct game *cur_game, struct worldmap *map, struct player *cur_player);

#endif
