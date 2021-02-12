#include <SDL2/SDL.h>
#include "disp.h"
#include "harv.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"
#include "rand.h"

struct h_data {
	short int biome;
	short int tile;
	short int n_items;
	short int item_id[9];
	short int item_prob[9];
} H_TABLE[] = {
	{ 0, 1, 4, { 1, 2, 3, 4, 0, 0, 0, 0, 0 }, { 25, 25, 25, 25, 0, 0, 0, 0, 0 } },
	{ 0, 2, 4, { 5, 6, 7, 8, 0, 0, 0, 0, 0 }, { 11, 11, 11, 67, 0, 0, 0, 0, 0 } },
	{ -1, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

/* Function prototypes */
static SDL_bool		harvest_item(struct worldmap *map, struct player *cur_player, int x, int y);
static short int	get_item(short int biome, short int tile);
static SDL_bool		stash_item(struct worldmap *map, struct player *cur_player, int x, int y, short int item_id, unsigned char quantity);
static void		drop_item(struct worldmap *map, int x, int y, short int item_id, unsigned char quantity);

void
get_harvest_input(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };

	/* Prompt for direction and render */
	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*16 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
	draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Which direction?");
	render_present(cur_game);
	
	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while (finished == SDL_FALSE && SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_UP: /* pickup up */
				case SDLK_w:
					finished = harvest_item(map, cur_player, cur_player->x, cur_player->y-1);
					break;
				case SDLK_RIGHT: /* pickup right */
				case SDLK_d:
					finished = harvest_item(map, cur_player, cur_player->x+1, cur_player->y);
					break;
				case SDLK_DOWN: /* pickup down */
				case SDLK_s:
					finished = harvest_item(map, cur_player, cur_player->x, cur_player->y+1);
					break;
				case SDLK_LEFT: /* pickup left */
				case SDLK_a:
					finished = harvest_item(map, cur_player, cur_player->x-1, cur_player->y);
					break;
			default:
					finished = SDL_TRUE;
					return;
					break;
			}
		}
	}
}

static SDL_bool
harvest_item(struct worldmap *map, struct player *cur_player, int x, int y)
{
	short int biome;
	short int item;
	unsigned char quantity;
	short int tile;
	
	if (x < 0 || y < 0 || x > map->col_size - 1 || y > map->row_size - 1) return SDL_TRUE;
	
	/* Is there an item on the map? */
	if (*(*(map->loot+y)+x) != 0) {
		item = *(*(map->loot+y)+x);
		quantity = *(*(map->quantity+y)+x);
		printf("Harvested: %s\n", get_loot_name(item));
		/* Take item off map */
		*(*(map->quantity+y)+x) = 0;
		*(*(map->loot+y)+x) = 0;
		/* Try to stash the item and drop the remainder */
		if (stash_item(map, cur_player, x, y, item, quantity) == SDL_FALSE) {
			drop_item(map, x, y, item, quantity);
		}
		return SDL_TRUE;
	}
	
	/* No item on map, try to harvest */
	biome = *(*(map->biome+y)+x);
	tile = *(*(map->tile+y)+x);
	item = get_item(biome, tile);
	if (item == -1) {
		printf("Harvested nothing!\n");
	} else {
		printf("Harvested: %s\n", get_loot_name(item));
		if (stash_item(map, cur_player, x, y, item, 1) == SDL_FALSE) {
			drop_item(map, x, y, item, 1);
		}
	}
	return SDL_TRUE;
}

static short int
get_item(short int biome, short int tile)
{
	int prob;
	int prob_sum;
	short int x, y;
	
	/* Check that there is an item in that tile */
	for (x = 0; H_TABLE[x].biome != -1; x++) {
		if (biome == H_TABLE[x].biome && tile == H_TABLE[x].tile) break;
	}
	if (H_TABLE[x].biome == -1) return -1;
	
	/* Calculate maximum probability */
	for (y = 0, prob_sum = 0; y < H_TABLE[x].n_items; y++) {
		prob_sum += H_TABLE[x].item_prob[y];
	}
	prob = rand_num(0, prob_sum - 1);
	for (y = 0, prob_sum = 0; y < H_TABLE[x].n_items; y++) {
		prob_sum += H_TABLE[x].item_prob[y];
		if (prob < prob_sum) {
			return(H_TABLE[x].item_id[y]);
		}
	}
	printf("Uh oh\n");
	return -1;
}

static SDL_bool
stash_item(struct worldmap *map, struct player *cur_player, int x, int y, short int item_id, unsigned char quantity)
{
	int i;
	
	/* If it's stackable, check for a non-maxed stack in the inventory */
	if (is_loot_stackable(item_id) == STACKABLE) {
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == item_id && cur_player->quantity[i] < 255) {
				if ((int) quantity + (int) cur_player->quantity[i] > 255) {
					quantity -= 255 - cur_player->quantity[i];
					cur_player->quantity[i] = 255;
				} else {
					cur_player->quantity[i] += quantity;
					return SDL_TRUE;
				}		
			}
		}
	}
	/* There's still item left (or the item was unstackable) */
	if (quantity > 0) {
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == 0) {
				cur_player->loot[i] = item_id;
				cur_player->quantity[i] = quantity;
				return SDL_TRUE;
			}
		}
	}
	/* Could not put item in inventory */
	drop_item(map, x, y, item_id, quantity);
	return SDL_TRUE;
}

static void
drop_item(struct worldmap *map, int x, int y, short int item_id, unsigned char quantity)
{
	*(*(map->loot+y)+x) = item_id;
	*(*(map->quantity+y)+x) = quantity;
	printf("Dropping %d item(s) %s\n", quantity, get_loot_name(item_id));
}
