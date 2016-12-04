#include <stdio.h>
#include <pthread.h>


struct reaction {
	
    int num_h_atoms;
};

void reaction_init(struct reaction *reaction);

void reaction_h(struct reaction *reaction);

void reaction_o(struct reaction *reaction);
