#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "main.h"
#include "make.h"
#include "maps.h"
#include "play.h"

/* Structure for crafting parameters */
struct CRAFT_PAR {
	unsigned short int current;
	unsigned short int quantity;
	SDL_bool enter_text;
	SDL_bool sufficient;
};

/* Function prototypes */
static void			draw_make(struct game *cur_game, struct player *cur_player, struct CRAFT_PAR *cur_craft);
static void			mouse_click_craft(int x, int y, struct player *cur_player, struct CRAFT_PAR *cur_craft);
static void	 		change_quantity(int number, struct CRAFT_PAR *cur_craft);
static unsigned short int	determine_max(struct player *cur_player, struct CRAFT_PAR *cur_craft);

void
make_stuff(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	struct CRAFT_PAR crafting = { 1, 1, SDL_FALSE, SDL_TRUE };
	SDL_bool finished = SDL_FALSE;
	SDL_Event event;
	
	while (finished == SDL_FALSE) {
		/* Redraw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_make(cur_game, cur_player, &crafting);
		render_present(cur_game);
		SDL_Delay(10);
		/* check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_i) {
				toggle_inv(cur_game);
			} else if (event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9' && crafting.enter_text == SDL_TRUE) {
				change_quantity(event.key.keysym.sym - 48, &crafting);
			} else if (event.key.keysym.sym == '\b' && crafting.enter_text == SDL_TRUE) {
				change_quantity(-1, &crafting);
			} else if (event.key.keysym.sym == '\r' && crafting.enter_text == SDL_TRUE) {
				crafting.enter_text = SDL_FALSE;
				if (crafting.quantity < 1) crafting.quantity = 1;
			} else {
				finished = SDL_TRUE;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			mouse_click_craft(event.motion.x, event.motion.y, cur_player, &crafting);
		}
	}
}

static void
draw_make(struct game *cur_game, struct player *cur_player, struct CRAFT_PAR *cur_craft)
{
	char black[3] = { 0, 0, 0};
	char white[3] = { 255, 255, 255 };
	char darkred[3] = { 128, 0, 0 };
	char amount[10];
	int i;
	
	/* Draw main crafting window */
	draw_rect(cur_game, MAKER_X, MAKER_Y, MAKER_W, MAKER_H, SDL_TRUE, black, SDL_TRUE, white);
	/* Draw header */
	draw_rect(cur_game, MAKER_X, MAKER_Y - 20, MAKER_W, 18 + 3, SDL_TRUE, black, SDL_TRUE, white);
	draw_small_sentence(cur_game, MAKER_X + 2, MAKER_Y - 17, "CRAFTING");
	draw_line(cur_game, MAKER_X + MAKER_W - 15, MAKER_Y - 15, MAKER_X + MAKER_W - 5, MAKER_Y - 5, white);
	draw_line(cur_game, MAKER_X + MAKER_W - 6, MAKER_Y - 15, MAKER_X + MAKER_W - 16, MAKER_Y - 5, white);
	/* Draw recipe book outline */
	draw_rect(cur_game, MAKER_X + 10, MAKER_Y + 20 + 10, SPRITE_W * WIN_SCALE * 4, MAKER_H - 20 - 10 - 10, SDL_TRUE, black, SDL_TRUE, white);
	draw_rect(cur_game, MAKER_X + 10, MAKER_Y + 10, SPRITE_W * WIN_SCALE * 4, 20, SDL_TRUE, black, SDL_TRUE, white);
	draw_small_sentence(cur_game, MAKER_X + 10 + 2, MAKER_Y + 10 + 2, "RECIPES");
	/* Draw current recipes */
	char *names[] = {"STONE WALL", "STONE FLOOR", "SLATE ROOF", "FISHING ROD"};
	char *recipes[] = {"1x stone", "1x stone", "1x slate", "1x branch\n1x spider silk"};
	unsigned short int sprites[] = { 576, 575, 579, 583 };
	for (i = 0; i < 4; i++) {
		draw_rect(cur_game, MAKER_X + 10 + 10, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE, SDL_TRUE, black, SDL_TRUE, white);
		draw_small_sentence(cur_game, MAKER_X + 10 + 10 + 80, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42, names[i]);
		draw_small_sentence(cur_game, MAKER_X + 10 + 10 + 80, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + 20, recipes[i]);
		draw_tile(cur_game, MAKER_X + 10 + 10 + 3, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + 3, SPRITE_W * WIN_SCALE * 0.9, SPRITE_H * WIN_SCALE * 0.9, sprites[i], 255);
		if (cur_craft->current - 1 == i) {
			draw_rect(cur_game, MAKER_X + 10 + 10 - 3, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 - 3, SPRITE_W * WIN_SCALE + 6, SPRITE_H * WIN_SCALE + 6, SDL_FALSE, darkred, SDL_FALSE, NULL);
			draw_rect(cur_game, MAKER_X + 10 + 10 - 2, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 - 2, SPRITE_W * WIN_SCALE + 4, SPRITE_H * WIN_SCALE + 4, SDL_FALSE, darkred, SDL_FALSE, NULL);
			draw_rect(cur_game, MAKER_X + 10 + 10 - 1, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 - 1, SPRITE_W * WIN_SCALE + 2, SPRITE_H * WIN_SCALE + 2, SDL_FALSE, darkred, SDL_FALSE, NULL);
		}
	}
	/* Draw crafting parameters */
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 2, "CURRENT:");
	draw_rect(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 15, MAKER_Y + 10 + 20, SPRITE_W * WIN_SCALE * 3 + 2, SPRITE_H * WIN_SCALE * 3 + 2, SDL_FALSE, white, SDL_FALSE, NULL);
	if (cur_craft->current != 0) {
		draw_tile(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 1 + 15, MAKER_Y + 10 + 20 + 1, SPRITE_W * WIN_SCALE * 3, SPRITE_H * WIN_SCALE * 3, sprites[cur_craft->current - 1], 255);
	}
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4, "QUANTITY:");
	draw_rect(cur_game,
		  MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100,
		  MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10,
		  120,
		  22,
		  SDL_FALSE, white, SDL_FALSE, NULL);
	if (cur_craft->enter_text == SDL_TRUE) {
		if (cur_craft->quantity > 0) {
			sprintf(amount, "%d|", cur_craft->quantity);
		} else {
			sprintf(amount, "|");
		}
	} else {
		sprintf(amount, "%d", cur_craft->quantity);
	}
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100 + 4, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4, amount);
	/* Draw the ol' buttonorooni */
	SDL_Rect rect = {MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 6 + 2, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4 + 20 + 10 + 50, 230, 70};
	/* If quantity is insufficient, say "INSUFFICIENT MATERIALS" */
	if (cur_craft->quantity > determine_max(cur_player, cur_craft)) {
		draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4 + 20, "INSUFFICIENT\n MATERIALS");
	}
	
	/* RENDER */
	SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft, NULL, &rect);
}

static void
mouse_click_craft(int x, int y, struct player *cur_player, struct CRAFT_PAR *cur_craft)
{
	unsigned short int max;
	int i;
	
	/* Exit enter text mode if necessary */
	cur_craft->enter_text = SDL_FALSE;
	
	/* Clicked on recipe */
	for (i = 0; i < 4; i++) {
		if (x >= MAKER_X + 10 + 10 &&
		    x <= MAKER_X + 10 + 10 + SPRITE_W * WIN_SCALE &&
		    y >= MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 &&
		    y <= MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + SPRITE_H * WIN_SCALE) {
		    	cur_craft->current = i + 1;
		    	return;
		}
	}
	/* Clicked on quantity box */
	if (x >= MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100 &&
	    x <= MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100 + 120 &&
	    y >= MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 &&
	    y <= MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 22) {
	    	cur_craft->enter_text = SDL_TRUE;
	}
	
	/* Clicked on craft buttons */
	max = determine_max(cur_player, cur_craft);
	if (x >= 660 && x <= 733 &&
	    y >= 375 && y <= 409) {
	    	cur_craft->quantity = 10;
	    	if (cur_craft->quantity > max) cur_craft->quantity = max;
	} else if (x >= 737 && x <= 809 &&
		   y >= 375 && y <= 409) {
	    	cur_craft->quantity = 100;
	    	if (cur_craft->quantity > max) cur_craft->quantity = max;
	} else if (x >= 813 && x <= 885 &&
		   y >= 375 && y <= 409) {
	    	cur_craft->quantity = 999;
	    	if (cur_craft->quantity > max) cur_craft->quantity = max;
	} else if (x >= 660 && x <= 885 &&
		   y >= 411 && y <= 445) {
		if (cur_craft->quantity <= max) {
		    	printf("Hey I'm craftin' here!\n");
		} else {
			printf("Hey I can't be craftin' these!\n");
		}
	}
}

static void
change_quantity(int number, struct CRAFT_PAR *cur_craft)
{
	int tmp_quantity;
		
	if (number >= 0) {
		/* Make sure new quantity is not over 999 */
		tmp_quantity = cur_craft->quantity * 10;
		if (tmp_quantity >= 999) {
			return;
		} else {
			cur_craft->quantity = tmp_quantity += number;
		}
	} else {
		cur_craft->quantity /= 10;
	}	
}

static unsigned short int
determine_max(struct player *cur_player, struct CRAFT_PAR *cur_craft)
{
	int i;
	unsigned short int item_id;
	unsigned short int mat1, mat2;
	unsigned short int need1, need2;
	int have1 = 0, have2 = 0;
	unsigned short int max_quantity;
	
	/* Which item? */
	switch(cur_craft->current) {
		case 1:
			item_id = 1;
			mat1 = 18;
			mat2 = 0;
			need1 = 1;
			need2 = 0;
			break;
		case 2:
			item_id = 2;
			mat1 = 18;
			mat2 = 0;
			need1 = 1;
			need2 = 0;
			break;
		case 3:
			item_id = 3;
			mat1 = 24;
			mat2 = 0;
			need1 = 1;
			need2 = 0;
			break;
		case 4:
			item_id = 9;
			mat1 = 19;
			mat2 = 21;
			need1 = 1;
			need2 = 1;
			break;
		default:
			return 0;
			break;
	}
	
	/* Check for first item */
	for (i = 0; i < MAX_INV; i++) {
		if (cur_player->loot[i] == mat1) {
			have1 += cur_player->quantity[i];
		} else if (cur_player->loot[i] == mat2) {
			have2 += cur_player->quantity[i];
		}
	}
	/* Determine how many you can make */
	have1 = have1/need1;
	if (need2 != 0) {
		have2 = have2/need2;
	} else {
		have2 = have1;
	}
	/* Return lowest value */
	if (have1 < have2) {
		max_quantity = have1;
	} else {
		max_quantity = have2;
	}
	/* Is it over 999? */
	if (max_quantity > 999) max_quantity = 999;
	return max_quantity;
}
