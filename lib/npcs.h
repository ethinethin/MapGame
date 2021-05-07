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
extern void	setup_npcs(struct worldmap *map);
extern void	add_npc(int x, int y, char personality);
extern void	del_npc(void);
extern void	kill_npcs(void);

#endif
