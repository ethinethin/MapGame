#include <stdlib.h>
#include <sys/time.h>

void
seed_rng(void)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	srand(time.tv_sec + time.tv_usec);
}

int
rand_num(int min, int max)
{
	return rand() % (1+max-min) + min;
}
