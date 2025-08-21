#include "client.h"
#include <stdio.h>
#include <stdlib.h>
//Create player 
client* makeClient(int socket_fd) { 
     //dynamically allocate for struct 
    client* newClient = (client*)calloc(1, sizeof(client)); 
    //if player allocation succeeds, proceed 
    if(newClient == NULL) return NULL; //error 
    //instantiate values 
    newClient->socket_fd=socket_fd; 
    newClient->next = NULL; 
    newClient->blen=0;
    //initalize damaged to be all zeros for now (no damage)
    return newClient;
}
//remove player from L.L 
client* removeClient(client* head, int socket_fd) { 
    //iterate through entire L.L, checking the coordinates for each player, and their 'damaged'
    if(head==NULL) return NULL; 
    //set temp to head, iterate through list 
    //set a dummy pointer for LL deletion 
    client dummy; 
    client*prev = &dummy; 
    client*curr = head; 
    client*temp = head; 
    dummy.next=head; 
    while(curr!=NULL) { 
        //iterate through coordinates 
        if(curr->socket_fd == socket_fd) {
            //remove from L.L 
            prev->next=curr->next; //previous points to current next 
            //close connection 
            free(curr);
            break; 
        }
        //increment curr, prev
        prev = prev->next;
        curr = curr->next;  //now increment current to be next
    }
    return dummy.next; 
}
//add player to L.L 
client* addClient(client*head, client*newClient) {  
    if(head==NULL || newClient == NULL)return newClient;
    //else append to head 
    newClient->next=head;
    return newClient; 
}
//get player info 
client* getClient(client*head, int socket_fd) { 
    if(head!=NULL) { 
        while(head!=NULL) { 
            if(socket_fd==head->socket_fd) return head; 
            head=head->next; 
        }
    }
    return NULL; 
}
