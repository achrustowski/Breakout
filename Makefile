CC = cc
CFLAGS = -g -Wall
IFLAGS = -I/opt/homebrew/Cellar/sdl2/2.32.8/include
LDFLAGS = -L/opt/homebrew/Cellar/sdl2/2.32.8/lib/

build:
	${CC} ${CFLAGS} main.c ${IFLAGS} ${LDFLAGS} -lSDL2 -o breakout

run:
	./breakout

clean:
	rm breakout
