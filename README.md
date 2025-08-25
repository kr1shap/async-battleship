
---

# Async-Battleship

An asynchronous Battleship game server built using **I/O multiplexing** with the Unix `select()` API. This project demonstrates socket programming in C, handling of multiple clients [non-blocking issues handled], as well as other errors such as 'writers block'. 

## Updates [08.25.2025]
* Added random coordinate generating option for users 
* Allowed users to send async messages to other registered users
* Added ANSI colouring to server -> client messages
* Client bot! Use it for testing/running

## Features
* Asynchronous server using `select()` for multiple clients
* Stores connected clients in a linked list structure
* Non-blocking I/O to handle multiple game sessions concurrently
* Handling of errors such as SIGPIPE, EAGAIN regarding non-blocking I/O
* Simple Battleship game logic for up to ~2000 players (the limit of select()!)

## Simple How-to!

Some important assumptions: For simplicity, (any message sent to server) >= 99 characters is treated as a disconnection. As well, this means that sending messages to other users has a 60 character limit. 

```bash
direction = | OR -  # | is vertical ship, - is horizontal, where the ship is 5 blocks in length.
REG name    #register, with a given arbituary x, y, direction
REG name x y direction  #register with given x, y, direction. 
BOMB x y    #bomb a given coordinate  
SEND username message   #send message (spaces allowed) to given username. Note 0 <= len(message) <= 60
LIST    #list out all registered users, and their usernames
```

## Getting Started

* GCC or any C compiler
* Unix-based environment (Linux/macOS)

### Compilation

```bash
make server 
make bot
```

### Running

Start the server:

```bash
./server <port>
```

Running as a client:
- To act as a client, you can use the 'nc' command in a new terminal, where port is the same number used in starting the server

```bash
 nc localhost <port>
```

For the client bot, running it is as simple as you think 

```bash
./clientbot port    #note that the bot connects to localhost. Thus, this game will work on your local machine.
```

## Future Updates

* Implement a front-end UI (so clients don't have to use the terminal)
* Add a client bot!
* Implement better error handling and logging

