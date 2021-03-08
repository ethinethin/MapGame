#ifndef MAPS_H
#define MAPS_H

#include <SDL2/SDL.h>

/* Defines for map properties */
#define IMPASSABLE 0
#define PASSABLE 1
#define FRAME_SPEED 30

/* Structure for map */
struct worldmap {
	int row_size;
	int col_size;
	int **tile;
	short int **biome;
	unsigned short int **loot;
	unsigned short int **quantity;
	unsigned short int **ground;
	unsigned short int **roof;
	char **frame;
	unsigned short int **frame_count;
	unsigned short int frame_speed;
	SDL_bool **harvestable;
};

extern void		 create_map(struct worldmap *map, int row_size, int col_size);
extern void		 free_map(struct worldmap *map);
extern void		 populate_map(struct worldmap *map, int start_tile, short int biome);
extern short int	 get_sprite(int tile, short int biome);
extern char		*get_color(int tile, short int biome);
extern char		 is_passable(int tile, short int biome);
extern char 		*get_tile_name(int tile, short int biome);
extern void		 process_frames(struct worldmap *map, int row, int col);

#endif

