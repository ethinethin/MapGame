#ifndef DISP_H
#define DISP_H

#include <SDL2/SDL.h>
#include "main.h"
#include "maps.h"
#include "play.h"

extern void	display_init(struct game *cur_game);
extern void	display_quit(struct game *cur_game);
extern void	render_clear(struct game *cur_game);
extern void	render_present(struct game *cur_game);
extern void	draw_rect(struct game *cur_game, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
			  SDL_bool fill, char *fill_col, SDL_bool border, char *bord_col);
extern void	draw_tile(struct game *cur_game, int x, int y, int w, int h, int sprite_index, unsigned char alpha);
extern void	draw_all(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void	draw_game(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void	worldmap(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void	draw_sentence(struct game *cur_game, int start_x, int start_y, const char* sentence);
extern void	loading_bar(struct game *cur_game, char *title, int percentage);

#endif
