#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "loot.h"
#include "main.h"
#include "make.h"
#include "maps.h"
#include "play.h"

/* Structure for crafting parameters */
struct craft_par {
	short int recipe;
	unsigned short int current;
	unsigned short int quantity;
	SDL_bool enter_text;
	SDL_bool sufficient;
};

/* Structure for recipe table */
struct r_data {
	unsigned short int item_id;
	unsigned short int mat1;
	unsigned short int mat2;
	unsigned short int need1;
	unsigned short int need2;
} R_TABLE[] = {
	{1, 18, 0, 3, 0},
	{2, 18, 0, 3, 0},
	{3, 24, 0, 1, 0},
	{9, 19, 21, 1, 1},
	{6, 21, 18, 10, 5},
	{4, 21, 18, 5, 3},
	{7, 6, 16, 1, 1},
	{8, 6, 16, 1, 1},
	{-1, 0, 0, 0, 0}
};

/* Function prototypes */
static void			draw_make(struct game *cur_game, struct player *cur_player, struct craft_par *cur_craft);
static void			mouse_click_craft(int x, int y, struct player *cur_player, struct craft_par *cur_craft);
static void	 		change_quantity(int number, struct craft_par *cur_craft);
static unsigned short int	determine_max(struct player *cur_player, struct craft_par *cur_craft);
static void			craft_it(struct player *cur_player, struct craft_par *cur_craft);

void
make_stuff(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	struct craft_par crafting = { 0, 1, 1, SDL_FALSE, SDL_TRUE };
	SDL_bool finished = SDL_FALSE;
	SDL_Event event;
	
	/* Update recipe knowledge */
	check_recipes(cur_player);
	
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
			mouse_click_craft(event.motion.x / cur_game->screen.scale_x, event.motion.y / cur_game->screen.scale_y, cur_player, &crafting);
		}
	}
}

static void
draw_make(struct game *cur_game, struct player *cur_player, struct craft_par *cur_craft)
{
	char black[3] = { 0, 0, 0};
	char white[3] = { 255, 255, 255 };
	char darkred[3] = { 128, 0, 0 };
	char amount[10];
	char recipe[128];
	int i;
	SDL_Rect rect;
	
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
	/* Draw scrolling arrows */
	rect.x = MAKER_X + 10 + 10 + SPRITE_W * WIN_SCALE * 4 - 80; rect.y = MAKER_Y + 10 + 20 + 10; rect.w = 69; rect.h = 69;
	if (cur_craft->recipe == 0) {
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[4], NULL, &rect);
	} else {
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[2], NULL, &rect);
	}
	rect.y += 5 * SPRITE_H * WIN_SCALE + 40;
	if (cur_craft->recipe == 4) {	
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[5], NULL, &rect);
	} else {
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[3], NULL, &rect);
	}
	/* Draw current recipes */
	for (i = 0; i < 4; i++) {
		/* Draw white border around icon */
		draw_rect(cur_game, MAKER_X + 10 + 10, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE, SDL_TRUE, black, SDL_TRUE, white);
		if (cur_player->recipe[i + cur_craft->recipe] == 0) {
			/* Draw "UNKNOWN" */
			draw_small_sentence(cur_game, MAKER_X + 10 + 10 + 80, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42, "UNKNOWN RECIPE");
			/* Drawing materials */
			draw_small_sentence(cur_game, MAKER_X + 10 + 10 + 80, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + 20, "?x ??????\n?x ??????");
			/* Drawing item icon */
			rect.x = MAKER_X + 10 + 10 + 3; rect.y = MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + 3; rect.w = SPRITE_W * WIN_SCALE * 0.9; rect.h = SPRITE_H * WIN_SCALE * 0.9;
			SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[6], NULL, &rect);
		} else {
			/* Draw name of item */
			draw_small_sentence(cur_game, MAKER_X + 10 + 10 + 80, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42, get_loot_name(R_TABLE[i + cur_craft->recipe].item_id));
			/* Drawing materials */
			sprintf(recipe, "%dx %s", R_TABLE[i + cur_craft->recipe].need1, get_loot_name(R_TABLE[i + cur_craft->recipe].mat1));
			if (R_TABLE[i + cur_craft->recipe].mat2 != 0) sprintf(recipe, "%s\n%dx %s", recipe, R_TABLE[i + cur_craft->recipe].need2, get_loot_name(R_TABLE[i + cur_craft->recipe].mat2));	
			draw_small_sentence(cur_game, MAKER_X + 10 + 10 + 80, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + 20, recipe);
			/* Drawing item icon */
			draw_tile(cur_game, MAKER_X + 10 + 10 + 3, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 + 3, SPRITE_W * WIN_SCALE * 0.9, SPRITE_H * WIN_SCALE * 0.9, get_loot_sprite(R_TABLE[i + cur_craft->recipe].item_id), 255);
		}
		/* Drawing red outline */
		if (cur_craft->current - 1 - cur_craft->recipe == i) {
			draw_rect(cur_game, MAKER_X + 10 + 10 - 3, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 - 3, SPRITE_W * WIN_SCALE + 6, SPRITE_H * WIN_SCALE + 6, SDL_FALSE, darkred, SDL_FALSE, NULL);
			draw_rect(cur_game, MAKER_X + 10 + 10 - 2, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 - 2, SPRITE_W * WIN_SCALE + 4, SPRITE_H * WIN_SCALE + 4, SDL_FALSE, darkred, SDL_FALSE, NULL);
			draw_rect(cur_game, MAKER_X + 10 + 10 - 1, MAKER_Y + 10 + 20 + 10 + i*SPRITE_H*WIN_SCALE + i*42 - 1, SPRITE_W * WIN_SCALE + 2, SPRITE_H * WIN_SCALE + 2, SDL_FALSE, darkred, SDL_FALSE, NULL);
		}
	}
	/* Draw crafting parameters */
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 2, "CURRENT:");
	draw_rect(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 15, MAKER_Y + 10 + 20, SPRITE_W * WIN_SCALE * 3 + 2, SPRITE_H * WIN_SCALE * 3 + 2, SDL_FALSE, white, SDL_FALSE, NULL);
	if (cur_player->recipe[cur_craft->current - 1] != 0) {
		draw_tile(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 1 + 15, MAKER_Y + 10 + 20 + 1, SPRITE_W * WIN_SCALE * 3, SPRITE_H * WIN_SCALE * 3, get_loot_sprite(R_TABLE[cur_craft->current - 1].item_id), 255);
	} else {
		rect.x = MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 1 + 15; rect.y = MAKER_Y + 10 + 20 + 1; rect.w = SPRITE_W * WIN_SCALE * 3; rect.h = SPRITE_H * WIN_SCALE * 3;
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[6], NULL, &rect);
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
	rect.x = MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 6 + 2;
	rect.y = MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4 + 20 + 10 + 50;
	rect.w = 230;
	rect.h = 152;
	/* If quantity is insufficient, say "INSUFFICIENT MATERIALS" */
	if (cur_player->recipe[cur_craft->current - 1] == 0 || cur_craft->quantity > determine_max(cur_player, cur_craft) || cur_craft->quantity == 0) {
		draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4 + 20, "\n    INSUFFICIENT\n      MATERIAL");
		cur_craft->sufficient = SDL_FALSE;
	} else {
		cur_craft->sufficient = SDL_TRUE;
	}
	if (cur_craft->sufficient == SDL_TRUE && cur_craft->quantity > 0 && cur_player->recipe[cur_craft->current - 1] != 0) {
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[0], NULL, &rect);
	} else {
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->craft[1], NULL, &rect);
	}
}

static void
mouse_click_craft(int x, int y, struct player *cur_player, struct craft_par *cur_craft)
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
		    	cur_craft->current = i + 1 + cur_craft->recipe;
		    	return;
		}
	}
	
	/* Clicked on scroll arrow */
	if (x >= MAKER_X + 10 + 10 + SPRITE_W * WIN_SCALE * 4 - 80 &&
	    x <= MAKER_X + 10 + 10 + SPRITE_W * WIN_SCALE * 4 - 80 + 69) {
	    	if (y >= MAKER_Y + 10 + 20 + 10 &&
		    y <= MAKER_Y + 10 + 20 + 10 + 69) {
		    	/* clicked up */
		    	cur_craft->recipe -= 1;
		} else if (y >= MAKER_Y + 10 + 20 + 10 + 5 * SPRITE_H * WIN_SCALE + 40 &&
			   y <= MAKER_Y + 10 + 20 + 10 + 5 * SPRITE_H * WIN_SCALE + 40 + 69) {
			/* clicked down */
			cur_craft->recipe += 1;
		}
		if (cur_craft->recipe < 0) cur_craft->recipe = 0;
		if (cur_craft->recipe > 4) cur_craft->recipe = 4;
	}
	
	/* Clicked on quantity box */
	if (x >= MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100 &&
	    x <= MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100 + 120 &&
	    y >= MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 &&
	    y <= MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 22) {
	    	cur_craft->enter_text = SDL_TRUE;
	    	return;
	}
	
	/* Clicked on craft buttons */
	if (cur_player->recipe[cur_craft->current - 1] == 0) {
		max = 0;
	} else {
		max = determine_max(cur_player, cur_craft);
	}
	if (x >= 661 && x <= 733 &&
	    y >= 375 && y <= 411) {
	    	cur_craft->quantity = 1;
	} else if (x >= 737 && x <= 809 &&
		   y >= 375 && y <= 411) {
	    	cur_craft->quantity = 10;
	} else if (x >= 813 && x <= 885 &&
		   y >= 375 && y <= 411) {
	    	cur_craft->quantity = 100;
	} else if (x >= 661 && x <= 885 &&
		   y >= 414 && y <= 450) {
	    	cur_craft->quantity = max;
	} else if (x >= 661 && x <= 733 &&
		   y >= 452 && y <= 488) {
	    	cur_craft->quantity += 1;
	} else if (x >= 737 && x <= 809 &&
		   y >= 452 && y <= 488) {
	    	cur_craft->quantity += 10;
	} else if (x >= 813 && x <= 885 &&
		   y >= 452 && y <= 488) {
	    	cur_craft->quantity += 100;
	} else if (cur_craft->sufficient == SDL_TRUE &&
		   x >= 661 && x <= 885 &&
		   y >= 492 && y <= 527) {
		craft_it(cur_player, cur_craft);
		check_recipes(cur_player);
	}
}

static void
change_quantity(int number, struct craft_par *cur_craft)
{
	int tmp_quantity;
		
	if (number >= 0) {
		/* Make sure new quantity is not over MAX_STACK */
		tmp_quantity = cur_craft->quantity * 10;
		if (tmp_quantity >= MAX_STACK) {
			return;
		} else {
			cur_craft->quantity = tmp_quantity += number;
		}
	} else {
		cur_craft->quantity /= 10;
	}	
}

static unsigned short int
determine_max(struct player *cur_player, struct craft_par *cur_craft)
{
	int i;
	unsigned short int item_id;
	unsigned short int mat1, mat2;
	unsigned short int need1, need2;
	int have1 = 0, have2 = 0;
	unsigned short int max_quantity;
	
	/* Get recipe from recipe table */
	item_id = R_TABLE[cur_craft->current - 1].item_id;
	mat1 = R_TABLE[cur_craft->current - 1].mat1;
	mat2 = R_TABLE[cur_craft->current - 1].mat2;
	need1 = R_TABLE[cur_craft->current - 1].need1;
	need2 = R_TABLE[cur_craft->current - 1].need2;
	
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
	/* Is it over MAX_STACK? */
	if (max_quantity > MAX_STACK) max_quantity = MAX_STACK;
	return max_quantity;
}

static void
craft_it(struct player *cur_player, struct craft_par *cur_craft)
{
	int i;
	unsigned short int item_id;
	unsigned short int mat1, mat2;
	unsigned short int need1, need2;
	unsigned short int made;
	
	/* Not trying to make any */
	if (cur_craft->quantity == 0) {
		printf("None were made!\n");
		return;
	}
	
	/* Get recipe from recipe table */
	item_id = R_TABLE[cur_craft->current - 1].item_id;
	mat1 = R_TABLE[cur_craft->current - 1].mat1;
	mat2 = R_TABLE[cur_craft->current - 1].mat2;
	need1 = R_TABLE[cur_craft->current - 1].need1;
	need2 = R_TABLE[cur_craft->current - 1].need2;	
	
	/* Place items in inventory */
	/* Looking for existing stacks and trying to put them in */
	for (made = 0, i = 0; i < MAX_INV; i++) {
		if (cur_player->loot[i] == item_id) {
			if (cur_player->quantity[i] + cur_craft->quantity - made <= MAX_STACK) {
				cur_player->quantity[i] += cur_craft->quantity - made;
				made = cur_craft->quantity;
				break;
			} else {
				made += MAX_STACK - cur_player->quantity[i];
				cur_player->quantity[i] = MAX_STACK;
			}
		}
	}
	/* Do I have any left to put in empty spaces? */
	if (made < cur_craft->quantity) {
		/* Try to put it in empty spots */
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == 0) {
				cur_player->loot[i] = item_id;
				cur_player->quantity[i] = cur_craft->quantity - made;
				made = cur_craft->quantity;
				break;
			}
		}
	}
	
	/* Can't fit any */
	if (made == 0) {
		printf("No inventory room!\n");
		return;
	}

	/* Calculate needs */
	need1 *= made;
	need2 *= made;
	
	/* Check for first item */
	for (i = 0; i < MAX_INV; i++) {
		if (cur_player->loot[i] == mat1) {
			if (cur_player->quantity[i] >= need1) {
				cur_player->quantity[i]	-= need1;
				need1 = 0;
				if (cur_player->quantity[i] == 0) cur_player->loot[i] = 0;
			} else {
				need1 -= cur_player->quantity[i];
				cur_player->loot[i] = 0;
			}
		} else if (cur_player->loot[i] == mat2) {
			if (cur_player->quantity[i] >= need2) {
				cur_player->quantity[i]	-= need2;
				need2 = 0;
				if (cur_player->quantity[i] == 0) cur_player->loot[i] = 0;
			} else {
				need2 -= cur_player->quantity[i];
				cur_player->loot[i] = 0;
			}
		}
	}
	
	/* If you couldn't craft them all, report it to user */
	if (made < cur_craft->quantity) {
		printf("Made some, couldn't fit them all!\n");
	} else {
		printf("Crafted everything!\n");
	}
}

void
check_recipes(struct player *cur_player) {
	int i;
	unsigned short int max;
	struct craft_par test_craft;
	
	/* Loop through every possible recipe and check the max */
	for (i = 1; i < MAX_RECIPE + 1; i++) {
		test_craft.current = i;
		if (cur_player->recipe[i - 1] == 0) {
			max = determine_max(cur_player, &test_craft);
			if (max > 0) {
				cur_player->recipe[i - 1] = 1;
			}
		}
	}
}
