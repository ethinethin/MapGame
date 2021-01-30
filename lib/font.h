#ifndef FONT_H
#define FONT_H

#include "main.h"

extern void	draw_sentence(struct game *cur_game, int x, int y, const char *sentence);
extern void	draw_small_sentence(struct game *cur_game, int x, int y, const char *sentence);
extern void	load_font(struct game *cur_game);
extern void	unload_font(struct game *cur_game);

#endif
