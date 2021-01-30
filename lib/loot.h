#ifndef LOOT_H
#define LOOT_H

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

#endif
