#ifndef LOOT_H
#define LOOT_H

#include "main.h"
#include "maps.h"
#include "play.h"

#define UNSTACKABLE 0
#define STACKABLE 1

#define ITEM 0
#define GROUND 1
#define ROOF 2
#define WALL 3
#define C_DOOR 4
#define O_DOOR 5
#define HOLDER 6
#define NOITEM 7

struct loot {
	char *name;
	short int sprite;
	char stackable;
	char passable;
	char type;
};

extern short int	 get_loot_sprite(short int id);
extern char		 is_loot_passable(short int id);
extern char	 	*get_loot_name(short int id);
extern char 		 is_loot_stackable(short int id);
extern char		 get_loot_type(short int id);
extern void		 pickup_item(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern SDL_bool		 handle_pickup(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y);
extern void		 throw_item(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern SDL_bool		 handle_throw(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y, short int quantity);
extern void		 move_cursor(struct game *cur_game, char dir);
extern void		 swap_item(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern SDL_bool		 handle_swap(struct game *cur_game, struct player *cur_player, char start_pos);

#endif
