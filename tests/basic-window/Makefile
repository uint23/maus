CC = cc
CFLAGS = -std=c99 -Wall -Wextra -g
CPPFLAGS = -DBACKEND_X11
LIBS = -lX11 -lXext

SRC = main.c \
      ../../maus.c \
      ../../maus_x11.c

all:
	${CC} ${SRC} -o basic-window ${CFLAGS} ${LIBS}

