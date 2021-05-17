#include <SDL2/SDL.h>
#include "font.h"
#include "main.h"

static void	draw_char(struct game *cur_game, int x, int y, int letter, float scale);

static void
draw_char(struct game *cur_game, int x, int y, int letter, float scale)
{
	SDL_Rect rect = {x, y, 16 * scale * cur_game->screen.scale_x, 18 * scale * cur_game->screen.scale_y};
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
		if (sentence[i] != '\n') {
			draw_char(cur_game, x, y, sentence[i] - 32, 1);
			x += 16 * cur_game->screen.scale_x;
		}
		if (x >= (WIN_W - GAME_X)  * cur_game->screen.scale_x || sentence[i] == '\n') {
			x = start_x;
			y = y + 18 * cur_game->screen.scale_y;
		}
	}
}

void
draw_small_sentence(struct game *cur_game, int x, int y, const char *sentence)
{
	int i;
	int start_x = x;
	int len;
	float scale = 0.70;

	for (i = 0, len = strlen(sentence); i < len; i++) {
		if (sentence[i] != '\n') {
			draw_char(cur_game, x, y, sentence[i] - 32, scale);
			x += 16 * scale * cur_game->screen.scale_x;
		}
		if (x >= (WIN_W - GAME_X) * cur_game->screen.scale_x || sentence[i] == '\n') {
			x = start_x;
			y = y + 18 * scale * cur_game->screen.scale_y;
		}
	}
}

void
load_font(struct game *cur_game)
{
	int i, j;
	SDL_Surface* surface;
	SDL_Rect rect = {0, 0, 16, 18};

	/* Allocate memory for 96 font characters */
	cur_game->font = (SDL_Surface**) malloc(sizeof(SDL_Surface*)*95);
	/* Load sprite sheet */
	surface = SDL_LoadBMP("art/font.bmp");
	/* Load all sprites */
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (i*10+j == 95) break;
			rect.x = j*16;
			rect.y = i*18;
			cur_game->font[i*10+j] = SDL_CreateRGBSurface(0, 16, 18, 24, 0, 0, 0, 0);
			SDL_SetColorKey(cur_game->font[i*10+j], 1, 0xFF00FF);
			SDL_FillRect(cur_game->font[i*10+j], 0, 0xFF00FF);
			SDL_BlitSurface(surface, &rect, cur_game->font[i*10+j], NULL);
		}
	}
	SDL_FreeSurface(surface);
}

void
unload_font(struct game *cur_game)
{
	int i;

	/* Free all font characters */
	for (i = 0; i < 95; i++) {
		SDL_FreeSurface(cur_game->font[i]);
	}
	free(cur_game->font);
}
