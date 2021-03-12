#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "harv.h"
#include "loot.h"
#include "maps.h"
#include "rand.h"

/* Colors for tiles */
char red[3] = { 255, 0, 0 };
char green[3] = { 0, 192, 0 };
char lightgreen[3] = { 0, 255, 0 };
char medgreen[3] = { 0, 144, 0 };
char darkgreen[3] = { 0, 64, 0 };
char blue[3] = { 0, 0, 255 };
char darkblue[3] = { 0, 0, 128 };
char grey[3] = { 144, 144, 144 };
char yellow[3] = { 192, 192, 0 };
char khaki[3] = { 240, 230, 140 };
char cyan[3] = { 0, 255, 255 };
char white[3] = { 255, 255, 255 };
char black[3] = { 26, 26, 26 };

/* Custom grassland colors */
char grass[3] = {0x19, 0xB3, 0x4D};
char tall_grass[3] = {0x15, 0x95, 0x40};
char brush[3] = {0x11, 0x77, 0x33};
char tall_brush[3] = {0x0D, 0x59, 0x26};
char rocky_grass[3] = {0x6A, 0xEA, 0x95};
char big_rock[3] = {0xA6, 0xF2, 0xBF};
char puddles[3] = {0x88, 0xEE, 0xAA};

/* Structure for tiles */
struct biome {
	char *name;
	struct tile {
		char *name;
		short int sprite;
		char *col;
		char passable;
		short int prob[9];
	} tiles[9];
};

/* Biomes */
struct biome BIOMES[] = {{
	"grassland",
	{
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{"grass", 512, grass, PASSABLE, {0, 80, 8, 5, 0, 5, 0, 2, 0}},
		{"tall grass", 514, tall_grass, PASSABLE, {0, 10, 80, 4, 4, 2, 0, 0, 0}},
		{"brush", 516, brush, PASSABLE, {0, 10, 20, 50, 15, 5, 0, 0, 0}},
		{"tall brush", 518, tall_brush, PASSABLE, {0, 5, 10, 25, 50, 10, 0, 0, 0}},
		{"rocky grass", 520, rocky_grass, PASSABLE, {0, 10, 15, 15, 5, 50, 5, 0, 0}},
		{"big rock", 522, big_rock, IMPASSABLE, {0, 20, 20, 20, 20, 20, 0, 0, 0}},
		{"puddles", 524, puddles, PASSABLE, {0, 75, 7, 3, 0, 0, 0, 10, 5}},
		{"swamp", 526, puddles, IMPASSABLE, {0, 30, 7, 3, 0, 0, 0, 50, 10}}
	}},
	{"tundra",
	{
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{"snow", 77, white, PASSABLE, {0, 92, 5, 1, 2, 0, 0, 0, 0}},
		{"mountain", 13, grey, IMPASSABLE, {0, 25, 75, 0, 0, 0, 0, 0, 0}},
		{"water", 2, blue, IMPASSABLE, {0, 10, 0, 25, 65, 0, 0, 0, 0}},
		{"ice", 3, cyan, PASSABLE, {0, 10, 0, 25, 65, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}}
	}},
	{"desert",
	{
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{"sand 1", 30, yellow, PASSABLE, {0, 160, 30, 8, 1, 0, 0, 0, 0}},
		{"sand 2", 31, khaki, PASSABLE, {0, 30, 160, 8, 1, 0, 0, 0, 0}},
		{"cactus", 47, green, IMPASSABLE, {0, 47, 47, 3, 1, 0, 0, 0, 0}},
		{"water", 2, blue, IMPASSABLE, {0, 5, 5, 0, 70, 20, 0, 0, 0}},
		{"tree", 46, darkgreen, IMPASSABLE, {0, 30, 30, 30, 5, 5, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}}
	}},
	{"forest",
	{
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{"grass", 6, lightgreen, PASSABLE, {0, 20, 50, 15, 0, 0, 15, 0, 0}},
		{"underbrush", 8, green, PASSABLE, {0, 10, 40, 20, 20, 0, 10, 0, 0}},
		{"light trees", 9, medgreen, PASSABLE, {0, 10, 30, 30, 30, 0, 0, 0, 0}},
		{"heavy trees", 10, darkgreen, IMPASSABLE, {0, 0, 10, 40, 50, 0, 0, 0, 0}},
		{"shallow water", 4, blue, PASSABLE, {0, 50, 20, 0, 0, 20, 10, 0, 0}},
		{"deep water", 2, darkblue, IMPASSABLE, {0, 20, 0, 0, 0, 70, 10, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}}
	}},
	{"ocean",
	{
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}}
	}},
	{"map_generator",
	{
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{"grassland", 0, black, IMPASSABLE, {0, 100, 0, 0, 0, 0, 0, 0, 0}},
		{"tundra", 0, black, IMPASSABLE, {0, 5, 90, 0, 5, 0, 0, 0, 0}},
		{"desert", 0, black, IMPASSABLE, {0, 10, 0, 90, 0, 0, 0, 0, 0}},
		{"forest", 0, black, IMPASSABLE, {0, 5, 5, 0, 90, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{NULL, 0, black, IMPASSABLE, {0, 0, 0, 0, 0, 0, 0, 0, 0}}
	}}
};

struct tile_prob {
	short int prob[9];
};

/* Function prototypes */
static void	calc_probs(struct worldmap *map, struct tile_prob ***probs, short int biome);
static int	calc_tiles(struct worldmap *map, struct tile_prob **probs);

void
create_map(struct worldmap *map, int row_size, int col_size)
{
	int rows, cols;
	
	/* Save dimensions to structure */
	map->col_size = col_size;
	map->row_size = row_size;
	map->frame_speed = FRAME_SPEED;

	/* Allocate space for map and zero out */
	map->tile = malloc(sizeof(*(map->tile))*row_size);
	map->biome = malloc(sizeof(*(map->biome))*row_size);
	map->loot = malloc(sizeof(*(map->loot))*row_size);
	map->quantity = malloc(sizeof(*(map->quantity))*row_size);
	map->harvestable = malloc(sizeof(*(map->harvestable))*row_size);
	map->ground = malloc(sizeof(*(map->ground))*row_size);
	map->roof = malloc(sizeof(*(map->roof))*row_size);
	map->frame = malloc(sizeof(*(map->frame))*row_size);
	map->frame_count = malloc(sizeof(*(map->frame_count))*row_size);
	for (rows = 0; rows < row_size; rows++) {
		*(map->tile+rows) = malloc(sizeof(**map->tile)*col_size);
		*(map->biome+rows) = malloc(sizeof(**map->biome)*col_size);
		*(map->loot+rows) = malloc(sizeof(**map->loot)*col_size);
		*(map->quantity+rows) = malloc(sizeof(**map->quantity)*col_size);
		*(map->ground+rows) = malloc(sizeof(**map->ground)*col_size);
		*(map->roof+rows) = malloc(sizeof(**map->roof)*col_size);
		*(map->harvestable+rows) = malloc(sizeof(**map->harvestable)*col_size);
		*(map->frame+rows) = malloc(sizeof(**map->frame)*col_size);
		*(map->frame_count+rows) = malloc(sizeof(**map->frame_count)*col_size);
		for (cols = 0; cols < col_size; cols++) {
			*(*(map->tile+rows)+cols) = 0;
			*(*(map->biome+rows)+cols) = 0;
			*(*(map->loot+rows)+cols) = 0;
			*(*(map->quantity+rows)+cols) = 0;
			*(*(map->ground+rows)+cols) = 0;
			*(*(map->roof+rows)+cols) = 0;
			*(*(map->harvestable+rows)+cols) = SDL_TRUE;
			*(*(map->frame+rows)+cols) = rand_num(0, 1);
			if (*(*(map->frame+rows)+cols) == 0) {
				*(*(map->frame_count+rows)+cols) = rand_num(map->frame_speed, map->frame_speed*2);
			} else {
				*(*(map->frame_count+rows)+cols) = rand_num(map->frame_speed/2, map->frame_speed);
			}
		}
	}
}

void
free_map(struct worldmap *map)
{
	int rows;
	
	/* Free memory allocated for the map */
	for (rows = 0; rows < map->row_size; rows++) {
		free(*(map->tile+rows));
		free(*(map->biome+rows));
		free(*(map->loot+rows));
		free(*(map->quantity+rows));
		free(*(map->ground+rows));
		free(*(map->roof+rows));
		free(*(map->harvestable+rows));
		free(*(map->frame+rows));
		free(*(map->frame_count+rows));
	}
	free(map->tile);
	free(map->biome);
	free(map->loot);
	free(map->quantity);
	free(map->ground);
	free(map->roof);
	free(map->harvestable);
	free(map->frame);
	free(map->frame_count);
}

void
populate_map(struct worldmap *map, int start_tile, short int biome)
{
	int rows, cols, z;
	int zero_count;
	struct tile_prob **probs;
	
	/* Set biome equal to given value */
	for (rows = 0; rows < map->row_size; rows++) {
		for (cols = 0; cols < map->col_size; cols++) {
			*(*(map->biome+rows)+cols) = biome;
		}
	}
	
	/* Set up nine initial tiles, at 1/4, 2/4, and 3/4 vertical and horizontal */
	for (z = 0; z < 3; z++) {
		*(*(map->tile+(z+1)*(map->row_size/4))+1*(map->col_size/4)) = start_tile;
		*(*(map->tile+(z+1)*(map->row_size/4))+2*(map->col_size/4)) = start_tile;
		*(*(map->tile+(z+1)*(map->row_size/4))+3*(map->col_size/4)) = start_tile;
	}
	/* Allocate memory for probabilities struct and zero out */
	probs = malloc(sizeof(*probs) * map->row_size);
	for (rows = 0; rows < map->row_size; rows++) {
		*(probs+rows) = malloc(sizeof(**probs) * map->col_size);
		for (cols = 0; cols < map->col_size; cols++) {
			for (z = 0; z < 9; z++) {
				(*(*(probs+rows)+cols)).prob[z] = 0;
			}
		}
	}

	while (1) {
		/* Calculate all probabilities */
		calc_probs(map, &probs, biome);
		/* Apply all probabilities */
		zero_count = calc_tiles(map, probs);
		if (zero_count == 0) {
			break;
		}
	}
	/* Free probabilties */
	for (rows = 0; rows < map->row_size; rows++) {
		free(*(probs+rows));
	}
	free(probs);
}

static void
calc_probs(struct worldmap *map, struct tile_prob ***probs, short int biome)
{
	int rows, cols, z;
	struct tile_prob **prob;
	
	/* Dereferencing this to make it easier to work with */
	prob = *probs;
	
	/* Iterate through each tile of map and add probabilities */
	for (rows = 0; rows < map->row_size; rows++) {
		for (cols = 0; cols < map->col_size; cols++) {
			if (*(*(map->tile+rows)+cols) != 0) {
				for (z = 0; z < 9; z++) {
					if (cols > 0) {
						/* tile to left */
						(*(*(prob+rows)+cols-1)).prob[z] += BIOMES[biome].tiles[*(*(map->tile+rows)+cols)].prob[z];
					}
					if (cols < map->col_size - 1) {
						/* tile to right */
						(*(*(prob+rows)+cols+1)).prob[z] += BIOMES[biome].tiles[*(*(map->tile+rows)+cols)].prob[z];
					}
					if (rows > 0) {
						/* tile to up */
						(*(*(prob+rows-1)+cols)).prob[z] += BIOMES[biome].tiles[*(*(map->tile+rows)+cols)].prob[z];
					}
					if (rows < map->row_size - 1) {
						/* tile to down */
						(*(*(prob+rows+1)+cols)).prob[z] += BIOMES[biome].tiles[*(*(map->tile+rows)+cols)].prob[z];
					}
				}
			}
		}
	}
}

static int
calc_tiles(struct worldmap *map, struct tile_prob **probs)
{
	int rows, cols, z;
	int zero_count;
	int prob_max;
	int prob_roll;
	
	/* Assume there are zero "zeroes" */
	zero_count = 0;
	/* Iterate through every tile of map, calculate new tile based on probability */
	for (rows = 0; rows < map->row_size; rows++) {
		for (cols = 0; cols < map->col_size; cols++) {
			if (*(*(map->tile+rows)+cols) == 0) {
				/* calculate max probability */
				for (z = 1, prob_max = 0; z < 9; z++) {
					prob_max += (*(*(probs+rows)+cols)).prob[z];
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
						prob_max += (*(*(probs+rows)+cols)).prob[z];
						if (prob_roll < prob_max) {
							break;
						}
					}
					*(*(map->tile+rows)+cols) = z;
				}	
			}
		}
	}
	return zero_count;
}

short int
get_sprite(int tile, short int biome)
{
	return BIOMES[biome].tiles[tile].sprite;
}

char *
get_color(int tile, short int biome)
{
	return BIOMES[biome].tiles[tile].col;
}

char
is_passable(int tile, short int biome)
{
	return BIOMES[biome].tiles[tile].passable;
}

char *
get_tile_name(int tile, short int biome)
{
	return BIOMES[biome].tiles[tile].name;
}

void
process_frames(struct worldmap *map, int row, int col)
{
	int rows, cols;
	for (rows = row; rows < row + WIN_ROWS; rows++) {
		for (cols = col; cols < col + WIN_COLS; cols++) {
			/* count down the frame count */
			if (is_harvestable(map, cols, rows) == SDL_FALSE) continue;
			*(*(map->frame_count+rows)+cols) -= 1;
			/* if frame count is 0, change the frame */
			if (*(*(map->frame_count+rows)+cols) <= 0) {
				switch(*(*(map->frame+rows)+cols)) {
					case 0:
						*(*(map->frame+rows)+cols) = 1;
						*(*(map->frame_count+rows)+cols) = rand_num(map->frame_speed/2, map->frame_speed);
						break;
					case 1:
						*(*(map->frame+rows)+cols) = 0;
						*(*(map->frame_count+rows)+cols) = rand_num(map->frame_speed, map->frame_speed*2);
						break;						
					default:
						break;
				}
			}
		}
	}
}
