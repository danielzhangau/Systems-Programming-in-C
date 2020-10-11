# Systems Programming in C
UQ CSSE

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

![A1demo](https://github.com/danielzhangau/CSSE2310-C/blob/master/a1demo.png)

## Assignment2: The Binary Bomb
The aim of this part of the assessment is to develop and test your skills using the gdb debugger. This exercise will solving puzzles (called phases) about the behaviour of a program called the Binary Bomb. You must \defuse" each phase by entering the correct passphrase. You will need to deduce this passphrase by gaining insights into the the bomb program's operation in gdb.

## Assignment3: a group of three C99 programs to play a game
One of the programs will be called 2310dealer it will control the game. The other two programs (2310A and 2310B will be players.
The dealer will be responsible for running the player processes and communicating with them via pipes (created via pipe()). These pipes will be connected to the players’ standard ins and outs so from the players’ point of view, communication will be via stdin and stdout.
### Game Description
The game is played using a sequence of sites called the path. Each site has a type and a limit of the number of players that can visit that site. Each player starts the game with seven money. Players take turns moving forward some number of steps, the type of site they end on determines what action they take.
### 2310dealer
The dealer will take the following commandline arguments (in order):
• the name of the file to read the item deck from.
• the name of the file to read the path from.
• one or more player programs to start
Eg: ./2310dealer d1.deck 1.map ./X ./X ./Y
Would start a game with two X players and a Y player.
Note: your dealer should run whatever program names it is given. Do not try to adjust your dealer’s behaviour based on the names it is passed.
When running player processes, the dealer must ensure that any output to stderr by players is supressed. 
As soon as the dealer reads a ˆ from a player process, the dealer should send the path to that player.
When the dealer receives SIGHUP, it should kill and reap any remaining child processes. Note: We won’t test the exit status for 2310dealer when it receives SIGHUP.
### Players
All player processes take the following commandline arguments (in order):
• Number of players
• This player’s ID (starting at 0)
Once the player has checked its commandline arguments, it should:
1. Print a caret(ˆ) to its standard out (no newline following it).
2. Read the path from stdin (in the same format as the path file).

![A3demo](https://github.com/danielzhangau/CSSE2310-C/blob/master/a3demo.png)

## Assignment4: a simple networked simulation of plane flights
A complete assignment will consist of three C99 programs:
• roc2310: Simulates an aircraft
• control2310: Simulates an airport
• mapper2310: Maps airport names to ports
Multiple instances of these programs will be executed to carry out the simulation.
You will use pthreads and communicate via IPv4 TCP networking. Your implementation must use blocking communications. You are not permitted to use pipes or any alternative means of interprocess commmunication. To complete all of the features of this assignment you will need to use pthreads, you are not to call fork() nor attempt to use any form of nonblocking I/O or I/O multiplexing.
### mapper2310
This program takes no commandline parameters. When it starts up, it will
1. listen on an ephemeral port,
2. print that port to stdout.
3. act on any commands it receives via connection to the port (the other end of the connection will close it when it has no more commands).

![A4demo](https://github.com/danielzhangau/CSSE2310-C/blob/master/a4mapperdemo.png)

### control2310
This program takes the following parameters:
• AirportID
• Airport info
• mapper port (This is optional).
When the program starts up, it will:
1. Listen on an ephemeral port.
2. Print that port to stdout.
3. If a mapper port is given, connect to the mapper and register the ID and port of this airport. It should then disconnect from the mapper.
4. In parallel — wait for connections by planes and act on them.
When a connection is made to the control’s port, one of two things should happen:
• If the text sent by the connecting party is “log”, then send back a newline seprated list of all the rocs which have visited them (in lexicographic order). Following this, it should send a full-stop followed by a newline and then close the connection.
• For any other text, the control should consider the text as the plane’s ID and send back the control’s info (newline terminated).
### roc2310
This program takes the following commandline parameters:
1. planeID
2. mapper port (This is either a port number or a dash (‘-’)).
3. zero or more destinations (controls) to connect to in order. These can either be port numbers or control IDs.
### roc2310 / control2310 communication
When roc2310 connects to a control, roc will send its ID to control and the control will send back its info.
  For example:
$ ./control2310 BNE "Quarantined"
45643
$ ./roc2310 F100 - 45643
  In this case, the roc would send F100 and the control would send Quarantined.

# What I learned: 
- Compilling c and intial features
- Processes
- Virtual memory
- File descriptor and pipe
- Threads and synchronization
- Networks - layers, ogres
- DNS + HTTP
- File Systems
