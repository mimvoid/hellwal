VERSION = 1.0.0

CFLAGS = -Wall -Wextra -lm -O3
DESTDIR = /usr/local/bin

hellwal: hellwal.c
	$(CC) $(CFLAGS) hellwal.c -o hellwal

hellwal-gdb: hellwal.c
	$(CC) $(CFLAGS) -ggdb hellwal.c -o hellwal-gdb

clean:
	rm hellwal hellwal-v*

install: hellwal
	mkdir -p $(DESTDIR)
	cp -f hellwal $(DESTDIR)
	chmod 755 $(DESTDIR)/hellwal # chmod u=rwx,g=rx,o=rx

uninstall:
	rm -f $(DESTDIR)/hellwal

release: hellwal
	tar czf hellwal-v$(VERSION).tar.gz hellwal

.PHONY: hellwal release clean install uninstall
