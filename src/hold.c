#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "hold.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"

struct holder {
	int x;
	int y;
	unsigned short int loot[40];
	unsigned short int quantity[40];
	struct holder *next;
} *HOLDERS;

struct mouse {
	int x;
	int y;
	int offset_x;
	int offset_y;
	SDL_bool mdown;
	int button;
} MOUSE;

struct coords {
	int x;
	int y;
};

#define QB_CLICK 1
#define INV_CLICK 2
#define HOLD_CLICK 3

struct item_clicked {
	char click_location;
	short int loot_location;
	short int loot;
	short int quantity;
	struct holder *holder;
};

/* Function prototypes */

static void		 item_click_qb(struct item_clicked *cur_click, struct player *cur_player);
static void		 item_click_inv(struct item_clicked *cur_click, struct player *cur_player);
static void		 item_click_hold(struct item_clicked *cur_click);
static void		 item_click_location(struct item_clicked *cur_click, struct player *cur_player);
static void		 handle_hold_swap(struct game *cur_game, struct player *cur_player, struct item_clicked *item_click, struct item_clicked *item_drop, unsigned short int quantity);
static void		 swap_usints(unsigned short int *usint1, unsigned short int *usint2);
static void		 draw_chest(struct game *cur_game, struct worldmap *map, int x, int y, struct holder *cur_holder);
static struct holder	*get_holder(int x, int y);
static void		 drag_item(struct game *cur_game, short int loot);
static void		 place_in_hold(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y, struct item_clicked *item_click);
static void		 update_quantity(struct item_clicked *item_click, struct player *cur_player);


void
setup_hold(void)
{
	struct holder *first;
	
	/* Make first (null) entry in holder table */
	first = malloc(sizeof(*first)*1);
	first->next = NULL;
	HOLDERS = first;
}

void
add_hold(int x, int y)
{
	int i;
	struct holder *new;
	struct holder *tmp;
	
	/* Create and populate a new holder */
	new = malloc(sizeof(*new)*1);
	new->x = x;
	new->y = y;
	for (i = 0; i < 40; i++) {
		new->loot[i] = 0;
		new->quantity[i] = 0;
	}
	new->next = NULL;
	
	/* Add it to the holder table */
	tmp = HOLDERS;
	while (SDL_TRUE) {
		if (tmp->next == NULL) {
			tmp->next = new;
			return;
		} else {
			tmp = tmp->next;
		}
	}
}

void
del_hold(int x, int y)
{
	struct holder *tmp;
	struct holder *last;
	
	/* Run through the holder table and find the one we need to kill */
	tmp = HOLDERS;
	while (tmp->next != NULL) {
		last = tmp;
		tmp = tmp->next;
		if (tmp->x == x && tmp->y == y) {
			/* Remove from linked list */
			last->next = tmp->next;
			free(tmp);
			tmp = last;
		}
	}
}

void
kill_hold(void)
{
	struct holder *tmp;
	struct holder *next;
	
	/* Delete all entries from the holder table */
	next = HOLDERS;
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
open_chest(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	char white[3] = { 255, 255, 255 };
	struct item_clicked item_click;
	struct item_clicked item_drop;
	
	/* Get holder */
	item_click.holder = get_holder(x, y);
	item_drop.holder = item_click.holder;
	item_click.loot = 0;
	/* Enter input loop */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while(finished == SDL_FALSE) {
		/* Redraw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_chest(cur_game, map, x, y, item_click.holder);
		if (MOUSE.mdown == SDL_TRUE && item_click.loot != 0) {
			drag_item(cur_game, item_click.loot);
		}
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_i) {
					toggle_inv(cur_game);
				} else {
					finished = SDL_TRUE;
				}
				break;
			case SDL_MOUSEMOTION:
				MOUSE.x = event.motion.x;
				MOUSE.y = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				MOUSE.button = event.button.button;
				/* If you click outside of inventory spaces, close the chest */
				if (!(
					(MOUSE.x >= QB_X && MOUSE.x <= QB_X + QB_W && MOUSE.y >= QB_Y && MOUSE.y <= QB_Y + QB_H) ||
					(cur_game->inventory == SDL_TRUE && MOUSE.x >= INV_X && MOUSE.x <= INV_X + INV_W && MOUSE.y >= INV_Y && MOUSE.y <= INV_Y + INV_H) ||
					(MOUSE.x >= HOLDER_X && MOUSE.x <= HOLDER_X + HOLDER_W && MOUSE.y >= HOLDER_Y && MOUSE.y <= HOLDER_Y + HOLDER_H)
				)) {
					finished = SDL_TRUE;
					continue;
				}
				/* Click in item locations */
				/* If left click: go into drag and drop */
				if (MOUSE.button == SDL_BUTTON_LEFT) {
					if (MOUSE.mdown == SDL_FALSE) {
						/* Determine location of click */
						item_click_location(&item_click, cur_player);
						MOUSE.mdown = SDL_TRUE;
					}
				} else if (MOUSE.button == SDL_BUTTON_RIGHT) {	
					/* If right click: go into placement mode */
					item_click_location(&item_click, cur_player);
					if (item_click.click_location != 0 && item_click.loot != 0) {
						place_in_hold(cur_game, map, cur_player, x, y, &item_click);
						item_click.loot = 0;
						MOUSE.mdown = SDL_FALSE;
					}
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (item_click.loot != 0) {
					/* Determine where dropping */
					item_click_location(&item_drop, cur_player);
					if (item_drop.click_location != 0) {
						handle_hold_swap(cur_game, cur_player, &item_click, &item_drop, item_click.quantity);
					}
				}
				MOUSE.mdown = SDL_FALSE;
				item_click.loot = 0;
				break;	
		}
	}
}

static void
item_click_qb(struct item_clicked *cur_click, struct player *cur_player)
{
	int i;
	
	cur_click->click_location = QB_CLICK;
	for (i = 0; i < 8; i++) {
		if (MOUSE.x > QB_X + SPRITE_W * WIN_SCALE * i && MOUSE.x <= QB_X + SPRITE_W * WIN_SCALE * (i+1)) {
			cur_click->loot_location = i;
			cur_click->loot = cur_player->loot[i];
			cur_click->quantity = cur_player->quantity[i];
			/* set mouse offset */
			MOUSE.offset_x = MOUSE.x - QB_X - i * SPRITE_W * WIN_SCALE;
			MOUSE.offset_y = MOUSE.y - QB_Y;
			break;
		}
	}
}

static void
item_click_inv(struct item_clicked *cur_click, struct player *cur_player)
{
	int i, j;
	
	cur_click->click_location = INV_CLICK;
	for (j = 0; j < 4; j++) {
		for (i = 0; i < 8; i++) {
			if (MOUSE.x > INV_X + j * SPRITE_W * WIN_SCALE && MOUSE.x <= INV_X + (j+1) * SPRITE_W * WIN_SCALE && MOUSE.y > INV_Y + i * SPRITE_H * WIN_SCALE * 1.25 && MOUSE.y <= INV_Y + (i+1) * SPRITE_H * WIN_SCALE * 1.25) {
				cur_click->loot_location = 8*j+i+8;
				cur_click->loot = cur_player->loot[8*j+i+8];
				cur_click->quantity = cur_player->quantity[8*j+i+8];
				/* set mouse offset */
				MOUSE.offset_x = MOUSE.x - INV_X - j * SPRITE_W * WIN_SCALE;
				MOUSE.offset_y = MOUSE.y - INV_Y - i * SPRITE_H * WIN_SCALE * 1.25;
				break;
			}
		}
	}
}

static void
item_click_hold(struct item_clicked *cur_click)
{
	int i, j;
	
	cur_click->click_location = HOLD_CLICK;
	for (j = 0; j < 10; j++) {
		for (i = 0; i < 4; i++) {
			if (MOUSE.x > HOLDER_X + j * SPRITE_W * WIN_SCALE && MOUSE.x <= HOLDER_X + (j+1) * SPRITE_W * WIN_SCALE && MOUSE.y > HOLDER_Y + i * SPRITE_H * WIN_SCALE * 1.25 && MOUSE.y <= HOLDER_Y + (i+1) * SPRITE_H * WIN_SCALE * 1.25) {
				cur_click->loot_location = 4*j+i;
				cur_click->loot = cur_click->holder->loot[4*j+i];
				cur_click->quantity = cur_click->holder->quantity[4*j+i];
				/* set mouse offset */
				MOUSE.offset_x = MOUSE.x - (HOLDER_X + j * SPRITE_W * WIN_SCALE);
				MOUSE.offset_y = MOUSE.y - (HOLDER_Y + i * SPRITE_H * WIN_SCALE * 1.25);
				break;
			}
		}
	}
}

static void
item_click_location(struct item_clicked *cur_click, struct player *cur_player)
{
	if (MOUSE.x >= QB_X && MOUSE.x <= QB_X + QB_W && MOUSE.y >= QB_Y && MOUSE.y <= QB_Y + QB_H) {
		/* clicked in quickbar */
		item_click_qb(cur_click, cur_player);
	} else if (MOUSE.x >= INV_X && MOUSE.x <= INV_X + INV_W && MOUSE.y >= INV_Y && MOUSE.y <= INV_Y + INV_H) {
		/* clicked in inventory */
		item_click_inv(cur_click, cur_player);
	} else if (MOUSE.x >= HOLDER_X && MOUSE.x <= HOLDER_X + HOLDER_W && MOUSE.y >= HOLDER_Y && MOUSE.y <= HOLDER_Y + HOLDER_H) {
		/* clicked in holder */
		item_click_hold(cur_click);
	} else {
		cur_click->click_location = 0;
	}
}

static void
handle_hold_swap(struct game *cur_game, struct player *cur_player, struct item_clicked *item_click, struct item_clicked *item_drop, unsigned short int quantity)
{
	unsigned short int *click_loot, *click_quantity, *drop_loot, *drop_quantity;
	
	/* Make sure you're not moving to the same slot */
	if (item_click->click_location == item_drop->click_location && item_click->loot_location == item_drop->loot_location) return;
	/* Find source and destination location and quantity */
	if (item_click->click_location <= INV_CLICK) {
		/* source is inventory */
		click_loot = &cur_player->loot[item_click->loot_location];
		click_quantity = &cur_player->quantity[item_click->loot_location];
	} else {
		/* source is holder */
		click_loot = &item_click->holder->loot[item_click->loot_location];
		click_quantity = &item_click->holder->quantity[item_click->loot_location];
	}
	if (item_drop->click_location <= INV_CLICK) {
		/* destination is inventory */
		drop_loot = &cur_player->loot[item_drop->loot_location];
		drop_quantity = &cur_player->quantity[item_drop->loot_location];
		cur_game->cursor = (char) item_drop->loot_location;
	} else {
		/* destination is holder */
		drop_loot = &item_drop->holder->loot[item_drop->loot_location];
		drop_quantity = &item_drop->holder->quantity[item_drop->loot_location];
	}
	/* Swap objects if click quantity is equal to quantity */
	if (*click_quantity == quantity) {
		if (*click_loot != *drop_loot) {
			/* Loot is different, swap it */
			swap_usints(click_loot, drop_loot);
			swap_usints(click_quantity, drop_quantity);
		} else if (is_loot_stackable(*click_loot) == UNSTACKABLE) {
			/* Loot is the same, but unstackable, so don't do anything */
			return;
		} else if (*click_quantity + *drop_quantity <= MAX_STACK) {
			/* Loot is the same and stackable with MAX_STACK or less total */
			*drop_quantity = *click_quantity + *drop_quantity;
			*click_loot = 0;
			*click_quantity = 0;
		} else {
			/* Loot is the same and stackable with more than MAX_STACK */
			*click_quantity -= MAX_STACK - *drop_quantity;
			*drop_quantity = MAX_STACK;
		}
	} else if ((*click_loot == *drop_loot && *drop_quantity < MAX_STACK) || *drop_loot == 0) {
		/* Otherwise, we're placing 1 item */
		*drop_loot = *click_loot;
		*drop_quantity += 1;
		*click_quantity -= 1;
		/* Dropped the last one */
		if (*click_quantity == 0) *click_loot = 0;
	}
}

static void
swap_usints(unsigned short int *usint1, unsigned short int *usint2)
{
	unsigned short int tmp;
	tmp = *usint1;
	*usint1 = *usint2;
	*usint2 = tmp;
}

static void
draw_chest(struct game *cur_game, struct worldmap *map, int x, int y, struct holder *cur_holder)
{
	char stackable;
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	char quantity[4];
	short int sprite_index;
	int i, j;

	/* Draw holder with grid  */
	draw_rect(cur_game, HOLDER_X, HOLDER_Y, HOLDER_W, HOLDER_H, SDL_TRUE, black, SDL_TRUE, white);
	for (i = 0; i < 10; i++) {
		draw_line(cur_game,
			  HOLDER_X + i * SPRITE_W * WIN_SCALE,
			  HOLDER_Y,
			  HOLDER_X + i * SPRITE_W * WIN_SCALE,
			  HOLDER_Y + HOLDER_H,
			  white);
		if (i < 4) {
			draw_line(cur_game,
				  HOLDER_X,
				  HOLDER_Y + i * SPRITE_W * WIN_SCALE * 1.25,
				  HOLDER_X + HOLDER_W,
				  HOLDER_Y + i * SPRITE_W * WIN_SCALE * 1.25,
				  white);
		}
	}
	/* Draw holder title */
	draw_rect(cur_game, HOLDER_X, HOLDER_Y - 20, HOLDER_W, 18 + 3, SDL_TRUE, black, SDL_TRUE, white);
	draw_small_sentence(cur_game, HOLDER_X + 2, HOLDER_Y - 17, get_loot_name(*(*(map->loot+y)+x)));
	draw_line(cur_game, HOLDER_X + HOLDER_W - 15, HOLDER_Y - 15, HOLDER_X + HOLDER_W - 5, HOLDER_Y - 5, white);
	draw_line(cur_game, HOLDER_X + HOLDER_W - 6, HOLDER_Y - 15, HOLDER_X + HOLDER_W - 16, HOLDER_Y - 5, white);
	/* Draw loot tiles for holder */
	for (j = 0; j < 10; j++) {
		for (i = 0; i < 4; i++) {
			if (cur_holder->loot[j*4+i] != 0) {
				sprite_index = get_loot_sprite(cur_holder->loot[j*4+i]);
				draw_tile(cur_game,
					  HOLDER_X + j * SPRITE_W * WIN_SCALE + 1,
					  HOLDER_Y + i * SPRITE_H * WIN_SCALE * 1.25 + 2,
					  SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE,
					  sprite_index,
					  255);
				stackable = is_loot_stackable(cur_holder->loot[j*4+i]);
				if (stackable == STACKABLE) {
					sprintf(quantity, "%3d", cur_holder->quantity[j*4+i]);
					draw_small_sentence(cur_game,
							    HOLDER_X + j * SPRITE_W * WIN_SCALE + 1 + 32,
							    HOLDER_Y + i * SPRITE_H * WIN_SCALE * 1.25 + SPRITE_H * WIN_SCALE + 4,
							    quantity);
				}
			}
		}
	}
}

SDL_bool
chest_empty(int x, int y)
{
	struct holder *tmp;
	int i;
	int num;
	
	/* Find holder from list */
	tmp = HOLDERS;
	while (SDL_TRUE) {
		tmp = tmp->next;
		if (tmp->x == x && tmp->y == y) break;
	}
	
	/* Check each item spot in holder */
	for (i = 0, num = 0; i < 40; i++) {
		if (tmp->loot[i] != 0) num++;
	}
	if (num == 0) {
		del_hold(x, y);
		return SDL_TRUE;
	} else {
		return SDL_FALSE;
	}
}

static struct holder *
get_holder(int x, int y)
{
	struct holder *tmp;
	
	tmp = HOLDERS;
	while (SDL_TRUE) {
		tmp = tmp->next;
		if (tmp->x == x && tmp->y == y) return tmp;
	}
	return NULL;
}

static void
drag_item(struct game *cur_game, short int loot)
{
	draw_tile(cur_game, MOUSE.x - MOUSE.offset_x, MOUSE.y - MOUSE.offset_y, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE,
	          get_loot_sprite(loot), 192);
}

static void
place_in_hold(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y, struct item_clicked *item_click)
{
	char white[3] = { 255, 255, 255 };
	struct item_clicked item_drop;
	
	/* Set drop holder equal to the holder being viewed */
	item_drop.holder = item_click->holder;

	/* Enter input loop */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while(finished == SDL_FALSE) {
		/* Redraw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_chest(cur_game, map, x, y, item_click->holder);
		drag_item(cur_game, item_click->loot);
		render_present(cur_game);
		SDL_Delay(10);
		
		/* Poll for mouse state */
		if (SDL_PollEvent(&event) == 0) continue;
		switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_i:
						toggle_inv(cur_game);
						break;
					default:
						finished = SDL_TRUE;
						break;
				}
				continue;
				break;
			case SDL_MOUSEMOTION:
				MOUSE.x = event.motion.x;
				MOUSE.y = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				/* Find where you clicked */
				item_click_location(&item_drop, cur_player);
				if (item_click->click_location == item_drop.click_location &&
				    item_click->loot_location == item_drop.loot_location) {
				    	/* Clicking on the stack you're dropping, exit */
				    	finished = SDL_TRUE;
				    	continue;
				}
				/* Left click, tried to place item */
				if (event.button.button == SDL_BUTTON_LEFT) {
					/* If you click outside of inventory spaces, close the chest */
					if (!(
						(MOUSE.x >= QB_X && MOUSE.x <= QB_X + QB_W && MOUSE.y >= QB_Y && MOUSE.y <= QB_Y + QB_H) ||
						(cur_game->inventory == SDL_TRUE && MOUSE.x >= INV_X && MOUSE.x <= INV_X + INV_W && MOUSE.y >= INV_Y && MOUSE.y <= INV_Y + INV_H) ||
						(MOUSE.x >= HOLDER_X && MOUSE.x <= HOLDER_X + HOLDER_W && MOUSE.y >= HOLDER_Y && MOUSE.y <= HOLDER_Y + HOLDER_H)
					)) {
						finished = SDL_TRUE;
						continue;
					}
					if (item_drop.click_location != 0) {
						handle_hold_swap(cur_game, cur_player, item_click, &item_drop, 1);
						/* Update quantity in item_drop */
						update_quantity(item_click, cur_player);
						/* Item stack depleted */
						if (item_click->quantity == 0) finished = SDL_TRUE;
						continue;
					}
				} else if (event.button.button == SDL_BUTTON_RIGHT) {
					finished = SDL_TRUE;
					continue;
				}
				break;
		}
	}
}

static void
update_quantity(struct item_clicked *item_click, struct player *cur_player)
{
	if (item_click->click_location <= INV_CLICK) {
		/* source is inventory */
		item_click->quantity = cur_player->quantity[item_click->loot_location];
	} else {
		/* source is holder */
		item_click->quantity = item_click->holder->quantity[item_click->loot_location];
	}
}
