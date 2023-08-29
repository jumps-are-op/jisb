.POSIX:
CFLAGS = -Wall -Wextra -Wpedantic
all: jisb
jisb: jisb.c config.h
clean:
	rm -f jisb
