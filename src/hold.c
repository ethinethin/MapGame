#include <SDL2/SDL.h>
#include "hold.h"
#include "main.h"
#include "maps.h"
#include "play.h"

struct holder {
	int x;
	int y;
	unsigned short int loot[40];
	unsigned short int quantity[40];
	struct holder *next;
} *HOLDERS;

void
setup_hold(void)
{
	struct holder *first;
	
	/* Make first (null) entry in holder table */
	first = malloc(sizeof(*first)*1);
	first->next = NULL;
	HOLDERS = first;
}

void
add_hold(int x, int y)
{
	int i;
	struct holder *new;
	struct holder *tmp;
	
	/* Create and populate a new holder */
	new = malloc(sizeof(*new)*1);
	new->x = x;
	new->y = y;
	for (i = 0; i < 40; i++) {
		new->loot[i] = 0;
		new->quantity[i] = 0;
	}
	new->next = NULL;
	
	/* Add it to the holder table */
	tmp = HOLDERS;
	while (SDL_TRUE) {
		if (tmp->next == NULL) {
			tmp->next = new;
			return;
		} else {
			tmp = tmp->next;
		}
	}
}

void
del_hold(int x, int y)
{
	struct holder *tmp;
	struct holder *last;
	
	/* Run through the holder table and find the one we need to kill */
	tmp = HOLDERS;
	while (tmp->next != NULL) {
		last = tmp;
		tmp = tmp->next;
		if (tmp->x == x && tmp->y == y) {
			/* Remove from linked list */
			last->next = tmp->next;
			free(tmp);
			tmp = last;
		}
	}
}

void
kill_hold(void)
{
	struct holder *tmp;
	struct holder *next;
	
	/* Delete all entries from the holder table */
	next = HOLDERS;
	while (SDL_TRUE) {
		if (next->next == NULL) {
			free(next);
			return;
		} else {
			tmp = next;
			next = next->next;
			free(tmp);
		}
	}		
}

void
open_chest(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{

}

SDL_bool
chest_empty(struct worldmap *map, int x, int y)
{
	struct holder *tmp;
	int i;
	int num;
	
	/* Find holder from list */
	tmp = HOLDERS;
	while (SDL_TRUE) {
		tmp = tmp->next;
		if (tmp->x == x && tmp->y == y) break;
	}
	
	/* Check each item spot in holder */
	for (i = 0, num = 0; i < 40; i++) {
		if (tmp->loot[i] != 0) num++;
	}
	if (num == 0) {
		del_hold(x, y);
		return SDL_TRUE;
	} else {
		return SDL_FALSE;
	}
}
