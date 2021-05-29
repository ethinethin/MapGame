#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "harv.h"
#include "hold.h"
#include "home.h"
#include "main.h"
#include "maps.h"
#include "npcs.h"
#include "play.h"

/* Function prototypes */
static void	save_map(struct worldmap *map, int save);
static void	save_player(struct worldmap *map, struct player *cur_player, int save);
static void	save_harv(int save);
static void	save_holders(struct worldmap *map, int save);
static void	save_npcs(int save);
static void	load_map(struct worldmap *map, int save);
static void	load_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save);
static void	load_harv(struct worldmap *map, int save);
static void	load_holders(struct worldmap *map, int save);
static void	load_npcs(int save);

/* Save file existence */
SDL_bool savefiles[4] = { SDL_FALSE, SDL_FALSE, SDL_FALSE, SDL_FALSE };

SDL_bool
*check_savefiles(void)
{
	char durr_name[12];
	char file_name[32];
	char *files[] = {"harv.mg", "hold.mg", "map.mg", "player.mg", "npcs.mg"};

	int i;
	int j;
	DIR *durr;
	
	/* Check that the save directory exists. If it doesn't, create it */
	durr = opendir("save");
	if (durr) {
		closedir(durr);
	} else {
		mkdir("save", 0744);
		mkdir("save/save00", 0744);
		mkdir("save/save01", 0744);
		mkdir("save/save02", 0744);
		mkdir("save/save03", 0744);
	}
		
	/* Check each folder for files */
	for (i = 0; i < 4; i++) {
		sprintf(durr_name, "save/save0%d", i);
		durr = opendir(durr_name);
		if (durr) {
			closedir(durr);
			/* Check each of the four files */
			savefiles[i] = SDL_TRUE;
			for (j = 0; j < 5; j++) {
				sprintf(file_name, "%s/%s", durr_name, files[j]);
				if (access(file_name, F_OK) != 0) {
					savefiles[i] = SDL_FALSE;
					break;
				}
			}
		} else {
			mkdir(durr_name, 0744);
			savefiles[i] = SDL_FALSE;
		}
	}
	return savefiles;
}

SDL_bool
save_all(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save)
{
	SDL_bool ok_to_save;
	char message[64];

	/* If there's a save in that file, make sure it's okay to overwrite */
	if (savefiles[save] == SDL_TRUE) {
		sprintf(message, "SAVE %d EXITS. IS IT OKAY TO OVERWRITE?", save + 1);
		ok_to_save = yesno_screen(cur_game, message);
	} else {
		ok_to_save = SDL_TRUE;
	}
	
	/* If it's okay to save, save, otherwise return */
	if (ok_to_save == SDL_TRUE) {
		save_map(map, save);
		save_harv(save);
		save_holders(map, save);
		save_player(map, cur_player, save);
		save_npcs(save);
		return SDL_TRUE;
	} else {
		return SDL_FALSE;
	}
}

static void
save_map(struct worldmap *map, int save)
{
	int i, j;
	FILE *fp = NULL;
	char filename[32];
	
	sprintf(filename, "save/save0%d/map.mg", save);
	
	/* Try to open file */
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Can't open %s\n", filename);
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

static void
save_player(struct worldmap *map, struct player *cur_player, int save)
{
	int i, j;
	FILE *fp = NULL;
	char filename[32];
	
	sprintf(filename, "save/save0%d/player.mg", save);
	
	/* Open file */
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Problem opening %s\n", filename);
		exit(1);
	}
	/* Save x and y values */
	fprintf(fp, "%d %d\n", cur_player->x, cur_player->y);
	/* Save loot and quantity */
	for (i = 0; i < MAX_INV; i++) {
		fprintf(fp, "%d %d\n", cur_player->loot[i], cur_player->quantity[i]);
	}
	/* Save player recipe knowledge */
	for (i = 0; i < MAX_RECIPE; i++) {
		fprintf(fp, "%d ", cur_player->recipe[i]);
	}
	/* Save map dimensions */
	fprintf(fp, "\n%d %d\n", map->row_size, map->col_size);
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

static void
save_harv(int save)
{
	FILE *fp;
	char filename[32];
	
	sprintf(filename, "save/save0%d/harv.mg", save);
	
	/* Try to open the file */
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Could not open %s\n", filename);
		exit(1);
	}
	/* Dump the harvest table to a file */
	dump_dtable(fp);
	/* Close the file */
	fclose(fp);
}

static void
save_holders(struct worldmap *map, int save)
{
	FILE *fp;
	char filename[32];
	
	sprintf(filename, "save/save0%d/hold.mg", save);
	
	/* Try to open the file */
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Could not open %s\n", filename);
		exit(1);
	}
	/* Dump the holder table to a file */
	dump_holders(map, fp);
	/* Close the file */
	fclose(fp);
}

static void
save_npcs(int save)
{
	FILE *fp;
	char filename[32];
	
	sprintf(filename, "save/save0%d/npcs.mg", save);
	
	/* Try to open the file */
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Could not open %s\n", filename);
		exit(1);
	}
	/* Dump the npc table to a file */
	dump_npcs(fp);
	/* Close the file */
	fclose(fp);
}

void
save_opts(struct game *cur_game)
{
	FILE *fp;
	
	fp = fopen("save/opts.mg", "w");
	if (fp == NULL) {
		printf("Could not open save/opts.mg\n");
		exit(1);
	}
	
	/* Save the settings from the options screen */
	fprintf(fp, "res=%dx%d\n", cur_game->screen.w, cur_game->screen.h);
	fprintf(fp, "vsync=%d\n", cur_game->screen.vsync);
	fprintf(fp, "displaymode=%d\n", cur_game->screen.displaymode);
	fprintf(fp, "scanlines=%d\n", cur_game->screen.scanlines_on);
	fclose(fp);
}

void
load_all(struct game *cur_game, struct worldmap *map, struct player *cur_player, int save)
{
	load_map(map, save);
	load_harv(map, save);
	load_holders(map, save);
	load_player(cur_game, map, cur_player, save);
	load_npcs(save);
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
	int recipe;
	int i, j;
	int row_size, col_size;
	#ifdef LOADBAR
	int perc;
	#endif
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
	/* Load recipe knowledge */
	for (i = 0; i < MAX_RECIPE; i++) {
		fscanf(fp, "%d ", &recipe);
		cur_player->recipe[i] = (char) recipe;
	}
	/* Load map dimensions */
	fscanf(fp, "\n%d %d\n", &row_size, &col_size);
	/* Allocate memory and load seen, then update map texture as needed */
	cur_player->seen = malloc(sizeof(*cur_player->seen)*row_size);
	for (i = 0, j = 0; i < row_size; i++) {
		#ifdef LOADBAR
		perc = 100 * ((i*map->col_size)+j) / (map->col_size * map->row_size);
		loading_bar(cur_game, "Loading map texture", perc);
		#endif
		*(cur_player->seen+i) = malloc(sizeof(**cur_player->seen)*col_size);
		for (j = 0; j < col_size; j++) {
			*(*(cur_player->seen+i)+j) = fgetc(fp) - '0';
			if (*(*(cur_player->seen+i)+j) == 1) {
				SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->map_texture);
				tile_col = get_color(*(*(map->tile+i)+j), *(*(map->biome+i)+j));
				draw_point(cur_game, j, i, tile_col);
			}
		}
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->screen.output);
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
	/* In case display settings have changed */
	cur_game->screen.display_reloaded = SDL_FALSE;
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

static void
load_npcs(int save)
{
	FILE *fp;
	char filename[32];
	
	sprintf(filename, "save/save0%d/npcs.mg", save);
	
	/* Try to open the file */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not open %s\n", filename);
		exit(1);
	}
	/* Load the npc table from the file */
	undump_npcs(fp);
	/* Close the file */
	fclose(fp);

}

void
load_opts(struct game *cur_game)
{
	FILE *fp;
	
	fp = fopen("save/opts.mg", "r");
	if (fp == NULL) {
		/* File does not exist, keep the default values */
		return;
	}
	
	/* Load the settings */
	fscanf(fp, "res=%dx%d\n", &cur_game->screen.w, &cur_game->screen.h);
	cur_game->screen.scale_x = cur_game->screen.w/1280.0;
	cur_game->screen.scale_y = cur_game->screen.h/720.0;
	fscanf(fp, "vsync=%d\n", &cur_game->screen.vsync);
	fscanf(fp, "displaymode=%d\n", &cur_game->screen.displaymode);
	fscanf(fp, "scanlines=%d\n", &cur_game->screen.scanlines_on);
	fclose(fp);
}
