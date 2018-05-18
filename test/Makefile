
CFLAGS = -I../include -I../driver -g

DRIVER_DIR = ../driver
DRIVER_SRC = $(DRIVER_DIR)/xterm.c

SERVER_DIR = ../server
SERVER_SRC = $(SERVER_DIR)/server.c

all: test


test: testapp.c $(SERVER_SRC) $(DRIVER_SRC)
	$(CC) $(CFLAGS) testapp.c $(SERVER_SRC) $(DRIVER_SRC) -o testapp
