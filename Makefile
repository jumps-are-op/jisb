.POSIX:
CFLAGS = -Wall -Wextra -Wpedantic
all: jisb
jisb: jisb.c config.h
	$(CC) $(CFLAGS) jisb.c -o jisb
clean:
	rm -f jisb
