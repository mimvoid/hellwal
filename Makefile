VERSION=0.0.5
CFLAGS = -Wall -Wextra -lm -O3

hellwal: hellwal.c
	$(CC) $(CFLAGS) hellwal.c -o hellwal

debug: hellwal.c
	$(CC) $(CFLAGS) -ggdb hellwal.c -o hellwal 

clean:
	rm hellwal

pkg: clean
	mkdir -p hellwal-$(VERSION)
	cp -R LICENSE* Makefile README.md hellwal.c hellwal-$(VERSION)
	tar -caf hellwal-$(VERSION).tar.gz hellwal-$(VERSION)
	rm -rf hellwal-$(VERSION)

install: hellwal
	mkdir -p /usr/local/bin
	cp -f hellwal /usr/local/bin
	chmod 755 /usr/local/bin/hellwal # chmod u=rwx,g=rx,o=rx
uninstall:
	rm -f usr/local/bin/hellwal

.PHONY: hellwal
