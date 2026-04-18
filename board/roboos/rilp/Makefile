CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lpthread

SRCS = src/rilp.c src/rilp_socket.c
OBJS = $(SRCS:.c=.o)

LIBRARY = librilp.a
TEST_PROG = rilp_test

all: $(LIBRARY) $(TEST_PROG)

$(LIBRARY): $(OBJS)
	ar rcs $@ $^

$(TEST_PROG): src/rilp_test.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ src/rilp_test.o -L. -lrilp $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) src/rilp_test.o $(LIBRARY) $(TEST_PROG)

install: $(LIBRARY) $(TEST_PROG)
	mkdir -p $(DESTDIR)/usr/local/lib
	mkdir -p $(DESTDIR)/usr/local/include
	mkdir -p $(DESTDIR)/usr/local/bin
	cp $(LIBRARY) $(DESTDIR)/usr/local/lib/
	cp include/rilp.h $(DESTDIR)/usr/local/include/
	cp $(TEST_PROG) $(DESTDIR)/usr/local/bin/

.PHONY: all clean install
