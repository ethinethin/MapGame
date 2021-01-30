#include <SDL2/SDL.h>
#include "font.h"
#include "main.h"

static void	draw_char(struct game *cur_game, int x, int y, int letter, float scale);

static void
draw_char(struct game *cur_game, int x, int y, int letter, float scale)
{
	SDL_Rect rect = {x, y, 28 * scale, 18 * scale};
	SDL_Texture* texture = SDL_CreateTextureFromSurface(cur_game->screen.renderer, cur_game->font[letter]);
	SDL_RenderCopyEx(cur_game->screen.renderer, texture, NULL, &rect, 0, NULL, 0);
	SDL_DestroyTexture(texture);
}

void
draw_sentence(struct game *cur_game, int x, int y, const char *sentence)
{
	int i;
	int start_x = x;
	int len;

	for (i = 0, len = strlen(sentence); i < len; i++) {
		draw_char(cur_game, x, y, sentence[i] - 32, 1);
		x += 28;
		if (x >= WIN_W - GAME_X) {
			x = start_x;
			y = y + 18;
		}
	}
}

void
draw_small_sentence(struct game *cur_game, int x, int y, const char *sentence)
{
	int i;
	int start_x = x;
	int len;
	float scale = 0.65;

	for (i = 0, len = strlen(sentence); i < len; i++) {
		draw_char(cur_game, x, y, sentence[i] - 32, scale);
		x += 28 * scale - 4;
		if (x >= WIN_W - GAME_X) {
			x = start_x;
			y = y + 18 * scale;
		}
	}
}

void
load_font(struct game *cur_game)
{
	int i, j;
	int count = 0;
	SDL_Surface* surface;
	SDL_Rect rect = {1, 1, 28, 18};

	/* Allocate memory for 96 font characters */
	cur_game->font = (SDL_Surface**) malloc(sizeof(SDL_Surface*)*96);
	/* Load sprite sheet */
	surface = SDL_LoadBMP("art/font.bmp");
	/* Load all sprites */
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 16; j++) {
			rect.x = i*28 + i + 1;
			rect.y = j*18 + j + 1;
			cur_game->font[count] = SDL_CreateRGBSurface(0, 28, 18, 24, 0, 0, 0, 0);
			SDL_SetColorKey(cur_game->font[count], 1, 0x000000);
			SDL_FillRect(cur_game->font[count], 0, 0x000000);
			SDL_BlitSurface(surface, &rect, cur_game->font[count], NULL);
			count++;
		}
	}
	SDL_FreeSurface(surface);
}

void
unload_font(struct game *cur_game)
{
	int i;

	/* Free all font characters */
	for (i = 0; i < 96; i++) {
		SDL_FreeSurface(cur_game->font[i]);
	}
	free(cur_game->font);
}
