NAME = dwmstatus
VERSION = $(shell git rev-list --count HEAD).$(shell git rev-parse --short HEAD)

CFLAGS = -g -std=c99 -pedantic -Wall -O0
LDFLAGS = -g -lX11 -lmpdclient

SRC = ${NAME}.c
OBJ = ${SRC:.c=.o}

all: ${NAME}

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h function.h

${NAME}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	${RM} ${NAME} ${OBJ} ${NAME}-${VERSION}.tar.gz

dist:
	git archive --format=tar --prefix=${NAME}-${VERSION}/ HEAD | gzip -9 > ${NAME}-${VERSION}.tar.gz

install: all
	install -Dm755 dwmstatus ${DESTDIR}/usr/bin/${NAME}
	install -Dm755 dwmvolume ${DESTDIR}/usr/bin/dwmvolume

uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/${NAME}
	${RM} ${DESTDIR}${PREFIX}/bin/dwmvolume

.PHONY: clean dist install uninstall
