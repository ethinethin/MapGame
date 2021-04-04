#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "main.h"
#include "make.h"
#include "maps.h"
#include "play.h"

/* Function prototypes */
static void	draw_make(struct game *cur_game, unsigned short int current, unsigned short int quantity);

void
make_stuff(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	unsigned short int current = 1;
	unsigned short int quantity = 1;
	SDL_bool finished = SDL_FALSE;
	SDL_Event event;
	
	while (finished == SDL_FALSE) {
		/* Redraw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_make(cur_game, current, quantity);
		render_present(cur_game);
		SDL_Delay(10);
		/* check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_i:
					toggle_inv(cur_game);
					break;
				default:
					finished = SDL_TRUE;
					break;
			}
		}
	}

}

static void
draw_make(struct game *cur_game, unsigned short int current, unsigned short int quantity)
{
	char black[3] = { 0, 0, 0};
	char white[3] = { 255, 255, 255 };
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
	}
	/* Draw crafting parameters */
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 2, "CURRENT:");
	draw_rect(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 15, MAKER_Y + 10 + 20, SPRITE_W * WIN_SCALE * 3 + 2, SPRITE_H * WIN_SCALE * 3 + 2, SDL_FALSE, white, SDL_FALSE, NULL);
	if (current != 0) {
		draw_tile(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 1 + 15, MAKER_Y + 10 + 20 + 1, SPRITE_W * WIN_SCALE * 3, SPRITE_H * WIN_SCALE * 3, sprites[current - 1], 255);
	}
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 2, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4, "QUANTITY:");
	draw_rect(cur_game,
		  MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100,
		  MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10,
		  120,
		  22,
		  SDL_FALSE, white, SDL_FALSE, NULL);
	sprintf(amount, "%d", quantity);
	draw_small_sentence(cur_game, MAKER_X + 10 + SPRITE_W * WIN_SCALE * 4 + 10 + 100 + 4, MAKER_Y + 10 + 20 + SPRITE_H * WIN_SCALE * 3 + 2 + 10 + 4, amount);
}
