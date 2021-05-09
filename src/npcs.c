#include <SDL2/SDL.h>
#include "main.h"
#include "maps.h"
#include "npcs.h"
#include "play.h"
#include "rand.h"

/* Function prototype */
// none yet

/* Linked list of NPCs */
struct npc *NPCS = NULL;

void
setup_npcs(void)
{
	struct npc *first;
	first = malloc(sizeof(*first));
	first->next = NULL;
	NPCS = first;
}

void
populate_npcs(struct worldmap *map)
{
	char personality;
	int x, y;
	int i;
	int del_x, del_y;
	
	/* Make 6 random NPCs */
	for (i = 0; i < 6; i++) {
		x = rand_num(0, map->col_size - 1);
		y = rand_num(0, map->row_size - 1);
		personality = (char) rand_num(NEUTRAL, FRIENDLY);
		add_npc(x, y, personality, NULL, NULL);
		if (i == 2) {
			del_x = x;
			del_y = y;
		}
		printf("Created npc at (%d, %d) with personality %d\n", x, y, personality);
	}
	/* Remove one of the NPCs */
	del_npc(del_x, del_y);
}

void
add_npc(int x, int y, char personality, unsigned short int *loot, unsigned short int *quantity)
{
	int i;
	struct npc *new;
	struct npc *tmp;

	/* Find the last entry in the linked list */
	tmp = NPCS;
	while (tmp->next != NULL) {
		tmp = tmp->next;
	}
	
	/* Make a new NPC and add it to the list */
	new = malloc(sizeof(*new));
	new->x = x;
	new->y = y;
	new->personality = personality;
	if (loot == NULL) {
		for (i = 0; i < 40; i++) {
			new->loot[i] = 0;
			new->quantity[i] = 0;
		}
	} else {
		/* Populate holder with values given */
		for (i = 0; i < 40; i++) {
			new->loot[i] = loot[i];
			new->quantity[i] = quantity[i];
		}
	}
	new->next = NULL;
	tmp->next = new;
}

void
del_npc(int x, int y)
{
	struct npc *tmp;
	struct npc *last;
	
	/* Run through the holder table and find the one we need to kill */
	tmp = NPCS;
	while (tmp->next != NULL) {
		last = tmp;
		tmp = tmp->next;
		if (tmp->x == x && tmp->y == y) {
			/* Remove from linked list */
			printf("Killing NPC at (%d, %d) with personality %d!\n", tmp->x, tmp->y, tmp->personality);
			last->next = tmp->next;
			free(tmp);
			tmp = last;
		}
	}
}

void
kill_npcs(void)
{
	struct npc *next;
	struct npc *curr;
	
	/* If the NPC table was never set up, get up on out of here */
	if (NPCS == NULL) return;
	/* If there are no NPCs, free the table and leave */
	if (NPCS->next == NULL) {
		free(NPCS);
		return;
	}
	/* There are NPCs, so loop through them and kill them */
	curr = NPCS->next;
	free(NPCS);
	while (curr->next != NULL) {
		next = curr->next;
		printf("Killing NPC at (%d, %d) with personality %d\n", curr->x, curr->y, curr->personality);
		free(curr);
		curr = next;
	}
	printf("Killing NPC at (%d, %d) with personality %d\n", curr->x, curr->y, curr->personality);
	free(curr);
}

void
dump_npcs(FILE *fp)
{
	int i;
	struct npc *dump;
	
	/* Go through every entry in the holders table and output values to the file */
	dump = NPCS;
	while (SDL_TRUE) {
		if (dump->next == NULL) {
			return;
		} else {
			dump = dump->next;
			fprintf(fp, "%d %d %d\n", dump->x, dump->y, dump->personality);
			for (i = 0; i < MAX_INV; i++) {
				fprintf(fp, "%hu %hu\n", dump->loot[i], dump->quantity[i]); 
			}
		}
	}
}

void
undump_npcs(FILE *fp)
{
	int i;
	int x, y, personality;
	unsigned short int loot[40], quantity[40];

	/* Set up the holders table */
	setup_npcs();
	/* Read line from file */
	while (fscanf(fp, "%d %d %d\n", &x, &y, &personality) == 3) {
		/* Populate the loot and quantity values */
		for (i = 0; i < 40; i++) {
			fscanf(fp, "%hu %hu\n", &loot[i], &quantity[i]);
		}
		/* Add the npc */
		add_npc(x, y, (char) personality, loot, quantity);
	}
}
