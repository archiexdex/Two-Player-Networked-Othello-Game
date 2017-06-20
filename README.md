# Two-Player Networked Reversi(Othello) Game

## Compile and Run

- `make clean`
- `make`
- `./othello -s <PORT>`
    - server will wait for only one client to connect
- `./othello -c <IP>:<PORT> ` 
    - prefer `127.0.0.1` than `localhost`
    
## Features


[O] Your program can act as either a server (player #1) or a client (player #2) by using the respective command arguments.
[O] A server has to wait for a client connection.
[O] A client can connect to the given server (IP address or host name).
[O] Once connected, display the game board. The game always starts from player #1 (server).
[O] Player can only put pieces (discs) on valid places (see game rule).
[O] Display correct number of pieces on the game board for the both players.
[O] Implement the rest of game logics.
[O] When there is no more moves, display a message to show the player wins or loses.
[O] Ensure the both two players have the same view of game board. If either the client or the server quits, the peer has to be terminated as well.

## How to play ?

- Move
    - Up: `k` or `key up`
    - Down: `j` or `key down`
    - LEFT: `h` or `key left`
    - RIGHT: `l` or `key right`
- Put a piece
    - `space` or `Enter`
- Quit
    - `q` or `Q`
