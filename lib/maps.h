#ifndef MAPS_H
#define MAPS_H

/* Defines for map properties */
#define IMPASSABLE 0
#define PASSABLE 1

/* Structure for map */
struct worldmap {
	int row_size;
	int col_size;
	int **tile;
	short int **biome;
};

extern void		 create_map(struct worldmap *map, int row_size, int col_size);
extern void		 free_map(struct worldmap *map);
extern void		 populate_map(struct worldmap *map, int start_tile, short int biome);
extern short int	 get_sprite(int tile, short int biome);
extern char		*get_color(int tile, short int biome);
extern char		 is_passable(int tile, short int biome);

#endif

