#include <stdio.h>
#include <stdlib.h>
#include "maps.h"
#include "rand.h"
#include "town.h"

struct coords {
	int row;
	int col;
};

/* Function prototypes */
static void	expand_rooms(struct house *cur_house, int row, int col);
static int	count_potential_rooms(struct house *cur_house);
static void	rand_potential_room(struct house *cur_house, struct coords *pos, int count);
static int	maze_entrances(struct house *cur_house, int row, int col);
static void	remove_wall(struct house *cur_house, int row, int col, int count);
static int	count_rooms(struct house *cur_house);
static void	remove_random_walls(struct house *cur_house);
static int	room_near_wall(struct house *cur_house, int row, int col);

void
init_house(struct house *cur_house, int rows, int cols)
{
	int i, j;
	
	/* Set up map dimensions */
	cur_house->rows = rows;
	cur_house->cols = cols;
	/* Allocate memory and set to default grid values */
	cur_house->tiles = malloc(sizeof(*cur_house->tiles) * rows);
	for (i = 0; i < rows; i++) {
		*(cur_house->tiles + i) = malloc(sizeof(**cur_house->tiles) * cols);
		for (j = 0; j < cols; j++) {
			if (i % 2 == 1 && j % 2 == 1) {
				*(*(cur_house->tiles + i) + j) = ROOM_UNKNOWN;
			} else {
				*(*(cur_house->tiles + i) + j) = ROOM_WALL;
			}
		}
	}	
}

void
kill_house(struct house *cur_house)
{
	int i;

	/* Free memory */
	for (i = 0; i < cur_house->rows; i++) {
		free(*(cur_house->tiles + i));
	}
	free(cur_house->tiles);
}

void
populate_house(struct house *cur_house, int row_start, int col_start)
{
	int count;
	struct coords *room;
	
	/* Make sure the starting position is an UNKNOWN room or exit */
	if (*(*(cur_house->tiles + row_start) + col_start) != ROOM_UNKNOWN) {
		printf("Starting coordinates (row: %d, col: %d) should be odd.\n", row_start, col_start);
		exit(1);
	}
	/* Make the given room a room and expand potential rooms around it */
	*(*(cur_house->tiles + row_start) + col_start) = ROOM_OPEN;
	expand_rooms(cur_house, row_start, col_start);
	/* Loop as long as there are potential rooms */
	room = malloc(sizeof(*room));
	while ((count = count_potential_rooms(cur_house)) > 0) {
		/* Pick a random potential room */
		rand_potential_room(cur_house, room, count);
		/* Remove one of the walls around it */
		remove_wall(cur_house, room->row, room->col, maze_entrances(cur_house, room->row, room->col));
		/* Expand potential rooms around it */
		expand_rooms(cur_house, room->row, room->col);
		/* Random chance to exit loop */
		if (count_rooms(cur_house) >= 8 && rand_num(0, 99) >= 80) break;
	}
	/* Remove some random walls */
	remove_random_walls(cur_house);
	free(room);
}

void
place_house(struct worldmap *map, struct house *cur_house, int row, int col)
{
	int i, j, count;

	/* Check if you can place it */
	if (row + cur_house->rows > map->row_size - 1 ||
	    col + cur_house->cols > map->col_size - 1) {
	    	printf("House too big to fit on map at row %d, col %d\n", row, col);
	    	return;
	}
	//1 wall, 2 floor, 3 roof
	
	/* First, place all walls and floors */
	for (i = 0; i < cur_house->rows; i++) {
		for (j = 0; j < cur_house->cols; j++) {
			if (is_passable(*(*(map->tile + row + i) + col + j), *(*(map->biome + row + i) + col + j)) == IMPASSABLE) {
				/* Floor tile is impassable, make it the default tile for the biome */
				*(*(map->tile + row + i) + col + j) = 1;
			}
			if (*(*(cur_house->tiles+i)+j) == ROOM_OPEN) {
				*(*(map->ground + row + i) + col + j) = 2;
				*(*(map->roof + row + i) + col + j) = 3;
			} else if (*(*(cur_house->tiles+i)+j) == ROOM_WALL) {
				count = room_near_wall(cur_house, i, j);
				if (count > 0) {
					if (count == 3 && rand_num(0, 99) > 75) {
						*(*(map->ground + row + i) + col + j) = 2;
						*(*(map->loot + row + i) + col + j) = 4;
						*(*(map->quantity + row + i) + col + j) = 1;
					} else {
						*(*(map->ground + row + i) + col + j) = 2;
						*(*(map->loot + row + i) + col + j) = 1;
						*(*(map->quantity + row + i) + col + j) = 1;
						*(*(map->roof + row + i) + col + j) = 3;
					}
				}
			}
		}
	}
}

static void
expand_rooms(struct house *cur_house, int row, int col)
{
	/* Make all UNKNOWN rooms around given room POTENTIAL rooms */
	if (row > 2 && *(*(cur_house->tiles + row - 2) + col) == ROOM_UNKNOWN) {
		*(*(cur_house->tiles + row - 2) + col) = ROOM_POTENTIAL;
	}
	if (col < cur_house->cols - 2 && *(*(cur_house->tiles + row) + col + 2) == ROOM_UNKNOWN)  {
		*(*(cur_house->tiles + row) + col + 2) = ROOM_POTENTIAL;
	}
	if (row < cur_house->rows - 2 && *(*(cur_house->tiles + row + 2) + col) == ROOM_UNKNOWN) {
		*(*(cur_house->tiles + row + 2) + col) = ROOM_POTENTIAL;
	}
	if (col > 2 && *(*(cur_house->tiles + row) + col - 2) == ROOM_UNKNOWN) {
		*(*(cur_house->tiles + row) + col - 2) = ROOM_POTENTIAL;
	}
}

static int
count_potential_rooms(struct house *cur_house)
{
	int i, j;
	int count;
	
	for (i = 0, count = 0; i < cur_house->rows - 1; i++) {
		for (j = 0; j < cur_house->cols - 1; j++) {
			if (*(*(cur_house->tiles + i) + j) == ROOM_POTENTIAL) count++;
		}
	}
	return count;
}

static void
rand_potential_room(struct house *cur_house, struct coords *pos, int count)
{
	int i, j, room, room_count;
	
	/* Generate a random number based on count */
	room = rand_num(0, count - 1);
	room_count = 0;
	/* Cycle through all POTENTIAL rooms until you hit the random number */
	 for (i = 0; i < cur_house->rows; i++) {
	 	for (j = 0; j < cur_house->cols; j++) {
	 		if (*(*(cur_house->tiles + i) + j) == ROOM_POTENTIAL) {
	 			if (room_count == room) {
	 				pos->row = i;
	 		    		pos->col = j;
	 		    		return;
	 		    	} else {
		 			room_count++;
		 		}
	 		}
	 	}
	 }
}

static int
maze_entrances(struct house *cur_house, int row, int col)
{
	int count;
	
	/* Count number of ways into the open maze from this space */
	count = 0;
	if (row > 2 && *(*(cur_house->tiles + row - 2) + col) == ROOM_OPEN) count++;
	if (col < cur_house->cols - 2 && *(*(cur_house->tiles + row) + col + 2) == ROOM_OPEN) count++;
	if (row < cur_house->rows - 2 && *(*(cur_house->tiles + row + 2) + col) == ROOM_OPEN) count++;
	if (col > 2 && *(*(cur_house->tiles + row) + col - 2) == ROOM_OPEN) count++;
	return count;
}

static void
remove_wall(struct house *cur_house, int row, int col, int count)
{
	/* Remove a wall to open this room to the open maze */
	if (row > 2 && *(*(cur_house->tiles + row - 2) + col) == ROOM_OPEN &&
	    (count == 1 || rand_num(0, 99) == 99)) {
	    	*(*(cur_house->tiles + row) + col) = ROOM_OPEN;
	    	*(*(cur_house->tiles + row - 1) + col) = ROOM_OPEN;
	    	return;
	} else if (col < cur_house->cols - 2 && *(*(cur_house->tiles + row) + col + 2) == ROOM_OPEN &&
	    (count == 1 || rand_num(0, 99) == 99)) {
	    	*(*(cur_house->tiles + row) + col) = ROOM_OPEN;
	    	*(*(cur_house->tiles + row) + col + 1) = ROOM_OPEN;
	    	return;
	} else if (row < cur_house->rows - 2 && *(*(cur_house->tiles + row + 2) + col) == ROOM_OPEN &&
	    (count == 1 || rand_num(0, 99) == 99)) {
	    	*(*(cur_house->tiles + row) + col) = ROOM_OPEN;
	    	*(*(cur_house->tiles + row + 1) + col) = ROOM_OPEN;
	    	return;
	} else if (col > 2 && *(*(cur_house->tiles + row) + col - 2) == ROOM_OPEN &&
	    (count == 1 || rand_num(0, 99) == 99)) {
	    	*(*(cur_house->tiles + row) + col) = ROOM_OPEN;
	    	*(*(cur_house->tiles + row) + col - 1) = ROOM_OPEN;
	    	return;
	}
	/* Made it to the end without a jackpot winner, try again */
	remove_wall(cur_house, row, col, count);
}

static int
count_rooms(struct house *cur_house)
{
	int i, j, count;
	
	for (i = 1, count = 0; i < cur_house->rows; i += 2) {
		for (j = 1; j < cur_house->cols; j += 2) {
			if (*(*(cur_house->tiles + i) + j) == ROOM_OPEN) count++;
		}
	}
	return count;
}

static void
remove_random_walls(struct house *cur_house)
{
	int i, j;

	for (i = 1; i < cur_house->rows - 1; i++) {
		for (j = 1; j < cur_house->cols - 1; j++) {
			/* Only give a chance to remove the wall if it separates two ROOMS */
			if (*(*(cur_house->tiles + i) + j) == ROOM_WALL &&
			    ((*(*(cur_house->tiles + i - 1) + j) == ROOM_OPEN && *(*(cur_house->tiles + i + 1) + j) == ROOM_OPEN) ||
			     (*(*(cur_house->tiles + i) + j - 1) == ROOM_OPEN && *(*(cur_house->tiles + i) + j + 1) == ROOM_OPEN))) {
			    	if (rand_num(0, 99) >= 50) {
			    		*(*(cur_house->tiles + i) + j) = ROOM_OPEN;
			    	}
			}
		}
	}
}

static int
room_near_wall(struct house *cur_house, int row, int col)
{
	int i, j, count;
	
	/* Count number of ways into the open maze from this space */
	for (i = -1, count = 0; i < 2; i++) {
		for (j = -1; j < 2; j++) {
			if (row + i < 0 || row + i > cur_house->rows - 1 ||
			    col + j < 0 || col + j > cur_house->cols - 1) continue;
			if (*(*(cur_house->tiles + row + i) + col + j) == ROOM_OPEN) count++;
		}
	}
	return count;
}
