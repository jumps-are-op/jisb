.POSIX:
CFLAGS = -Wall -Wextra -Wpedantic -lX11
all: jisb
jisb: jisb.c config.h
clean:
	rm -f jisb
