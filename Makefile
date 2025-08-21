CC = gcc
CFLAGS = -Wall -Wextra -g
DEPS = user.h gamelogic.h
OBJ = server.o user.o gamelogic.o 

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

server: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o server

clean:
	rm -f *.o server