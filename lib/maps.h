#ifndef MAPS_H
#define MAPS_H

/* Structure for map */
struct worldmap {
	int row_size;
	int col_size;
	int **tile;
};

extern void		 create_map(struct worldmap *map, int row_size, int col_size);
extern void		 free_map(struct worldmap *map);
extern void		 populate_map(struct worldmap *map, int start_tile);
extern short int	 get_sprite(int tile);
extern char		*get_color(int tile);

#endif

