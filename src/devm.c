#include "devm.h"
#include "disp.h"
#include "main.h"
#include "maps.h"
#include "play.h"

void
reveal_map(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char *tile_col;
	int rows, cols;
	
	/* Set up renderer to render to the map texture */
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->map_texture);
	/* Set all newly seen values to 1 */
	for (rows = 0; rows <= map->row_size - 1; rows++) {
		for (cols = 0; cols <= map->col_size - 1; cols++) {
			/* Set tile as seen */
			*(*(cur_player->seen+rows)+cols) = 1;
			/* Draw pixel on map texture */
			tile_col = get_color(*(*(map->tile + rows) + cols), *(*(map->biome + rows) + cols));
			draw_point(cur_game, cols, rows, tile_col);
		}
	}
	/* Reset renderer */
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);

}

void
give_me_floors(struct player *cur_player)
{
	cur_player->loot[0] = 8;
	cur_player->quantity[0] = 255;
	cur_player->loot[1] = 9;
	cur_player->quantity[1] = 255;
	cur_player->loot[2] = 10;
	cur_player->quantity[2] = 255;
}
