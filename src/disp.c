#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "harv.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"

/* Structure for window position */
struct win_pos {
	int x;
	int y;
};

/* Function prototypes */
static void		draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win);
static void		update_winpos(struct player *cur_player, struct win_pos win);
static void		draw_player_indicator(struct game *cur_game, struct win_pos win, int size);
static void		draw_inv(struct game *cur_game, struct player *cur_player);
static void		draw_map(struct game *cur_game);
static struct win_pos	find_win_pos(struct worldmap *map, struct player *cur_player);
static void		load_sprites(struct game *cur_game);
static void		unload_sprites(struct game *cur_game);
static void		map_init(struct game *cur_game);
static void		map_destroy(struct game *cur_game);

void
display_init(struct game *cur_game)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL error: %s\n", SDL_GetError());
		exit(1);
	}
	/* Create the main window and renderer */
	cur_game->screen.window = SDL_CreateWindow(cur_game->screen.name,
						   0, 0,
						   //SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						   cur_game->screen.w, cur_game->screen.h, 0);
	cur_game->screen.renderer = SDL_CreateRenderer(cur_game->screen.window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(cur_game->screen.renderer, SDL_BLENDMODE_BLEND);
	/* Poll for event */
	SDL_Event event;
	while (SDL_PollEvent(&event) == 1);
	/* Load sprites and font */
	load_sprites(cur_game);
	load_font(cur_game);
	/* Initialize worldmap */
	map_init(cur_game);
	/* Setup scanlines */
	setup_scanlines(cur_game);
	/* Clear screen */
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
	SDL_RenderPresent(cur_game->screen.renderer);
}

void
display_quit(struct game *cur_game)
{	
	/* Destroy renderer and window */
	SDL_DestroyRenderer(cur_game->screen.renderer);
	cur_game->screen.renderer = NULL;
	SDL_DestroyWindow(cur_game->screen.window);
	cur_game->screen.window = NULL;
	/* Unload sprites and font */
	unload_sprites(cur_game);
	unload_font(cur_game);
	/* Kill the world map and scanlines */
	map_destroy(cur_game);
	SDL_DestroyTexture(cur_game->scanlines);
	/* SDL quit */
	SDL_Quit();
}

void
draw_point(struct game *cur_game, int x, int y, char *col)
{
	SDL_SetRenderDrawColor(cur_game->screen.renderer, *(col+0), *(col+1), *(col+2), 255);
	SDL_RenderDrawPoint(cur_game->screen.renderer, x, y);
}

void
draw_line(struct game *cur_game, int x1, int y1, int x2, int y2, char *col)
{
	SDL_SetRenderDrawColor(cur_game->screen.renderer, *(col+0), *(col+1), *(col+2), 255);
	SDL_RenderDrawLine(cur_game->screen.renderer, x1, y1, x2, y2);
}

void
render_clear(struct game *cur_game)
{
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
}

void
render_present(struct game *cur_game)
{
	SDL_Rect rect = {0, 0, cur_game->screen.w, cur_game->screen.h};
	
	/* draw scanlines */
	if (cur_game->scanlines_on == SDL_TRUE) {
		SDL_RenderCopy(cur_game->screen.renderer, cur_game->scanlines, NULL, &rect);
	}
	SDL_RenderPresent(cur_game->screen.renderer);
}

void
draw_rect(struct game *cur_game, unsigned int x, unsigned int y, unsigned int w, unsigned int h, SDL_bool fill, char *fill_col, SDL_bool border, char *bord_col)
{
	SDL_Rect coords = { x, y, w, h };
	SDL_SetRenderDrawColor(cur_game->screen.renderer, fill_col[0], fill_col[1], fill_col[2], 255);
	
	/* Draw filled rectangle? */
	if (fill) {
		SDL_RenderFillRect(cur_game->screen.renderer, &coords);
	} else {
		SDL_RenderDrawRect(cur_game->screen.renderer, &coords);
	}
	
	/* Draw border? */
	if (border && bord_col != NULL) {
		SDL_SetRenderDrawColor(cur_game->screen.renderer, bord_col[0], bord_col[1], bord_col[2], 0xFF);
		SDL_RenderDrawRect(cur_game->screen.renderer, &coords);
	}
}

void
draw_tile(struct game *cur_game, int x, int y, int w, int h, int sprite_index, unsigned char alpha)
{
	SDL_Rect rect = {x, y, w, h};
	if (alpha != 255) SDL_SetTextureAlphaMod(cur_game->sprite_textures[sprite_index], alpha);
	SDL_RenderCopy(cur_game->screen.renderer, cur_game->sprite_textures[sprite_index], NULL, &rect);
	if (alpha != 255) SDL_SetTextureAlphaMod(cur_game->sprite_textures[sprite_index], 255);
}


void
draw_all(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };

	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);	
	render_present(cur_game);
}

void
draw_game(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char loot_type;
	int rows, cols;
	short int sprite_index;
	struct win_pos win;
	short int alpha;
	SDL_bool harvestable;
	
	/* Update window position */
	win = find_win_pos(map, cur_player);
	/* Update player winpos */
	update_winpos(cur_player, win);
	/* Update screen view */
	check_if_inside(map, cur_player);
	
	/* Draw game tiles on screen */
	render_clear(cur_game);
	for (rows = win.y; rows < win.y+WIN_ROWS; rows++) {
		for (cols = win.x; cols < win.x+WIN_COLS; cols++) {
			/* Check if there's ground */
			if (*(*(map->ground+rows)+cols) != 0) {
				sprite_index = get_loot_sprite(*(*(map->ground+rows)+cols));
				alpha = 255;
			} else {
				sprite_index = get_sprite(*(*(map->tile+rows)+cols),
							  *(*(map->biome+rows)+cols));
				/* Change sprite index based on frame */
				sprite_index += *(*(map->frame+rows)+cols);
				harvestable = is_harvestable(map, cols, rows);
				if (harvestable == SDL_TRUE) {
					alpha = 255;
				} else {
					alpha = 96;
				}
			}
			/* Draw ground tile */
			draw_tile(cur_game, (cols - win.x) * SPRITE_W * WIN_SCALE + GAME_X, (rows - win.y) * SPRITE_H * WIN_SCALE + GAME_Y, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE, sprite_index, alpha); 
			/* Draw loot */
			if (*(*(map->loot+rows)+cols) != 0) {
				sprite_index = get_loot_sprite(*(*(map->loot+rows)+cols));
				loot_type = get_loot_type(*(*(map->loot+rows)+cols));
				if (*(*(map->quantity+rows)+cols) > 1 || loot_type == ITEM || loot_type == GROUND || loot_type == ROOF) {
					draw_tile(cur_game, (cols - win.x) * SPRITE_W * WIN_SCALE + GAME_X + SPRITE_W * WIN_SCALE / 4, (rows - win.y) * SPRITE_H * WIN_SCALE + GAME_Y + SPRITE_W * WIN_SCALE / 4, SPRITE_W * WIN_SCALE / 2, SPRITE_H * WIN_SCALE / 2, sprite_index, 255); 
				} else if (loot_type == WALL || loot_type == C_DOOR || loot_type == O_DOOR || loot_type == HOLDER) {
					draw_tile(cur_game, (cols - win.x) * SPRITE_W * WIN_SCALE + GAME_X, (rows - win.y) * SPRITE_H * WIN_SCALE + GAME_Y, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE, sprite_index, 255); 
				}
			}
			/* Check if there's a roof and draw it */
			if (*(*(map->roof+rows)+cols) != 0) {
				if (*(*(cur_player->screen_view+(rows - win.y))+(cols - win.x)) == 0) {
					alpha = 255;
				} else {
					alpha = 64;
				}
				sprite_index = get_loot_sprite(*(*(map->roof+rows)+cols));
				draw_tile(cur_game, (cols - win.x) * SPRITE_W * WIN_SCALE + GAME_X, (rows - win.y) * SPRITE_H * WIN_SCALE + GAME_Y, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE, sprite_index, alpha); 
			}
		}
	}
	draw_player(cur_game, cur_player, win);
	draw_inv(cur_game, cur_player);
	/* Process frames for animation */
	process_frames(map, win.y, win.x);
}

static void
draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win)
{	
	draw_tile(cur_game,
		  cur_player->x * SPRITE_W * WIN_SCALE - win.x * SPRITE_W * WIN_SCALE + GAME_X,
		  cur_player->y * SPRITE_H * WIN_SCALE - win.y * SPRITE_H * WIN_SCALE + GAME_Y,
		  SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE, 334, 255);
}

static void
update_winpos(struct player *cur_player, struct win_pos win)
{
	cur_player->winpos_x = (cur_player->x - win.x);
	cur_player->winpos_y = (cur_player->y - win.y);
}

static void
draw_inv(struct game *cur_game, struct player *cur_player)
{
	int i, j, k;
	short int sprite_index;
	char stackable;
	char darkred[3] = { 128, 0, 0 };
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	char quantity[4];

	/* Draw quick bar */
	draw_rect(cur_game, QB_X, QB_Y, QB_W + 1, QB_H, SDL_TRUE, black, SDL_TRUE, white);
	for (i = 0; i < 8; i++) {
		draw_line(cur_game,
			  QB_X + i * SPRITE_W * WIN_SCALE,
			  QB_Y,
			  QB_X + i * SPRITE_W * WIN_SCALE,
			  QB_Y + SPRITE_H * WIN_SCALE * 1.25,
			  white);
		if (cur_player->loot[i] != 0) {
			sprite_index = get_loot_sprite(cur_player->loot[i]);
			draw_tile(cur_game,
				  QB_X + i * SPRITE_W * WIN_SCALE + 1,
				  QB_Y + 2,
				  SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE,
				  sprite_index,
				  255);
			stackable = is_loot_stackable(cur_player->loot[i]);
			if (stackable == STACKABLE) {
				sprintf(quantity, "%3d", cur_player->quantity[i]);
				draw_small_sentence(cur_game,
						    QB_X + i * SPRITE_W * WIN_SCALE + 30,
						    QB_Y + SPRITE_H * WIN_SCALE + 2,
						    quantity);
			}
		}
	}
	/* Draw cursor */
	if (cur_game->cursor <= 7) {
		for (i = 1; i < 5; i++) {
			draw_rect(cur_game, QB_X + SPRITE_W * WIN_SCALE * cur_game->cursor - i, QB_Y - i, SPRITE_W * WIN_SCALE + 1 + i*2, SPRITE_H * WIN_SCALE * 1.25 + i*2, SDL_FALSE, darkred, SDL_FALSE, NULL);
		}
	}

	/* Draw inventory? */
	if (cur_game->inventory == SDL_FALSE) return;
	/* Draw inventory rectangle */
	draw_rect(cur_game, INV_X, INV_Y, INV_W, INV_H, SDL_TRUE, black, SDL_TRUE, white);
	/* Draw "Items" text box */
	draw_rect(cur_game, INV_X, INV_Y - 20, INV_W, 18 + 3, SDL_TRUE, black, SDL_TRUE, white);
	draw_small_sentence(cur_game, INV_X + 2, INV_Y - 17, "INVENTORY");
	draw_line(cur_game, INV_X + INV_W - 15, INV_Y - 15, INV_X + INV_W - 5, INV_Y - 5, white);
	draw_line(cur_game, INV_X + INV_W - 6, INV_Y - 15, INV_X + INV_W - 16, INV_Y - 5, white);
	/* Draw grid */
	for (i = 0; i < 8; i++) {
		draw_line(cur_game,
			  INV_X, INV_Y + SPRITE_H * WIN_SCALE * 1.25 * i,
			  INV_X + INV_W, INV_Y + SPRITE_H * WIN_SCALE * 1.25 * i, white);
	}
	for (i = 1; i < 4; i++) {
		draw_line(cur_game,
			  INV_X + i * SPRITE_W * WIN_SCALE, INV_Y,
			  INV_X + i * SPRITE_W * WIN_SCALE, INV_Y + INV_H, white);
	}
	
	/* Draw items */
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			if (cur_player->loot[j+i*8+8] != 0) {
				sprite_index = get_loot_sprite(cur_player->loot[j+i*8+8]);
				draw_tile(cur_game,
					  INV_X + SPRITE_W * WIN_SCALE * i + 1,
					  INV_Y + SPRITE_H * WIN_SCALE * 1.25 * j + 2,
					  SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE,
					  sprite_index,
					  255);
				stackable = is_loot_stackable(cur_player->loot[j+i*8+8]);
				if (stackable == STACKABLE) {
					sprintf(quantity, "%3d", cur_player->quantity[j+i*8+8]);
					draw_small_sentence(cur_game,
							    INV_X + i * SPRITE_W * WIN_SCALE + 30,
							    INV_Y + SPRITE_H * WIN_SCALE * 1.25 * j + 2 + SPRITE_H * WIN_SCALE,
							    quantity);
				}
			}
		}
	}
	
	
	/* Draw cursor */
	if (cur_game->cursor > 7) {
		/* Determine horizontal position */
		if (cur_game->cursor >= 8 && cur_game->cursor < 16) {
			j = INV_X;
		} else if (cur_game->cursor >= 16 && cur_game->cursor < 24) {
			j = INV_X + SPRITE_W * WIN_SCALE;
		} else if (cur_game->cursor >= 24 && cur_game->cursor < 32) {
			j = INV_X + SPRITE_W * WIN_SCALE * 2;
		} else if (cur_game->cursor >= 32 && cur_game->cursor < 40) {
			j = INV_X + SPRITE_W * WIN_SCALE * 3;
		} else {
			return;
		}
		/* Determine vertical position */
		i = INV_Y + SPRITE_H * WIN_SCALE * 1.25 * (cur_game->cursor % 8);
		/* Draw cursor */
		for (k = 1; k < 5; k++) {
			draw_rect(cur_game, j - k, i - k, SPRITE_W * WIN_SCALE + k*2 + 1, SPRITE_H * WIN_SCALE * 1.25 + k*2 + 1, SDL_FALSE, darkred, SDL_FALSE, NULL);
		}
	}
}

static void
draw_map(struct game *cur_game)
{
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	SDL_Rect rect = { MAP_X, MAP_Y, MAP_W, MAP_H };
	
	/* Draw map */
	draw_rect(cur_game, MAP_X, MAP_Y, MAP_W, MAP_H, SDL_TRUE, black, SDL_TRUE, white);
	SDL_RenderCopy(cur_game->screen.renderer, cur_game->map_texture, NULL, &rect);
	/* Draw map border */
	draw_rect(cur_game, MAP_X, MAP_Y, MAP_W, MAP_H, SDL_FALSE, white, SDL_FALSE, NULL);

}

static void
draw_player_indicator(struct game *cur_game, struct win_pos win, int size)
{
	char red[3] = { 255, 0, 0 };
	int w_size;
	int h_size;
	
	/* Draw player indicator */
	w_size = size * 1;
	h_size = size * WIN_ROWS / WIN_COLS;
	draw_rect(cur_game, MAP_X + win.x + WIN_COLS/2 - w_size/2, MAP_Y + win.y + WIN_ROWS/2 - h_size/2, w_size, h_size, SDL_FALSE, red, SDL_FALSE, NULL);
	draw_rect(cur_game, MAP_X + win.x + WIN_COLS/2 - w_size/2 - 1, MAP_Y + win.y + WIN_ROWS/2 - h_size/2 - 1, w_size+2, h_size+2, SDL_FALSE, red, SDL_FALSE, NULL);
}

static struct win_pos
find_win_pos(struct worldmap *map, struct player *cur_player)
{
	struct win_pos win;
	
	/* Find where the window position starts; x/y values = world x/y */
	win.x = cur_player->x - WIN_COLS/2;
	win.y = cur_player->y - WIN_ROWS/2;
	
	/* correct x value */
	if (win.x < 0) {
		win.x = 0;
	} else if (win.x > map->col_size - WIN_COLS) {
		win.x = map->col_size - WIN_COLS;
	}
	
	/* correct y value */
	if (win.y < 0) {
		win.y = 0;
	} else if (win.y > map->row_size - WIN_ROWS) {
		win.y = map->row_size - WIN_ROWS;
	}
	
	return win;
}

void
worldmap(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char black[3] = { 0, 0, 0 };
	char white[3] = { 255, 255, 255 };
	struct win_pos win;
	
	/* Find window position */
	win = find_win_pos(map, cur_player);

	/* Wait for user input */
	SDL_Event event;
	int size = 1;
	int size_change = 2;
	while (SDL_TRUE) {
		/* Redraw screen */
		draw_game(cur_game, map, cur_player);
		/* Draw map */
		draw_map(cur_game);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		/* Write "Map" at the top of the screen with an X */
		draw_rect(cur_game, MAP_X, MAP_Y-20, MAP_W, 18+2+1, SDL_TRUE, black, SDL_TRUE, white);
		draw_sentence(cur_game, MAP_X+1, MAP_Y-19, "WORLD MAP");
		draw_line(cur_game, MAP_X + MAP_W - 15, MAP_Y - 15, MAP_X + MAP_W - 5, MAP_Y - 5, white);
		draw_line(cur_game, MAP_X + MAP_W - 6, MAP_Y - 15, MAP_X + MAP_W - 16, MAP_Y - 5, white);
		/* Draw player indicator, render, and change size */
		draw_player_indicator(cur_game, win, size);
		render_present(cur_game);
		SDL_Delay(10);
		size += size_change;
		if (size >= WIN_COLS) {
			size_change = -2; 
		} else if (size <= 1) {
			size_change = 2;
		}
		/* Poll for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
			return;
		}
	}
}

void
setup_scanlines(struct game *cur_game)
{
	unsigned int i;
	
	/* Destroy scanlines if they already exist */
	if (cur_game->scanlines != NULL) SDL_DestroyTexture(cur_game->scanlines);
	/* Create scanlines as a texture */
	cur_game->scanlines = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
				      SDL_TEXTUREACCESS_TARGET, cur_game->screen.w, cur_game->screen.h);
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->scanlines);
	SDL_SetTextureBlendMode(cur_game->scanlines, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 128);
	for (i = 0; i < cur_game->screen.h; i += 3) {
		SDL_RenderDrawLine(cur_game->screen.renderer, 0, i, cur_game->screen.w, i);
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
}

void
toggle_scanlines(struct game *cur_game)
{
	cur_game->scanlines_on = !cur_game->scanlines_on;
}

#define TMP_NUMSPRITES 584
static void
load_sprites(struct game *cur_game)
{
	/* Load placeholder sprites */
	SDL_Surface *surface = SDL_LoadBMP("art/sprites.bmp");
	SDL_Surface **tiles, *tile;
	tiles = (SDL_Surface**) malloc(sizeof(SDL_Surface*)*TMP_NUMSPRITES);
	cur_game->sprite_textures = (SDL_Texture**) malloc(sizeof(SDL_Texture*)*TMP_NUMSPRITES);
	int i, j;
	SDL_Rect rect = {0, 0, SPRITE_W, SPRITE_H};
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 32; j++) {
			tiles[(i*32)+j] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
			SDL_SetColorKey(tiles[(i*32)+j], 1, 0x000000);
			SDL_FillRect(tiles[(i*32)+j], 0, 0x000000);
			rect.x = j * SPRITE_W;
			rect.y = i * SPRITE_H;
			SDL_BlitSurface(surface, &rect, tiles[(i*32)+j], NULL);
			cur_game->sprite_textures[(i*32)+j] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tiles[(i*32)+j]);
		}
	}
	SDL_FreeSurface(surface);
	/* Load custom sprites I designed */
	surface = SDL_LoadBMP("art/grass.bmp");
	rect.y = 0;
	for (i = 0; i < 16; i++) {
		tiles[512+i] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
		SDL_SetColorKey(tiles[512+i], 1, 0x000000);
		SDL_FillRect(tiles[512+i], 0, 0x000000);
		rect.x = i * SPRITE_W;
		SDL_BlitSurface(surface, &rect, tiles[512+i], NULL);
		cur_game->sprite_textures[512+i] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tiles[512+i]);
	}
	SDL_FreeSurface(surface);
	/* Load custom sprites I designed */
	surface = SDL_LoadBMP("art/desert.bmp");
	rect.y = 0;
	for (i = 0; i < 32; i++) {
		tiles[528+i] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
		SDL_SetColorKey(tiles[528+i], 1, 0x000000);
		SDL_FillRect(tiles[528+i], 0, 0x000000);
		rect.x = i * SPRITE_W;
		SDL_BlitSurface(surface, &rect, tiles[528+i], NULL);
		cur_game->sprite_textures[528+i] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tiles[528+i]);
	}
	SDL_FreeSurface(surface);
	/* Load custom sprites I designed */
	surface = SDL_LoadBMP("art/grass_items.bmp");
	rect.y = 0;
	for (i = 0; i < 15; i++) {
		tiles[560+i] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
		SDL_SetColorKey(tiles[560+i], 1, 0xFF00FF);
		SDL_FillRect(tiles[560+i], 0, 0xFF00FF);
		rect.x = i * SPRITE_W;
		SDL_BlitSurface(surface, &rect, tiles[560+i], NULL);
		cur_game->sprite_textures[560+i] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tiles[560+i]);
	}
	SDL_FreeSurface(surface);
	/* Load custom sprites I designed */
	surface = SDL_LoadBMP("art/items.bmp");
	rect.y = 0;
	for (i = 0; i < 9; i++) {
		tiles[575+i] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
		SDL_SetColorKey(tiles[575+i], 1, 0xFF00FF);
		SDL_FillRect(tiles[575+i], 0, 0xFF00FF);
		rect.x = i * SPRITE_W;
		SDL_BlitSurface(surface, &rect, tiles[575+i], NULL);
		cur_game->sprite_textures[575+i] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tiles[575+i]);
	}
	/* Free surface tiles from memory */
	SDL_FreeSurface(surface);
	for (i = 0; i < TMP_NUMSPRITES; i++) {
		SDL_FreeSurface(tiles[i]);
	}
	free(tiles);
	/* Load crafting buttons */
	cur_game->craft = (SDL_Texture**) malloc(sizeof(SDL_Texture*)*7);
	surface = SDL_LoadBMP("art/craft.bmp");
	rect.x = 0; rect.y = 0; rect.w = 451; rect.h = 287;
	tile = SDL_CreateRGBSurface(0, 451, 287, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[0] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(tile);
	/* No crafting buttons */
	surface = SDL_LoadBMP("art/no_craft.bmp");
	tile = SDL_CreateRGBSurface(0, 451, 287, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[1] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(tile);
	/* Load scroll buttons */
	surface = SDL_LoadBMP("art/scroll_craft.bmp");
	rect.x = 0; rect.y = 0; rect.w = 69; rect.h = 69;
	tile = SDL_CreateRGBSurface(0, 69, 69, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[2] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(tile);
	rect.x = 69;
	tile = SDL_CreateRGBSurface(0, 69, 69, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[3] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(tile);
	rect.x = 138;
	tile = SDL_CreateRGBSurface(0, 69, 69, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[4] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(tile);
	rect.x = 207;
	tile = SDL_CreateRGBSurface(0, 69, 69, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[5] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(tile);
	SDL_FreeSurface(surface);
	/* Load unknown recipe icon */
	surface = SDL_LoadBMP("art/craft_unknown.bmp");
	rect.x = 0; rect.y = 0; rect.w = 32; rect.h = 32;
	tile = SDL_CreateRGBSurface(0, 32, 32, 24, 0x00, 0x00, 0x00, 0x00);
	SDL_SetColorKey(tile, 1, 0xFF00FF);
	SDL_FillRect(tile, 0, 0xFF00FF);
	SDL_BlitSurface(surface, &rect, tile, NULL);
	cur_game->craft[6] = SDL_CreateTextureFromSurface(cur_game->screen.renderer, tile);
	SDL_FreeSurface(tile);
	SDL_FreeSurface(surface);
}

static void
unload_sprites(struct game *cur_game)
{
	int i;

	/* Free all sprites */
	for (i = 0; i < TMP_NUMSPRITES; i++) {
		SDL_DestroyTexture(cur_game->sprite_textures[i]);
	}
	free(cur_game->sprite_textures);
	/* Free craft textures */
	for (i = 0; i < 7; i++) {
		SDL_DestroyTexture(cur_game->craft[i]);
	}
	free(cur_game->craft);
}

int LAST = 0;
void
loading_bar(struct game *cur_game, char *title, int percentage)
{
	char black[3] = { 0, 0, 0 };
	char white[3] = { 255, 255, 255 };
	char blue[3] = { 0, 0, 255 };
	
	/* Should I even draw the loading bar? */
	if (percentage == LAST) return;
	/* Yes */
	LAST = percentage;
	
	/* Clear screen */
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
	/* Write title to screen */
	draw_sentence(cur_game, 0, 0, title);
	/* Draw percentage bar */
	draw_rect(cur_game, 20, 30, 1000, 40, SDL_TRUE, black, SDL_TRUE, white);
	draw_rect(cur_game, 21, 31, percentage*10-2, 38, SDL_TRUE, blue, SDL_FALSE, NULL);
	render_present(cur_game);
}

static void
map_init(struct game *cur_game)
{
	cur_game->map_texture = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
						  SDL_TEXTUREACCESS_TARGET, MAP_COLS, MAP_ROWS);
}

static void
map_destroy(struct game *cur_game)
{
	SDL_DestroyTexture(cur_game->map_texture);
}
