OPTS:=-std=gnu99 -Wall -pedantic -Werror -Wextra -g

push2310: board.o errs.o main.o game.o
	gcc $(OPTS) -o push2310 board.o errs.o main.o game.o

game.o: game.c game.h
	gcc $(OPTS) -c game.c

board.o:    board.c board.h errs.h
	gcc $(OPTS) -c board.c

errs.o: errs.c errs.h
	gcc $(OPTS) -c errs.c

main.o: main.c errs.h board.h game.h
	gcc $(OPTS) -c main.c

clean:	
	rm *.o push2310
