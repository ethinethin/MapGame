#include <SDL2/SDL.h>
#include "font.h"
#include "main.h"

static void	draw_char(struct game *cur_game, int x, int y, int letter, float scale);

int FONT_ALPHA = 255;
void
set_font_alpha(int alpha)
{
	FONT_ALPHA = alpha;
}

static void
draw_char(struct game *cur_game, int x, int y, int letter, float scale)
{
	SDL_SetTextureAlphaMod(cur_game->font[letter], FONT_ALPHA);
	SDL_Rect rect = {x, y, 16 * scale, 18 * scale};
	SDL_RenderCopyEx(cur_game->screen.renderer, cur_game->font[letter], NULL, &rect, 0, NULL, 0);
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
			x += 16;
		}
		if (x >= (WIN_W - GAME_X) || sentence[i] == '\n') {
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
	float scale = 0.70;

	for (i = 0, len = strlen(sentence); i < len; i++) {
		if (sentence[i] != '\n') {
			draw_char(cur_game, x, y, sentence[i] - 32, scale);
			x += 16 * scale;
		}
		if (x >= (WIN_W - GAME_X)  || sentence[i] == '\n') {
			x = start_x;
			y = y + 18 * scale;
		}
	}
}

void
load_font(struct game *cur_game)
{
	int i, j;
	SDL_Surface *image;
	SDL_Surface *tmp;
	SDL_Rect rect = {0, 0, 16, 18};

	/* Allocate memory for 96 font characters */
	cur_game->font = malloc(sizeof(*cur_game->font)*95);
	/* Load sprite sheet */
	image = SDL_LoadBMP("art/font.bmp");
	/* Load all sprites */
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (i*10+j == 95) break;
			rect.x = j*16;
			rect.y = i*18;
			tmp = SDL_CreateRGBSurface(0, 16, 18, 24, 0x00, 0x00, 0x00, 0x00);
			SDL_SetColorKey(tmp, 1, 0xFF00FF);
			SDL_BlitSurface(image, &rect, tmp, NULL);
			cur_game->font[i*10+j] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tmp);
			SDL_SetTextureBlendMode(cur_game->font[i*10+j], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(tmp);		
		}
	}
	SDL_FreeSurface(image);
}

void
unload_font(struct game *cur_game)
{
	int i;

	/* Free all font characters */
	for (i = 0; i < 95; i++) {
		SDL_DestroyTexture(cur_game->font[i]);
	}
	free(cur_game->font);
}
