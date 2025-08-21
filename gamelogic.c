#include "gamelogic.h"
#include "user.h"

//Return 1 for hit, 0 for miss
int check_collision(player*player, int x, int y) { 
    //get the player 
    if(player==NULL) return 0; 
    for(int i = 0; i < 5; i++) { 
        if(player->coordinates[i].x == x && player->coordinates[i].y == y) {
            // if(player->damaged[i] == 0) { 
                player->damaged[i] = 1; //set to damaged
                return 1; //b/c max one hit 
            // }
        }
    }
    return 0; 
}

//1 if dead, 0 if not, -1 if error
int checkDead(player*player) { 
    if(player==NULL)return -1; //error  
    for(int i = 0; i < 5; i++) { 
        if(player->damaged[i] == 0) return 0; //if there is a non occupied cell (i.e. non death) return early
    }
    return 1; 
}