#ifndef TOWN_H
#define TOWN_H

#include "maps.h"

struct house {
	int rows;
	int cols;
	int **tiles;
};

enum room { ROOM_WALL, ROOM_OPEN, ROOM_UNKNOWN, ROOM_POTENTIAL };

extern void	init_house(struct house *cur_house, int rows, int cols);
extern void	kill_house(struct house *cur_house);
extern void	populate_house(struct house *cur_house, int row_start, int col_start);
extern void	place_house(struct worldmap *map, struct house *cur_house, int row, int col);

#endif
