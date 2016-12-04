#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "reaction.h"

pthread_mutex_t pc_mutex;
pthread_cond_t cond_avail_hydrogen, cond_done_reaction;

// Forward declaration. This function is implemented in reaction-runner.c
void make_water();


void reaction_init(struct reaction *reaction){

    reaction->num_h_atoms = 0;
}


void reaction_h(struct reaction *reaction){

    pthread_mutex_lock(&pc_mutex);    //enter critical section  
    
    reaction->num_h_atoms++;
    pthread_cond_signal(&cond_avail_hydrogen);		// signal new h atome
    pthread_cond_wait(&cond_done_reaction, &pc_mutex);  // wait reaction
    
    pthread_mutex_unlock(&pc_mutex);  // leave critical section
}


void reaction_o(struct reaction *reaction){

    pthread_mutex_lock(&pc_mutex);     //enter critical section 

    while(reaction->num_h_atoms < 2){

	pthread_cond_wait(&cond_avail_hydrogen, &pc_mutex);  // wait to have 2h
    }
	
    make_water();
    reaction->num_h_atoms -= 2;   
    pthread_cond_signal(&cond_done_reaction);	// signal the 2h as the reaction already done
    pthread_cond_signal(&cond_done_reaction);

    pthread_mutex_unlock(&pc_mutex);    // leave critical section
}
