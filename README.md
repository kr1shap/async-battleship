
---

# Async-Battleship

An asynchronous Battleship game server built using **I/O multiplexing** with the Unix `select()` API. This project demonstrates socket programming in C, handling of multiple clients [non-blocking issues handled], as well as other errors such as 'writers block'. 

## Features

* Asynchronous server using `select()` for multiple clients
* Stores connected clients in a linked list structure
* Non-blocking I/O to handle multiple game sessions concurrently
* Handling of errors such as SIGPIPE, EAGAIN regarding non-blocking I/O
* Simple Battleship game logic for up to ~2000 players (the limit of select()!)

## Getting Started

* GCC or any C compiler
* Unix-based environment (Linux/macOS)

### Compilation

```bash
make server 
```

### Running

Start the server:

```bash
./server <port>
```


Running as a client:
- In the future, I will add my own client, but to act as a client, you can use the 'nc' command in a new terminal, where port is the same number used in starting the server

```bash
 nc localhost <port>
```

## Future Updates

* Improve game state handling and persistence
* Implement better error handling and logging
* Branch out to epoll() API, use more advanced C library functions
* Use more efficient data structures to store clients, for larger handling. 

