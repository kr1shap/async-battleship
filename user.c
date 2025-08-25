#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

//get player name

//Validate if we can create the player ship 
//Return value: -1 if invalid, 0 if name taken, 1 if valid
int validatePlayerCreation(player*head, int x, int y, char name[21], char isVert) { 
    srand(time(NULL));
    //first see if name is taken (traverse through LL)
    player*temp = head; 
    while(temp!=NULL) { 
        if(strcmp(temp->name, name) == 0 && temp->isConnected == 1 && temp->isRegistered == 1) return 0; //name taken 
        temp = temp->next; 
    }
    //next, check coordinate validity based on x and y value 
    if(x < 0 || x > 9 || y < 0 || y > 9) return -1; 
    //if valid, now check if ship can be positioned 
    if(isVert=='-') { 
        if(x-2 < 0 || x+2 > 9) return -1; 
        return 1;
    }
    else if(isVert=='|') { 
        if(y-2 < 0 || y+2 > 9) return -1; 
        return 1; 
    }
    else return -1; //invalid number given 
}


//Validate if we can create the player ship 
//Return value: -1 if invalid, 0 if name taken, 1 if valid
int validatePlayerCreationRand(player*head, char name[21], int* x, int* y, char* direction) { 
    srand(time(NULL));
    //first see if name is taken (traverse through LL)
    player*temp = head; 
    while(temp!=NULL) { 
        if(strcmp(temp->name, name) == 0 && temp->isConnected == 1 && temp->isRegistered == 1) return 0; //name taken 
        temp = temp->next; 
    }
    //Until valid 
    while(1) { 
        //do automatic placement
        *x = (rand() % (9 - 0 + 1)) + 0;
        *y = (rand() % (9 - 0 + 1)) + 0;
        int val = (rand() % (1 - 0 + 1)) + 0;
        if(!val) *direction='-';
        else *direction='|';
        if(*direction=='-') if(*x-2 < 0 || *x+2 > 9) continue;
        if(*y-2 < 0 || *y+2 > 9) continue; 
        break;   
    }
    return 1; //valid automatically 

}

//insert coordinates 
void insertCoord(player*player, int x, int y, char isVert) { 
    if(player==NULL) return; 
    //else modify and check if vertical or not 
    if(isVert == '-') {  //horizontal 
        for(int i = 0; i < 5; i++) {
            player->coordinates[i].x = x-(2-i);
            player->coordinates[i].y = y; 
        }
    }
    else if(isVert=='|') { 
        for(int i = 0; i < 5; i++) {
            player->coordinates[i].y = y-(2-i);
            player->coordinates[i].x = x; 
        }
    }
    return; 
}

//Function creates new player in LL
player* createPlayer(int socket_fd) {
    //dynamically allocate for struct 
    player* newPlayer = (player*)calloc(1, sizeof(player)); 
    //if player allocation succeeds, proceed 
    if(newPlayer == NULL) return NULL; //error 
    //instantiate values 
    newPlayer->next = NULL; 
    newPlayer->socket_fd=socket_fd; 
    newPlayer->isConnected=1; //connected player 
    newPlayer->isRegistered = 0;  //not registered yet 
    newPlayer->blen = 0; 
    newPlayer->buf[0] = '\0';
    return newPlayer;
}

//Function registers player (i.e. set registered, name values...)
void registerPlayer(player*newPlayer, char name[21], int x, int y, char isVert) { 
    strcpy(newPlayer->name, name);
    //initalize damaged to be all zeros for now (no damage)
    for(int i = 0; i < 5; i++) newPlayer->damaged[i] = 0; 
    //compute coordinates (we assume coordinates are valid, so no need to worry)
    insertCoord(newPlayer, x, y, isVert);
    newPlayer->isConnected=1; //connected player (precautionary approach)
    newPlayer->isRegistered = 1; //offically registered  

}

//add player to L.L
player* addPlayer(player*head, player*newPlayer) { 
    if(head==NULL)return newPlayer;
    if(newPlayer==NULL) return head; 
    //else append to head 
    newPlayer->next=head;
    return newPlayer; 
}

//Removes all dead/disconnected players 
player* removeAllPlayers(player*head) { 
    //iterate through entire L.L, checking the coordinates for each player, and their 'damaged'
    if(head==NULL) return NULL; 
    //set temp to head, iterate through list 
    //set a dummy pointer for LL deletion 
    player dummy; 
    player*prev = &dummy; 
    player*curr = head; 
    dummy.next=head; 
    while (curr != NULL) {
    player *next = curr->next; // store before modification, as we have to re-link our LL if needed
        if (curr->isConnected==0) { //if not connected, we must remove 
            close(curr->socket_fd); //close their connection
            if (prev == NULL) { //if head
                head = next;
            } else { //if not head
                prev->next = next;
            }
        } else { //else iterate
            prev = curr;
        }
        curr = next;
    }
    return dummy.next; 
}
