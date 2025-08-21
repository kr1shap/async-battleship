#ifndef _GAMELOGIC_H
#define _GAMELOGIC_H
#include "user.h"
#include <stdio.h>

//function checks for collisions with given hit 'x' and 'y' value
int check_collision(player*player, int x, int y);
//checks if anyone is dead, and marks them as 'unregistered' as a result
int checkDead(player*head);

#endif
