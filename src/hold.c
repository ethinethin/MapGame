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
static void		 handle_hold_swap(struct item_clicked *item_click, struct item_clicked *item_drop);
static void		 draw_chest(struct game *cur_game, struct worldmap *map, int x, int y, struct holder *cur_holder);
static struct holder	*get_holder(int x, int y);
static void		 drag_item(struct game *cur_game, short int loot);

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
	new->loot[10] = 1;
	new->quantity[10] = 10;
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
	SDL_Rect rect = { 0, 0, WIN_W, WIN_H };
	struct item_clicked item_click;
	struct item_clicked item_drop;
	
	/* Get holder */
	item_click.holder = get_holder(x, y);
	item_drop.holder = item_click.holder;
	item_click.loot = 0;
	
	/* Enter input loop */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	SDL_bool redraw = SDL_TRUE;
	/* Draw screen to texture */
	SDL_Texture *texture;
	texture = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
				    SDL_TEXTUREACCESS_TARGET, WIN_W, WIN_H);
	SDL_SetRenderTarget(cur_game->screen.renderer, texture);
	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	draw_chest(cur_game, map, x, y, item_click.holder);
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
	while(finished == SDL_FALSE) {
		SDL_RenderCopy(cur_game->screen.renderer, texture, NULL, &rect); 
		render_present(cur_game);
		if (redraw == SDL_TRUE || (MOUSE.mdown == SDL_TRUE && item_click.loot != 0)) {
			/* Redraw screen to texture */
			SDL_DestroyTexture(texture);
			texture = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
				    SDL_TEXTUREACCESS_TARGET, WIN_W, WIN_H);
			SDL_SetRenderTarget(cur_game->screen.renderer, texture);
			draw_game(cur_game, map, cur_player);
			draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
			draw_chest(cur_game, map, x, y, item_click.holder);
			/* Draw cursor? */
			if (MOUSE.mdown == SDL_TRUE && item_click.loot != 0) {
				drag_item(cur_game, item_click.loot);
			}
			/* Render screen */
			SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
			redraw = SDL_FALSE;
		}
		/* Check for input */
		SDL_PollEvent(&event);
		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_i) {
					toggle_inv(cur_game);
					redraw = SDL_TRUE;
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
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (item_click.loot != 0) {
					/* Determine where dropping */
					item_click_location(&item_drop, cur_player);
					if (item_drop.click_location != 0) {
						handle_hold_swap(&item_click, &item_drop);
					}
				}
				MOUSE.mdown = SDL_FALSE;
				item_click.loot = 0;
				redraw = SDL_TRUE;
				break;	
		}
		SDL_Delay(10);
	}
	SDL_DestroyTexture(texture);
}

static void
item_click_qb(struct item_clicked *cur_click, struct player *cur_player)
{
	int i;
	
	cur_click->click_location = QB_CLICK;
	for (i = 0; i < 8; i++) {
		if (MOUSE.x > QB_X + 48*i && MOUSE.x <= QB_X + 48*(i+1)) {
			cur_click->loot_location = i;
			cur_click->loot = cur_player->loot[i];
			cur_click->quantity = cur_player->quantity[i];
			/* set mouse offset */
			MOUSE.offset_x = MOUSE.x - QB_X - i*48;
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
			if (MOUSE.x > INV_X + j*48 && MOUSE.x <= INV_X + (j+1)*48 && MOUSE.y > INV_Y + i*60 && MOUSE.y <= INV_Y + (i+1)*60) {
				cur_click->loot_location = 8*j+i+8;
				cur_click->loot = cur_player->loot[8*j+i+8];
				cur_click->quantity = cur_player->quantity[8*j+i+8];
				/* set mouse offset */
				MOUSE.offset_x = MOUSE.x - INV_X - j*48;
				MOUSE.offset_y = MOUSE.y - INV_Y - i*60;
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
			if (MOUSE.x > HOLDER_X + j*48 && MOUSE.x <= HOLDER_X + (j+1)*48 && MOUSE.y > HOLDER_Y + i*60 && MOUSE.y <= HOLDER_Y + (i+1)*60) {
				cur_click->loot_location = 4*j+i;
				cur_click->loot = cur_click->holder->loot[4*j+i];
				cur_click->quantity = cur_click->holder->quantity[4*j+i];
				/* set mouse offset */
				MOUSE.offset_x = MOUSE.x - HOLDER_X - j*48;
				MOUSE.offset_y = MOUSE.y - HOLDER_Y - i*60;
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
handle_hold_swap(struct item_clicked *item_click, struct item_clicked *item_drop)
{

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
			  HOLDER_X + i*48,
			  HOLDER_Y,
			  HOLDER_X + i*48,
			  HOLDER_Y + HOLDER_H,
			  white);
		if (i < 5) {
			draw_line(cur_game,
				  HOLDER_X,
				  HOLDER_Y + i*60,
				  HOLDER_X + HOLDER_W,
				  HOLDER_Y + i*60,
				  white);
		}
	}
	/* Draw holder title */
	draw_rect(cur_game, HOLDER_X, HOLDER_Y - 20, HOLDER_W, 18 + 3, SDL_TRUE, black, SDL_TRUE, white);
	draw_small_sentence(cur_game, HOLDER_X + 2, HOLDER_Y - 17, get_loot_name(*(*(map->loot+y)+x)));
	draw_line(cur_game, HOLDER_X + HOLDER_W - 15, HOLDER_Y - 15, HOLDER_X + HOLDER_W - 5, HOLDER_Y - 5, white);
	draw_line(cur_game, HOLDER_X + HOLDER_W - 5, HOLDER_Y - 15, HOLDER_X + HOLDER_W - 15, HOLDER_Y - 5, white);
	/* Draw loot tiles for holder */
	for (j = 0; j < 10; j++) {
		for (i = 0; i < 4; i++) {
			if (cur_holder->loot[j*4+i] != 0) {
				sprite_index = get_loot_sprite(cur_holder->loot[j*4+i]);
				draw_tile(cur_game,
					  HOLDER_X + j*48 + 1,
					  HOLDER_Y + i*60 + 2,
					  SPRITE_W * 1.5, SPRITE_H * 1.5,
					  sprite_index,
					  255);
				stackable = is_loot_stackable(cur_holder->loot[j*4+i]);
				if (stackable == STACKABLE) {
					sprintf(quantity, "%3d", cur_holder->quantity[j*4+i]);
					draw_small_sentence(cur_game,
							    HOLDER_X + j*48 + 1,
							    HOLDER_Y + i*60 + 48,
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
	draw_tile(cur_game, MOUSE.x - MOUSE.offset_x, MOUSE.y - MOUSE.offset_y, 48, 48,
	          get_loot_sprite(loot), 192);
}
