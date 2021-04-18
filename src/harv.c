#include <SDL2/SDL.h>
#include "disp.h"
#include "harv.h"
#include "hold.h"
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
	short int min[9];
	short int max[9];
	short int unharvest_prob;
	short int reharvest_turns;
	short int reharvest_prob;
} H_TABLE[] = {
	{ 0, 1, 5, { 14, 15, 16, 17, 18, 0, 0, 0, 0 }, { 70, 15, 4, 1, 10, 0, 0, 0, 0 },
		   { 1, 1, 1, 1, 1, 0, 0, 0, 0 }, { 5, 2, 1, 1, 2, 0, 0, 0, 0 }, 40, 50, 5 },
	{ 0, 2, 5, { 14, 15, 16, 17, 18, 0, 0, 0, 0 }, { 75, 10, 4, 1, 10, 0, 0, 0, 0 },
		   { 2, 1, 1, 1, 1, 0, 0, 0, 0 }, { 10, 3, 1, 1, 2, 0, 0, 0, 0 }, 30, 40, 5 },
	{ 0, 3, 5, { 14, 15, 19, 20, 18, 0, 0, 0, 0 }, { 70, 15, 4, 1, 10, 0, 0, 0, 0 },
		   { 2, 1, 1, 1, 1, 0, 0, 0, 0 }, { 10, 3, 5, 1, 2, 0, 0, 0, 0 }, 30, 30, 5 },
	{ 0, 4, 5, { 14, 15, 21, 19, 20, 0, 0, 0, 0 }, { 70, 15, 10, 4, 1, 0, 0, 0, 0 },
		   { 3, 1, 1, 1, 1, 0, 0, 0, 0 }, { 5, 2, 1, 5, 1, 0, 0, 0, 0 }, 20, 30, 5 },
	{ 0, 5, 5, { 14, 24, 18, 22, 23, 0, 0, 0, 0 }, { 30, 10, 54, 5, 1, 0, 0, 0, 0 },
		   { 1, 1, 2, 1, 1, 0, 0, 0, 0 }, { 5, 2, 10, 3, 1, 0, 0, 0, 0 }, 40, 50, 5 },
	{ 0, 6, 5, { 18, 24, 22, 25, 26, 0, 0, 0, 0 }, { 67, 20, 10, 2, 1, 0, 0, 0, 0 },
		   { 4, 1, 1, 1, 1, 0, 0, 0, 0 }, { 10, 2, 5, 1, 1, 0, 0, 0, 0 }, 30, 150, 1 },
	{ 0, 7, 5, { 14, 18, 23, 27, 28, 0, 0, 0, 0 }, { 30, 30, 15, 20, 5, 0, 0, 0, 0 },
		   { 1, 1, 1, 1, 1, 0, 0, 0, 0 }, { 5, 2, 1, 1, 1, 0, 0, 0, 0 }, 50, 50, 5 },
	{ 0, 8, 5, { 14, 18, 23, 27, 28, 0, 0, 0, 0 }, { 20, 30, 15, 30, 5, 0, 0, 0, 0 },
		   { 1, 1, 1, 1, 1, 0, 0, 0, 0 }, { 5, 2, 1, 2, 1, 0, 0, 0, 0 }, 30, 30, 5 },
	{ -1, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		   { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0, 0, 0 }
};

struct d_data {
	struct d_data *next;
	int x;
	int y;
	short int turn;
	short int prob;
} *D_TABLE;

/* Function prototypes */
static short int	get_item(short int biome, short int tile);
static unsigned char	get_quantity(short int biome, short int tile, short int item_id);
static void		make_unharvestable(struct worldmap *map, int x, int y, short int biome, short int tile);
static SDL_bool		stash_item(struct player *cur_player, short int item_id, unsigned char quantity);
static void		drop_item(struct worldmap *map, int x, int y, short int item_id, unsigned char quantity);
static SDL_bool		harvest_loop(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static void		add_dtable(struct worldmap *map, int x, int y, short int turn, short int prob);

void
get_harvest_input(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };

	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*16 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
		draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Which direction?");
		render_present(cur_game);
		SDL_Delay(10);
		/* Poll for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_UP: /* pickup up */
				case SDLK_w:
					finished = harvest_item(cur_game, map, cur_player, cur_player->x, cur_player->y-1);
					break;
				case SDLK_RIGHT: /* pickup right */
				case SDLK_d:
					finished = harvest_item(cur_game, map, cur_player, cur_player->x+1, cur_player->y);
					break;
				case SDLK_DOWN: /* pickup down */
				case SDLK_s:
					finished = harvest_item(cur_game, map, cur_player, cur_player->x, cur_player->y+1);
					break;
				case SDLK_LEFT: /* pickup left */
				case SDLK_a:
					finished = harvest_item(cur_game, map, cur_player, cur_player->x-1, cur_player->y);
					break;
				case SDLK_i:
					toggle_inv(cur_game);
					break;
			default:
					finished = SDL_TRUE;
					return;
					break;
			}
		}
	}
}

SDL_bool
harvest_item(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	short int biome;
	short int item;
	unsigned char quantity;
	short int tile;
	SDL_bool finished;
	
	if (x < 0 || y < 0 || x > map->col_size - 1 || y > map->row_size - 1) return SDL_TRUE;
	
	/* Is there a roof on the map and the player is outdoors? */
	if (*(*(map->roof+y)+x) != 0 && *(*(map->roof+cur_player->y)+cur_player->x) == 0) {
		item = *(*(map->roof+y)+x);
		/* Try to stash the roof and take off map if stash is successful */
		if (stash_item(cur_player, item, 1) == SDL_TRUE) {
			/* Take roof off map */
			*(*(map->roof+y)+x) = 0;
		}
		return SDL_TRUE;
	} else if (*(*(map->loot+y)+x) != 0) {
		item = *(*(map->loot+y)+x);
		quantity = *(*(map->quantity+y)+x);
		/* If it's a placed open door, change it to a closed door */
		if (quantity == 1 && get_loot_type(item) == O_DOOR) {
			item -= 1;
		/* If it's a container, either open or collect it */	
		} else if (get_loot_type(item) == HOLDER) {
			if (chest_empty(x, y) == SDL_FALSE) {
				open_chest(cur_game, map, cur_player, x, y);
				return SDL_TRUE;
			}
		}
		/* Take item off map */
		*(*(map->quantity+y)+x) = 0;
		*(*(map->loot+y)+x) = 0;
		/* Try to stash the item and drop the remainder */
		if (stash_item(cur_player, item, quantity) == SDL_FALSE) {
			drop_item(map, x, y, item, quantity);
		}
		return SDL_TRUE;
	} else if (*(*(map->ground+y)+x) != 0) {
		/* Is there a ground on the map? */
		item = *(*(map->ground+y)+x);
		/* Try to stash the ground and take off map if stash is successful */
		if (stash_item(cur_player, item, 1) == SDL_TRUE) {
			/* Take ground off map */
			*(*(map->ground+y)+x) = 0;
		}
		return SDL_TRUE;
	}	
	
	/* No item in roof, loot, or ground, so try to harvest */
	biome = *(*(map->biome+y)+x);
	tile = *(*(map->tile+y)+x);
	item = get_item(biome, tile);
	if (item != -1) {
		do {
			if (is_harvestable(map, x, y) == SDL_FALSE) return SDL_TRUE;
			finished = harvest_loop(cur_game, map, cur_player);
			if (finished == SDL_TRUE) {
				quantity = get_quantity(biome, tile, item);
				if (stash_item(cur_player, item, quantity) == SDL_FALSE) {
					drop_item(map, x, y, item, quantity);
					finished = SDL_FALSE;
				}
				make_unharvestable(map, x, y, biome, tile);
			}
			item = get_item(biome, tile);
		} while (finished == SDL_TRUE);
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
	return -1;
}

static unsigned char
get_quantity(short int biome, short int tile, short int item_id)
{
	int x, y;
	int rando;
	
	/* Find H_TABLE entry */
	for (x = 0; H_TABLE[x].biome != -1; x++) {
		if (biome == H_TABLE[x].biome && tile == H_TABLE[x].tile) break;
	}
	if (H_TABLE[x].biome == -1) return 0;
	for (y = 0; y < 9; y++) {
		if (H_TABLE[x].item_id[y] == item_id) break;
	}
	
	/* Generate random number */
	rando = rand_num(H_TABLE[x].min[y], H_TABLE[x].max[y]);
	return rando;
}

SDL_bool
is_harvestable(struct worldmap *map, int x, int y)
{
	return *(*(map->harvestable+y)+x);
}

static void
make_unharvestable(struct worldmap *map, int x, int y, short int biome, short int tile)
{
	int i;
	int rando;
	
	/* Find H_TABLE entry */
	for (i = 0; H_TABLE[i].biome != -1; i++) {
		if (biome == H_TABLE[i].biome && tile == H_TABLE[i].tile) break;
	}
	if (H_TABLE[i].biome == -1) return;
	
	/* Generate a random number and make unharvestable? */
	rando = rand_num(0, 99);
	if (rando < H_TABLE[i].unharvest_prob) {
		*(*(map->harvestable+y)+x) = SDL_FALSE;
		add_dtable(map, x, y, -1, -1);
	}
}

static SDL_bool
stash_item(struct player *cur_player, short int item_id, unsigned char quantity)
{
	int i;
	
	/* If it's stackable, check for a non-maxed stack in the inventory */
	if (is_loot_stackable(item_id) == STACKABLE) {
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == item_id && cur_player->quantity[i] < MAX_STACK) {
				if ((int) quantity + (int) cur_player->quantity[i] > MAX_STACK) {
					quantity -= MAX_STACK - cur_player->quantity[i];
					cur_player->quantity[i] = MAX_STACK;
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
	return SDL_FALSE;
}

static void
drop_item(struct worldmap *map, int x, int y, short int item_id, unsigned char quantity)
{
	*(*(map->loot+y)+x) = item_id;
	*(*(map->quantity+y)+x) = quantity;
}

static SDL_bool
harvest_loop(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char black[3] = { 0, 0, 0 };
	char blue[3] = { 0, 0, 255 };
	char white[3] = { 255, 255, 255 };
	int counter;
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	
	/* Enter input loop */	
	counter = 0;
	while (finished == SDL_FALSE) {
		/* Redraw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		/* Draw bar above player */
		draw_rect(cur_game,
		          GAME_X + SPRITE_W * WIN_SCALE * cur_player->winpos_x - SPRITE_W * WIN_SCALE / 2,
		          GAME_Y + SPRITE_H * WIN_SCALE * cur_player->winpos_y - 40,
		          SPRITE_W * WIN_SCALE * 2, 20, SDL_TRUE, black, SDL_TRUE, white);
		draw_rect(cur_game,
		          GAME_X + SPRITE_W * WIN_SCALE * cur_player->winpos_x - SPRITE_W * WIN_SCALE / 2 + 1,
		          GAME_Y + SPRITE_H * WIN_SCALE * cur_player->winpos_y - 40 + 1,
		          (SPRITE_W * WIN_SCALE * 2 - 2) * counter / 100, 20 - 2, SDL_TRUE, blue, SDL_FALSE, NULL);
		render_present(cur_game);
		SDL_Delay(10);
		/* Increment bar */
		counter++;
		if (counter == 100) break;

		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_i) {
				/* toggle inventory and re-render screen */
				toggle_inv(cur_game);
			} else {
				return SDL_FALSE;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			return SDL_FALSE;
		}
	}
	return SDL_TRUE;
}

void
setup_dtable(void)
{
	struct d_data *first;
	
	/* Make first (null) entry in depleted table */
	first = malloc(sizeof(*first)*1);
	first->next = NULL;
	D_TABLE = first;
}

static void
add_dtable(struct worldmap *map, int x, int y, short int turn, short int prob)
{
	short int biome;
	short int tile;
	int i;
	struct d_data *current;
	struct d_data *tmp;

	/* Look up turn and prob */
	if (turn == -1) {
		biome = *(*(map->biome+y)+x);
		tile = *(*(map->tile+y)+x);
		for (i = 0; H_TABLE[i].biome != -1; i++) {
			if (H_TABLE[i].biome == biome && H_TABLE[i].tile == tile) {
				turn = H_TABLE[i].reharvest_turns;
				prob = H_TABLE[i].reharvest_prob;
				break;
			}
		}
		if (H_TABLE[i].biome == -1) return;
	}
					
	/* Set values in current d_data */
	current = malloc(sizeof(*current)*1);
	current->x = x;
	current->y = y;
	current->turn = turn;
	current->prob = prob;
	current->next = NULL;
	
	/* Add to linked list */
	tmp = D_TABLE;
	while (SDL_TRUE) {
		if (tmp->next == NULL) {
			tmp->next = current;
			return;
		} else {
			tmp = tmp->next;
		}
	}
}

void
check_depleted(struct worldmap *map)
{
	int rando;
	struct d_data *tmp;
	struct d_data *last;
	
	/* Run through the D_TABLE and check if tile needs to regenerate */
	tmp = D_TABLE;
	while (tmp->next != NULL) {
		last = tmp;
		tmp = tmp->next;
		if (tmp->turn > 0) {
			/* If tile is not ready to regenerate, adjust turns */
			tmp->turn--;
		} else {
			/* Tile is ready to regenerate, check probability */
			rando = rand_num(0, 99);
			if (rando < tmp->prob) {
				/* Regenerate the tile */
				*(*(map->harvestable+tmp->y)+tmp->x) = SDL_TRUE;
				/* Remove from linked list */
				last->next = tmp->next;
				free(tmp);
				tmp = last;
			}
		}
	}
}

void
kill_dtable(void)
{
	struct d_data *next;
	struct d_data *tmp;
	
	/* Delete all entries from the holder table */
	next = D_TABLE;
	while (SDL_TRUE) {
		if (next->next == NULL) {
			free(next);
			return;
		} else {
			tmp = next;
			next = next->next;
			free(tmp);
		}
	}	
}

void
dump_dtable(FILE *fp)
{
	struct d_data *dump;
	
	/* Go through every entry in the dump table and output values */
	dump = D_TABLE;
	while (SDL_TRUE) {
		if (dump->next == NULL) {
			return;
		} else {
			dump = dump->next;
			fprintf(fp, "%d %d %hd %hd\n", dump->x, dump->y, dump->turn, dump->prob);
		}
	}
}

void
load_dtable(struct worldmap *map, FILE *fp)
{
	int x, y;
	short int turn, prob;
	struct worldmap *not_used = NULL;

	/* Set up the dtable */
	setup_dtable();
	/* Read line from file */
	while (fscanf(fp, "%d %d %hd %hd\n", &x, &y, &turn, &prob) == 4) {
		add_dtable(not_used, x, y, turn, prob);
		*(*(map->harvestable+y)+x) = SDL_FALSE;
	}
}
