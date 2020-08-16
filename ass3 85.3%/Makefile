CFLAGS = -Wall -pedantic -std=gnu99 -g -lm
TARGETS = 2310dealer 2310A 2310B

# Mark the default target to run (otherwise make will select the first target in the file)
.DEFAULT: all
# Mark targets as not generating output files (ensure the targets will always run)
.PHONY: all debug clean run

all: $(TARGETS)

# A debug target to update flags before cleaning and compiling all targets
debug: CFLAGS += -g
debug: clean $(TARGETS)

shared.o: shared.c shared.h
	gcc $(CFLAGS) -c shared.c -o shared.o

player.o: player.c player.h
	gcc $(CFLAGS) -c player.c -o player.o

2310dealer: 2310dealer.c shared.o
	gcc $(CFLAGS) 2310dealer.c shared.o -o 2310dealer

2310A: player.o shared.o playerAStrategy.c
	gcc $(CFLAGS) player.o shared.o playerAStrategy.c -o 2310A

2310B: player.o shared.o playerBStrategy.c
	gcc $(CFLAGS) player.o shared.o playerBStrategy.c -o 2310B

# Clean up our directory - remove objects and binaries
clean:
	rm -f $(TARGETS) *.o *.in *.out *.err
