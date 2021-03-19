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
	cur_player->quantity[0] = 999;
	cur_player->loot[1] = 9;
	cur_player->quantity[1] = 999;
	cur_player->loot[2] = 10;
	cur_player->quantity[2] = 999;
	cur_player->loot[3] = 11;
	cur_player->quantity[3] = 10;
	cur_player->loot[4] = 13;
	cur_player->quantity[4] = 10;
	cur_player->loot[8] = 14;
	cur_player->quantity[8] = 100;
	cur_player->loot[9] = 15;
	cur_player->quantity[9] = 100;
	cur_player->loot[10] = 16;
	cur_player->quantity[10] = 100;
	cur_player->loot[11] = 17;
	cur_player->quantity[11] = 100;
	cur_player->loot[12] = 18;
	cur_player->quantity[12] = 100;
	cur_player->loot[13] = 19;
	cur_player->quantity[13] = 100;
	cur_player->loot[14] = 20;
	cur_player->quantity[14] = 100;
	cur_player->loot[15] = 21;
	cur_player->quantity[15] = 100;
}
