VERSION = $(shell git archive --always)

CFLAGS = -g -std=c99 -pedantic -Wall -O0
LDFLAGS = -g -lX11 -lmpdclient

all: dwmstatus

dwmstatus: dwmstatus.c dwmstatus.h

clean:
	$(RM) dwmstatus

dist:
	git archive --format=tar --prefix=dwmstatus-$(VERSION)/ HEAD | gzip -9 > dwmstatus-$(VERSION).tar.gz

install: dwmstatus
	install -Dm755 dwmstatus $(DESTDIR)/usr/bin/dwmstatus
	install -Dm755 dwmvolume $(DESTDIR)/usr/bin/dwmvolume

.PHONY: clean dist install
