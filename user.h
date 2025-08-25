#ifndef _USER_H
#define _USER_H

//Stores coordinate
typedef struct coord { 
    int x;
    int y;
} coord; 

typedef struct player { 
    char name[21];
    coord coordinates[5];
    int damaged[5]; //1 if damaged, 0 if not 
    int socket_fd; //to talk to client 

    int isConnected; //0 if not connected, 1 if connected 
    int isRegistered; //0 if not registered, 1 if registered 
    char buf[110];  //message buffer (due to out of stream boundaries)
    int blen;       //length of current buffer

    struct player*next; 
} player; 

//Return value: -1 if invalid, 0 if valid
int validatePlayerCreation(player*head, int x, int y, char name[21], char isVert);
int validatePlayerCreationRand(player*head, char name[21], int* x, int* y, char* direction);

//insert coordinates 
void insertCoord(player*player, int x, int y, char isVert);

//Create player - this does not mean registered 
player* createPlayer(int socket_fd);
//Register player - confirm information 
void registerPlayer(player*newPlayer, char name[21], int x, int y, char isVert);
//add player to L.L 
player* addPlayer(player*head, player*newPlayer);
//remove all dead/disconnected players 
player* removeAllPlayers(player*head);
#endif