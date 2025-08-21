#ifndef _CLIENT_H
#define _CLIENT_H

typedef struct client { 
    int socket_fd; //to talk to client 
    int isConnected; //1 if connected, 0 if not
    char buf[101];
    int blen; 
    struct client*next; 
} client; 


//Create player 
client* makeClient(int socket_fd);
//remove player from L.L 
client* removeClient(client*head, int socket_fd);
//add player to L.L 
client* addClient(client*head, client*newClient);
//get player info 
client* getClient(client*head, int socket_fd);

#endif