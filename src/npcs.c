#include "main.h"
#include "maps.h"
#include "npcs.h"
#include "play.h"
#include "rand.h"

/* Function prototype */
static void	populate_npcs(struct worldmap *map);

/* Linked list of NPCs */
struct npc *NPCS = NULL;

void
setup_npcs(struct worldmap *map)
{
	struct npc *first;
	first = malloc(sizeof(*first));
	first->next = NULL;
	NPCS = first;
	populate_npcs(map);
}

static void
populate_npcs(struct worldmap *map)
{
	char personality;
	int x, y;
	int i;
	
	/* Make 5 random NPCs */
	for (i = 0; i < 5; i++) {
		x = rand_num(0, map->col_size - 1);
		y = rand_num(0, map->row_size - 1);
		personality = (char) rand_num(NEUTRAL, FRIENDLY);
		add_npc(x, y, personality);
		printf("Created npc at (%d, %d) with personality %d\n", x, y, personality);
	}
	
}

void
add_npc(int x, int y, char personality)
{
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
	new->next = NULL;
	tmp->next = new;
}

void
del_npc(void)
{

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
