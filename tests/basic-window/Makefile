CC = cc
CFLAGS = -std=c99 -Wall -Wextra -O2
LIBS = -lX11

SRC = main.c \
      ../../maus.c \
      ../../maus_x11.c

all:
	${CC} ${SRC} -o basic-window ${CFLAGS} ${LIBS}

