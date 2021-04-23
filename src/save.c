#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "harv.h"
#include "hold.h"
#include "main.h"
#include "maps.h"
#include "play.h"

/* Function prototypes */
static void	save_map(struct worldmap *map);
static void	save_player(struct worldmap *map, struct player *cur_player);
static void	save_harv(void);
static void	save_holders(struct worldmap *map);
static void	load_map(struct worldmap *map, int save);
static void	load_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save);
static void	load_harv(struct worldmap *map, int save);
static void	load_holders(struct worldmap *map, int save);

/* Save file existence */
SDL_bool savefiles[4] = { SDL_FALSE, SDL_FALSE, SDL_FALSE, SDL_FALSE };

SDL_bool
*check_savefiles(void)
{
	char durr_name[12];
	char file_name[32];
	char *files[] = {"harv.mg", "hold.mg", "map.mg", "player.mg"};

	int i;
	int j;
	DIR *durr;
	
	for (i = 0; i < 4; i++) {
		sprintf(durr_name, "save/save0%d", i);
		durr = opendir(durr_name);
		if (durr) {
			closedir(durr);
			/* Check each of the four files */
			savefiles[i] = SDL_TRUE;
			for (j = 0; j < 4; j++) {
				sprintf(file_name, "%s/%s", durr_name, files[j]);
				if (access(file_name, F_OK) != 0) {
					savefiles[i] = SDL_FALSE;
					break;
				}
			}
		} else {
			savefiles[i] = SDL_FALSE;
		}
	}
	return savefiles;
}

void
save_all(struct worldmap *map, struct player *cur_player)
{
	save_map(map);
	save_harv();
	save_holders(map);
	save_player(map, cur_player);
}

#define MAP_FILE "save/save00/map.mg"
static void
save_map(struct worldmap *map)
{
	int i, j;
	FILE *fp = NULL;
	
	/* Try to open file */
	fp = fopen(MAP_FILE, "w");
	if (fp == NULL) {
		printf("Can't open %s\n", MAP_FILE);
		exit(1);
	}
	/* Save map dimensions */
	fprintf(fp, "%d %d\n", map->row_size, map->col_size);
	/* Save all map data */
	for (i = 0; i < map->row_size; i++) {
		for (j = 0; j < map->col_size; j++) {
			fprintf(fp, "%d %hd %hu %hu %hu %hu\n",
			        *(*(map->tile+i)+j),
			        *(*(map->biome+i)+j),
			        *(*(map->loot+i)+j),
			        *(*(map->quantity+i)+j),
			        *(*(map->ground+i)+j),
			        *(*(map->roof+i)+j));
		}
	}
	/* Close file */
	fclose(fp);
}

#define PLAYER_FILE "save/save00/player.mg"
static void
save_player(struct worldmap *map, struct player *cur_player)
{
	int i, j;
	FILE *fp = NULL;
	
	/* Open file */
	fp = fopen(PLAYER_FILE, "w");
	if (fp == NULL) {
		printf("Problem opening %s\n", PLAYER_FILE);
		exit(1);
	}
	/* Save x and y values */
	fprintf(fp, "%d %d\n", cur_player->x, cur_player->y);
	/* Save loot and quantity */
	for (i = 0; i < MAX_INV; i++) {
		fprintf(fp, "%d %d\n", cur_player->loot[i], cur_player->quantity[i]);
	}
	/* Save map dimensions */
	fprintf(fp, "%d %d\n", map->row_size, map->col_size);
	/* Save seen */
	for (i = 0; i < map->row_size; i++) {
		for (j = 0; j < map->col_size; j++) {
			fprintf(fp, "%d", *(*(cur_player->seen+i)+j));
		}
	}
	fputc('\n', fp);
	/* Close file */
	fclose(fp);
}

#define HARV_FILE "save/save00/harv.mg"
static void
save_harv(void)
{
	FILE *fp;
	
	/* Try to open the file */
	fp = fopen(HARV_FILE, "w");
	if (fp == NULL) {
		printf("Could not open %s\n", HARV_FILE);
		exit(1);
	}
	/* Dump the harvest table to a file */
	dump_dtable(fp);
	/* Close the file */
	fclose(fp);
}

#define HOLD_FILE "save/save00/hold.mg"
static void
save_holders(struct worldmap *map)
{
	FILE *fp;
	
	/* Try to open the file */
	fp = fopen(HOLD_FILE, "w");
	if (fp == NULL) {
		printf("Could not open %s\n", HOLD_FILE);
		exit(1);
	}
	/* Dump the holder table to a file */
	dump_holders(map, fp);
	/* Close the file */
	fclose(fp);
}

void
load_all(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save)
{
	load_map(map, save);
	load_harv(map, save);
	load_holders(map, save);
	load_player(cur_game, map, cur_player, save);
}

static void
load_map(struct worldmap *map, int save)
{
	int i, j;
	int row_size, col_size;
	FILE *fp = NULL;
	char filename[32];
	
	sprintf(filename, "save/save0%d/map.mg", save);
	
	/* Try to open file */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Can't open %s\n", filename);
		exit(1);
	}
	/* Load map dimensions and create an empty map */
	fscanf(fp, "%d %d\n", &row_size, &col_size);
	create_map(map, row_size, col_size);
	
	/* Load all map data */
	for (i = 0; i < row_size; i++) {
		for (j = 0; j < col_size; j++) {
			fscanf(fp, "%d %hd %hu %hu %hu %hu\n",
			        (*(map->tile+i)+j),
			        (*(map->biome+i)+j),
			        (*(map->loot+i)+j),
			        (*(map->quantity+i)+j),
			        (*(map->ground+i)+j),
			        (*(map->roof+i)+j));
		}
	}
	/* Close file */
	fclose(fp);
}

static void
load_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save)
{
	char *tile_col;
	int i, j;
	int row_size, col_size;
	FILE *fp = NULL;
	char filename[32];
	
	sprintf(filename, "save/save0%d/player.mg", save);

	/* Open file */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Problem opening %s\n", filename);
		exit(1);
	}
	/* Load x and y values */
	fscanf(fp, "%d %d\n", &cur_player->x, &cur_player->y);
	/* Load loot and quantity */
	for (i = 0; i < MAX_INV; i++) {
		fscanf(fp, "%hu %hu\n", &cur_player->loot[i], &cur_player->quantity[i]);
	}
	/* Load map dimensions */
	fscanf(fp, "%d %d\n", &row_size, &col_size);
	/* Allocate memory and load seen, then update map texture as needed */
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->map_texture);
	cur_player->seen = malloc(sizeof(*cur_player->seen)*row_size);
	for (i = 0; i < row_size; i++) {
		*(cur_player->seen+i) = malloc(sizeof(**cur_player->seen)*col_size);
		for (j = 0; j < col_size; j++) {
			*(*(cur_player->seen+i)+j) = fgetc(fp) - '0';
			if (*(*(cur_player->seen+i)+j) == 1) {
				tile_col = get_color(*(*(map->tile+i)+j), *(*(map->biome+i)+j));
				draw_point(cur_game, j, i, tile_col);
			}
		}
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
	/* Close file */
	fclose(fp);
	/* Allocate memory for screenview */
	cur_player->screen_view = malloc(sizeof(*cur_player->screen_view)*WIN_ROWS);
	for (i = 0; i < WIN_ROWS; i++) {
		*(cur_player->screen_view+i) = malloc(sizeof(**cur_player->screen_view)*WIN_COLS);
		for (j = 0; j < WIN_COLS; j++) {
			*(*(cur_player->screen_view+i)+j) = 0;
		}
	}
}

static void
load_harv(struct worldmap *map, int save)
{
	FILE *fp;
	char filename[32];
	
	sprintf(filename, "save/save0%d/harv.mg", save);

	/* Try to open the file */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not open %s\n", filename);
		exit(1);
	}
	/* Load the harvest table from a file */
	load_dtable(map, fp);
	/* Close the file */
	fclose(fp);
}

static void
load_holders(struct worldmap *map, int save)
{
	FILE *fp;
	char filename[32];
	
	sprintf(filename, "save/save0%d/hold.mg", save);

	/* Try to open the file */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not open %s\n", filename);
		exit(1);
	}
	/* Load the holders table from the file */
	undump_holders(map, fp);
	/* Close the file */
	fclose(fp);
}
