CC=cc
CFLAGS=-Wall -Werror

clock: clock.c
	$(CC) $(CFLAGS) -o clock clock.c

