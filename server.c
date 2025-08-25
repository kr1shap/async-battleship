#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "gamelogic.h"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLDRED "\033[1;31m"
#define RESET   "\033[0m"
#define BOLD    "\033[1m"

//Global var
int clientLength=0;

//Function sets nonblocking mode on socket_fd
void setNonBlocking(int socket_fd) { 
    //Set to nonblocking 
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

//Function checks if we can write, note that I have ignored SIGPIPE so it will return -1 if writing end closed instead of terminating
void checkWrite(char msg[], int len, player*player) { 
    int res = write(player->socket_fd, msg, len);
    if(res == -1) { 
        player->isConnected = 0; // premature disconnect
    }
}


//Function sets sighandler for SIGPIPE to be ignored
void ignore_sigpipe(void){
  struct sigaction myaction;
  myaction.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &myaction, NULL);
}

//Function broadcasts a 'hit' or 'miss' message depending on 'int isHitorMiss' -> 1 = hit 0 = miss
void broadcast_hit_miss(player*head, player*victim, player*attacker, int isHitOrMiss, int x, int y) { 
    if(head==NULL) return;  //somehow if I pass an empty list
    player*temp=head; 
    while(temp!=NULL) { 
        if(temp->isConnected==1 && temp->isRegistered==1) { //only broadcast given they are registered AND connected
            char buffer[128]; //buffer to write message 
            int len; 
            if(isHitOrMiss) { //hit msg
                // len = snprintf(buffer, sizeof(buffer), BOLDRED "HIT %s %d %d %s\n" RESET, attacker->name, x, y, victim->name);
                len = snprintf(buffer, sizeof(buffer),"HIT " YELLOW "%s" RESET" " MAGENTA "%d" RESET" " MAGENTA "%d" RESET" " YELLOW "%s" RESET "\n",attacker->name, x, y, victim->name);
            }
            else { //if miss
                len = snprintf(buffer, sizeof(buffer),"MISS " YELLOW "%s" RESET" " MAGENTA "%d" RESET" " MAGENTA "%d" RESET "\n",attacker->name, x, y);
            }
            if (len > 0) { //if snprintf succeeds, call checkwrite to write and handle errors if needed (i.e. disconnection)
                checkWrite(buffer, len, temp);
            }
        }
        temp = temp->next; 
    }
}

//Function broadcasts a 'gg' or 'join' message to entire server depending on 'isGG'
void broadcast_gg_join(player*head, player*target, int isGG) { 
    if(head==NULL)return; 
    player*temp=head; 
    while(temp!=NULL) { 
        char buffer[128];
        int len; 
        if(temp->isConnected==1 && temp->isRegistered==1) { //only broadcast given they are registered AND connected
            if(isGG) { //gg msg
                len = snprintf(buffer, sizeof(buffer), RED "GG %s\n" RESET, target->name);
            }
            else { //if join
                len = snprintf(buffer, sizeof(buffer), GREEN "JOIN %s\n" RESET, target->name);
            }
            if (len > 0) { //if snprintf succeeds, call checkwrite to write and handle errors if needed (i.e. disconnection)
                checkWrite(buffer, len, temp);
            }
        }
        temp = temp->next; 
    }
}

//Function attempts to read any input from user, storing the final result (given client protocol is to end with \n) in 'temp'
//Return values: -1 if error/failure, 0 if nothing (i.e. skip), 1 if success
int read_from_user(player *c, char temp[110]) {
    int n;
    while (1) {
        if (c->blen > 100) { //If blen is greater than 100, disconnect
            return -1;
        }
        //Read at most 100-c->blen char (this includes the newline!!!!)
        n = read(c->socket_fd, c->buf + c->blen, 100 - c->blen);
        if (n < 0) { //if n < 0 we get an error
            // no data to read, so don't worry about it and try again
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return 0; //no issue
            }
            return -1;
        }
        // EOF (client disconnected)
        if (n == 0) return -1;

        c->blen += n; //Add to length 
        //If you found a new line, we break and just stop reading 
        if (memchr(c->buf, '\n', c->blen) != NULL) {
            break;
        }
    }

    // find the newline location 
    char *newline = memchr(c->buf, '\n', c->blen);
    if (newline!=NULL) { //if we found newline
        int len = newline - c->buf + 1;  //include newline
        if (len >= 110) len = 109;      //precautionary approach (even though this should never occur!!!!)

        memcpy(temp, c->buf, len); //copy the string into temp (i.e. until buffer length as that is our msg)
        temp[len] = '\0';

        //shift to front (given that we can)
        int remaining = c->blen - len;
        if (remaining > 0) {
            memmove(c->buf, c->buf + len, remaining);
        }
        c->blen = remaining;
        return 1;
    }

    return 0; //nothing somehow
}

//Function decodes an unregistered message
void decode_message_unregistered(player*c, player* phead, char msg[110]) { 
    //check if first three char is 'REG' 
    int x, y; 
    char name[21];
    char direction; 
    if(strncmp("REG", msg, 3) == 0) {  //The first 3 char is 'REG'
        if (sscanf(msg, "REG %20s %d %d %c\n", name, &x, &y, &direction) == 4) { 
            //we can create the player - see if we can create it with given coordinates
            int validate = validatePlayerCreation(phead, x, y, name, direction); 
            //If we can create the player, register them!!
            if(validate == 1) { 
                //register player (this sets registered status)
                registerPlayer(c, name, x, y, direction); 
                checkWrite(GREEN BOLD "WELCOME\n" RESET, strlen(GREEN BOLD "WELCOME\n" RESET), c);
                broadcast_gg_join(phead, c, 0); //Broadcast join message
                return; //return early 
            }
            //name taken 
            else if(validate == 0) { 
                checkWrite(RED BOLD "TAKEN\n" RESET, strlen(RED BOLD "TAKEN\n" RESET), c); //Attempt to write 'TAKEN' to the user
                return; 
            }
        }
        //Sending no coordinates means auto coords!
        else if(sscanf(msg, "REG %20s\n", name) == 1) { 
            int validate = validatePlayerCreationRand(phead, name, &x, &y, &direction); 
            //If we can create the player, register them!!
            if(validate == 1) { 
                //register player (this sets registered status)
                registerPlayer(c, name, x, y, direction); 
                checkWrite(GREEN BOLD "WELCOME\n" RESET, strlen(GREEN BOLD "WELCOME\n" RESET), c);
                char buf[1024];
                int len = snprintf(buf, sizeof(buf), "YOUR COORDS, x: " MAGENTA "%d" RESET " y: " MAGENTA "%d" RESET " direction: " BLUE "%c" RESET "\n", x, y, direction);
                if(len > 0) checkWrite(buf, len, c); //Attempt to write the given coords to the user
                broadcast_gg_join(phead, c, 0); //Broadcast join message
                return; //return early 
            }
            //name taken 
            else if(validate == 0) { 
                checkWrite(RED BOLD "TAKEN\n" RESET, strlen(RED BOLD "TAKEN\n" RESET), c); //Attempt to write 'TAKEN' to the user
                return; 
            }
        }

    }
    checkWrite(RED BOLD "INVALID\n" RESET, strlen(RED BOLD "INVALID\n" RESET), c);

}

//Function prints out entire LL of usernames to given client 
//Accumulates in dynamic buffer, then all sent to client
void printNames(player* phead, player* c) {
    int bufSize = 128; //rough inital estimate
    char *buf = malloc(bufSize);
    if(!buf) return;
    buf[0] = '\0';
    strcat(buf, CYAN BOLD ". ݁₊ ⊹ . PLAYER LIST . ⊹ ₊ ݁.\n" RESET);
    player* temp = phead;
    while(temp) {
        if(temp->isConnected && temp->isRegistered) { 
            int needed = strlen(buf) + strlen(temp->name) + 2; //newline and terminating char 
            if(needed > bufSize) {
                bufSize *= 2;
                buf = realloc(buf, bufSize);
                if(!buf) return;
            }
            strcat(buf, temp->name);
            strcat(buf, "\n");
        }
        temp = temp->next;
    }

    checkWrite(buf, strlen(buf), c);
    free(buf);
}

//Function decodes a registered message
void decode_message_registered(player*c, player* phead, char msg[110]) { 
    int x, y; 
    int hit = 0; //stores whether someone has been hit or not 
    player* temp = phead; 
    char message[61];
    char name[21];
    if(strncmp("BOMB", msg, 4) == 0) {  //The first 4 char is 'BOMB'
        if (sscanf(msg, "BOMB %d %d\n", &x, &y) == 2) { 
            while(temp!=NULL) { //iterate through entire list 
                if(temp->isConnected==1 && temp->isRegistered==1) {  //only check collision given they are registered AND connected
                    if(check_collision(temp, x, y) == 1) { //check if the collision is 1 (we hit them)
                        broadcast_hit_miss(phead, temp, c, 1, x, y); //if so, broadcast hit message
                        hit = 1; //set hit to one
                    }
                }
                temp = temp->next; 
            }
            if(hit == 0) { //if no one was hit, set miss message
                broadcast_hit_miss(phead, temp, c, 0, x, y);
            }
            return; 
        }
    }
    else if(strncmp("SEND", msg, 4) == 0) { //client wants to send a message to a user!
        if (sscanf(msg, "SEND %20s %60[^\n]", name, message) == 2) { 
            //iterate through LL and see if client name exists, if not; send invalid
            if(strcmp(name, c->name) == 0)  { 
                checkWrite(CYAN BOLD "YOU CAN'T SEND YOURSELF THAT!\n" RESET, strlen(CYAN BOLD "YOU CAN'T SEND YOURSELF THAT!\n" RESET), c); 
                return; 
            }
            while(temp!=NULL) { //iterate through entire list 
                if(temp!= c && temp->isConnected==1 && temp->isRegistered==1 && strcmp(name, temp->name)==0) {  //must be registered and connected user to send to!
                    char buf[1024];
                    int len = snprintf(buf, sizeof(buf), "SENT FROM " YELLOW "%s" RESET ": %s\n", c->name, message);
                    if(len>0) checkWrite(buf, len, temp);
                    return;
                }
                temp = temp->next; 
            }
            checkWrite(RED BOLD "USER DOES NOT EXIST!\n" RESET, strlen(RED BOLD "USER DOES NOT EXIST!\n" RESET), c);
            return;
        }
    }
    else if(strcmp("LIST\n", msg) == 0) { 
        printNames(phead, c);
        return;
    }
    checkWrite(RED BOLD "INVALID\n" RESET, strlen(RED BOLD "INVALID\n" RESET), c);
    return; 
}

//Function checks for any deaths, this marks them as disconnected
void check_deaths(player*phead) { 
    //traverse through players, marking any deaths as 'disconnected'
    player*temp = phead; 
    while(temp!=NULL) { 
        if(temp->isConnected==1 && temp->isRegistered==1 && checkDead(temp) == 1) { //Check if they are dead (only runs given registered and connected)
            broadcast_gg_join(phead, temp, 1); //broadcast the GG message to everyone 
            //Then set to not connected/registered
            temp->isConnected = 0; 
            temp->isRegistered = 0; 
            clientLength--;
        }
        else if(temp->isConnected==0 && temp->isRegistered==1) { //special case: not connected user but is registered, we should broadcast gg instance
            broadcast_gg_join(phead, temp, 1); 
        }
        temp = temp->next; 
    }
}


int main(int argc, char const* argv[]) {
    if(argc == 1) return 1; //not enough arguments 

    //Instantiate variables 
    player* playerHead = NULL; //LL of all clients
    int sfd, maxfd;
    struct sockaddr_in serverAdd;
    int opt = 1;
    ignore_sigpipe(); //ignore SIGPIPE
    //Create instances of variables 
    fd_set readfds;

    memset(&serverAdd, 0, sizeof(struct sockaddr_in)); //best practice due to potential padding 
    if((sfd=socket(AF_INET, SOCK_STREAM, 0))==-1) return 1; //failure to make socket
    maxfd = sfd; //set maxfd to sfd for now...
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //fill in address struct 
    serverAdd.sin_family = AF_INET; //ipv4 
    int port = atoi(argv[1]); //convert first 
    if(port < 0) return 1; //invalid number (i.e. 0) 
    if(port==0 && strcmp(argv[1], "0") != 0) return 1; //error
    //If it returns 0, htons knows to assign any port, so
    serverAdd.sin_port = htons(atoi(argv[1])); //convert arg to int -> then to readable port address
    serverAdd.sin_addr.s_addr = htonl(INADDR_ANY); //any address
    //call bind 
    if (-1 == bind(sfd, (struct sockaddr *)&serverAdd, sizeof(struct sockaddr_in))) {
        perror("binding error");
        return 1;
    }
    //call listen 
    if (-1 == listen(sfd, 30)) { //keep queue to ~30 players? 
        perror("listening error");
        return 1;
    }
    
    for(;;) { 
        //Instantiate the fd set to zero, and add sfd to set 
        FD_ZERO(&readfds);
        FD_SET(sfd, &readfds);
        maxfd = sfd; //temporarily set maxfd to sfd 
        //Iterate through client L.L, adding the sfd to listen to 
        player*temp = playerHead; 
        while(temp!=NULL) { 
            FD_SET(temp->socket_fd, &readfds);
            if(temp->socket_fd > maxfd) maxfd = temp->socket_fd; 
            temp = temp->next; 
        }
        //Call select
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            // perror("error with calling select");
            continue;
        }
        
        //Check for any new/ connections 
        if (FD_ISSET(sfd, &readfds)) {
            int cfd; //stores client fd 
            cfd = accept(sfd, NULL, NULL); //accept incoming client fd 
            if(cfd!= -1) { 
                //add to player list 
                playerHead = addPlayer(playerHead, createPlayer(cfd)); //add to client
                setNonBlocking(cfd); //set nonblocking mode
                continue; //re-iterate with newly freshened L.L
                clientLength++;
            }   
        }

        //traverse through player L.L 
        temp = playerHead; 
        while(temp!= NULL) { 
            //Temporary variable storing retrieved client message following client protocol
            char msg[110];
            //check if ready 
            if(FD_ISSET(temp->socket_fd, &readfds)) { 
                strcpy(msg, ""); //instantiate to nothing
                //read message
                int res = read_from_user(temp, msg);
                if(res == 1) {  //everything went well 
                    if(temp->isRegistered != 1) {  //if unregistered player
                        //unregistered
                        decode_message_unregistered(temp, playerHead, msg);
                    }
                    else { 
                        //registered player 
                        decode_message_registered(temp, playerHead, msg);
                    }
                } 
                else if(res == -1) temp->isConnected=0; //error in reading, we disconnect them
                
            }
            temp = temp->next; 
        }
        //after listening, check deaths, disconnect, remove anyone if 'is connected' is 0 
        check_deaths(playerHead);
        playerHead=removeAllPlayers(playerHead);
    }
    
   close(sfd);
   return 0; 

}
