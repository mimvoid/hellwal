CFLAGS = -ggdb -Wall -Wextra -lm -O3

hellwal: hellwal.c
	$(CC) $(CFLAGS) hellwal.c -o hellwal

clean:
	rm hellwal colors.pallette
