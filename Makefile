CFLAGS = -Wall -Wextra -lm -O3

hellwal: hellwal.c
	$(CC) $(CFLAGS) hellwal.c -o hellwal

debug: hellwal.c
	$(CC) $(CFLAGS) -ggdb hellwal.c -o hellwal 

clean:
	rm hellwal

.PHONY: hellwal
