# CSSE2310-C
UQ CSSE2310 C

Systems Programming in C. Operating Systems Principles: memory management, basics of machine organization, file systems, processes & threads, interprocess communication. Computer Networks Principles: topologies & models of computer networks, protocols, network programming, network applications.

## Assignment1: a C99 program (called push2310) which allows the user to play a game
### Game instructions
The game is played on an R×C grid of cells (where R is the number of rows and C is the number of columns). The corners of the board are removed. Each cell has a point value. The value of the border cells is zero, interior cells have values between 1 and 9 (inclusive).
Empty cells are indicated with a dot. Players take turns placing “stones” on empty cells. 

The game ends when the interior of the board is full.

Each player gets points for each cell they have a stone in.
### Invocation
The push2310 program takes the following parameters in order:
1. The type of player for O — This must be one of 0, 1, H. Where zero and one are automated players and H is a human player.
2. The type of player for X — same possibilities as above.
3. The save game file to load.
eg: push2310 0 0 prev would start game loading from a save file called prev. Both players will be type 0 automated players. 

Note that even new games start with a save game.
### Interaction
After the game has been loaded, and after each player makes a move, display the board. Whenever a human player needs to provide a move, display the following prompt:
? :(R C)>
where, ? is either O or X (depending whose turn it is). Note that there is a single space character following the >. If the input is not valid, show the prompt again. 
  
  When an automated player makes a move, print the following before redisplaying the board: 

Player ? placed at ? ? where the missing values are:
1. The player character
2. The row the player placed in
3. The column the player placed in
