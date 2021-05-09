#ifndef NPCS_H
#define NPCS_H

#include "play.h"

/* Main NPC structure */
struct npc {
	int x;
	int y;
	unsigned short int loot[MAX_INV];
	unsigned short int quantity[MAX_INV];
	char personality;
	struct npc *next;
};

/* Personalities */
#define NEUTRAL 0
#define HOSTILE 1
#define FRIENDLY 2

/* Function prototypes */
extern void	setup_npcs(void);
extern void	populate_npcs(struct worldmap *map);
extern void	add_npc(int x, int y, char personality, unsigned short int *loot, unsigned short int *quantity);
extern void	del_npc(int x, int y);
extern void	kill_npcs(void);
extern void	dump_npcs(FILE *fp);
extern void	undump_npcs(FILE *fp);

#endif
