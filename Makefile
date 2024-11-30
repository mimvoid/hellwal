CFLAGS = -Wall -Wextra -lm -O3

hellwal: hellwal.c
	$(CC) $(CFLAGS) hellwal.c -o hellwal

debug: hellwal.c
	$(CC) $(CFLAGS) -ggdb hellwal.c -o hellwal 

clean:
	rm hellwal

install: hellwal
	mkdir -p /usr/local/bin
	cp -f hellwal /usr/local/bin
	chmod 755 /usr/local/bin/hellwal # chmod u=rwx,g=rx,o=rx

uninstall:
	rm -f /usr/local/bin/hellwal

release: hellwal
	mkdir -p release
	cp hellwal release/
	tar czf release/hellwal-v$(VERSION).tar.gz -C release hellwal

.PHONY: hellwal release clean install uninstall
