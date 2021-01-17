#include <stdio.h>
#include <stdlib.h>
#include "maps.h"
#include "rand.h"

/* Structure for tiles */
struct tile {
	int prob[9];
} TILES[9] = {
	{{0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 92, 5, 3, 0, 0, 0, 0, 0}},
	{{0, 23, 75, 0, 2, 0, 0, 0, 0}},
	{{0, 0, 0, 75, 0, 25, 0, 0, 0}},
	{{0, 0, 90, 0, 10, 0, 0, 0, 0}},
	{{0, 45, 0, 35, 0, 20, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

/* Function prototypes */
static int	calc_tiles(struct worldmap *map, struct tile **probs);
static void	calc_probs(struct worldmap *map, struct tile ***probs);

void
create_map(struct worldmap *map, int row_size, int col_size)
{
	int x, y;
	
	/* Save dimensions to structure */
	map->col_size = col_size;
	map->row_size = row_size;
	/* Allocate space for map and zero out */
	map->tile = malloc(sizeof(*(map->tile))*row_size);
	for (x = 0; x < row_size; x++) {
		*(map->tile+x) = malloc(sizeof(**map->tile)*col_size);
		for (y = 0; y < col_size; y++) {
			*(*(map->tile+x)+y) = 0;
		}
	}
}

void
free_map(struct worldmap *map)
{
	int x;
	
	/* Free memory allocated for the map */
	for (x = 0; x < map->row_size; x++) {
		free(*(map->tile+x));
	}
	free(map->tile);
}

void
populate_map(struct worldmap *map, int start_tile)
{
	int x, y, z;
	int zero_count;
	struct tile **probs;
	
	
	/* Set up nine initial tiles, at 1/4, 2/4, and 3/4 vertical and horizontal */
	for (x = 0; x < 3; x++) {
		*(*(map->tile+(x+1)*(map->row_size/4))+1*(map->col_size/4)) = start_tile;
		*(*(map->tile+(x+1)*(map->row_size/4))+2*(map->col_size/4)) = start_tile;
		*(*(map->tile+(x+1)*(map->row_size/4))+3*(map->col_size/4)) = start_tile;
	}
	/* Allocate memory for probabilities struct and zero out */
	probs = malloc(sizeof(*probs)*map->row_size);
	for (x = 0; x < map->row_size; x++) {
		*(probs+x) = malloc(sizeof(**probs)*map->col_size);
		for (y = 0; y < map->col_size; y++) {
			for (z = 0; z < 9; z++) {
				(*(*(probs+x)+y)).prob[z] = 0;
			}
		}
	}
	
	while(1) {
		/* Calculate all probabilities */
		calc_probs(map, &probs);
		/* Apply all probabilities */
		zero_count = calc_tiles(map, probs);
		if (zero_count == 0) {
			break;
		}
	}
	/* Free probabilties */
	for (x = 0; x < map->row_size; x++) {
		free(*(probs+x));
	}
	free(probs);
}

static void
calc_probs(struct worldmap *map, struct tile ***probs)
{
	int x, y, z;
	struct tile **prob;
	
	/* Dereferencing this to make it easier to work with */
	prob = *probs;
	
	/* Iterate through each tile of map and add probabilities */
	for (x = 0; x < map->row_size; x++) {
		for (y = 0; y < map->col_size; y++) {
			if (*(*(map->tile+x)+y) != 0) {
				for (z = 0; z < 9; z++) {
					if (x > 0) {
						/* tile to left */
						(*(*(prob+x-1)+y)).prob[z] = TILES[*(*(map->tile+x)+y)].prob[z];
					}
					if (x < map->row_size - 1) {
						/* tile to right */
						(*(*(prob+x+1)+y)).prob[z] = TILES[*(*(map->tile+x)+y)].prob[z];
					}
					if (y > 0) {
						/* tile to up */
						(*(*(prob+x)+y-1)).prob[z] = TILES[*(*(map->tile+x)+y)].prob[z];
					}
					if (y < map->col_size - 1) {
						/* tile to down */
						(*(*(prob+x)+y+1)).prob[z] = TILES[*(*(map->tile+x)+y)].prob[z];
					}
				}
			}
		}
	}
}

static int
calc_tiles(struct worldmap *map, struct tile **probs)
{
	int x, y, z;
	int zero_count;
	int prob_max;
	int prob_roll;
	
	/* Assume there are zero "zeroes" */
	zero_count = 0;
	/* Iterate through every tile of map, calculate new tile based on probability */
	for (x = 0; x < map->row_size; x++) {
		for (y = 0; y < map->col_size; y++) {
			if (*(*(map->tile+x)+y) == 0) {
				/* calculate max probability */
				prob_max = 0;
				for (z = 1; z < 9; z++) {
					prob_max += (*(*(probs+x)+y)).prob[z];
				}
				if (prob_max == 0) {
					/* There were no probabilities, this will remain a zero */
					zero_count++;
				} else {
					/* Roll probabilities */
					prob_roll = rand_num(0, prob_max - 1);
					/* Calculate which tile to apply */
					prob_max = 0;
					for (z = 1; z < 9; z++) {
						prob_max += (*(*(probs+x)+y)).prob[z];
						if (prob_roll < prob_max) {
							break;
						}
					}
					*(*(map->tile+x)+y) = z;
				}	
			}
		}
	}
	return zero_count;
}
