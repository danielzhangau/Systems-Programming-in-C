CFLAGS = -std=gnu99 -pedantic -Wall -pthread -lm -lpthread -lrt -g
TARGETS = mapper2310 control2310 roc2310

# Mark the default target to run (otherwise make will select the first target in the file)
.DEFAULT: all
# Mark targets as not generating output files (ensure the targets will always run)
.PHONY: all debug clean run

# A debug target to update flags before cleaning and compiling all targets
debug: CFLAGS += -g
debug: clean $(TARGETS)

debug: CFLAGS += -g
debug: clean $(EXECS)

shared.o: shared.c shared.h
	gcc $(CFLAGS) -c shared.c -o shared.o

connectionHandler.o: connectionHandler.c connectionHandler.h
	gcc $(CFLAGS) -c connectionHandler.c -o connectionHandler.o

mapper2310: shared.o connectionHandler.o mapper2310.c
	gcc $(CFLAGS) shared.o connectionHandler.o mapper2310.c -o mapper2310

control2310: shared.o connectionHandler.o control2310.c
	gcc $(CFLAGS) shared.o connectionHandler.o control2310.c -o control2310

roc2310: shared.o connectionHandler.o roc2310.c
	gcc $(CFLAGS) shared.o connectionHandler.o roc2310.c -o roc2310

# Clean up our directory - remove objects and binaries
clean:
	rm -f $(TARGETS) *.o *.in *.out *.err
