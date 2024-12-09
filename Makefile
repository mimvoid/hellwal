VERSION = 1.0.1

CFLAGS = -Wall -Wextra -O3
LDFLAGS = -lm

DESTDIR = /usr/local/bin

hellwal: hellwal.c
	$(CC) $(CFLAGS) $(LDFLAGS) hellwal.c -o hellwal

debug: hellwal.c
	$(CC) $(CFLAGS) -ggdb $(LDFLAGS) hellwal.c -o hellwal

clean:
	rm hellwal

install: hellwal
	mkdir -p $(DESTDIR)
	cp -f hellwal $(DESTDIR)
	chmod 755 $(DESTDIR)/hellwal # chmod u=rwx,g=rx,o=rx

uninstall:
	rm -f $(DESTDIR)/hellwal

release: hellwal
	tar czf hellwal-v$(VERSION).tar.gz hellwal

.PHONY: hellwal debug release clean install uninstall
