CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
HTTPD = httpd
HTTPD_OBJS = httpd.o
SERVER = server
SERVER_OBJS = server.o net.o
PROGS = $(HTTPD) $(SERVER)

all : $(PROGS)

$(HTTPD) : $(HTTPD_OBJS)
	$(CC) $(CFLAGS) -o $(HTTPD) $(HTTPD_OBJS)

httpd.o : httpd.c
	$(CC) $(CFLAGS) -c httpd.c

$(SERVER) : $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJS)

server.o : server.c net.h
	$(CC) $(CFLAGS) -c server.c

net.o : net.c net.h
	$(CC) $(CFLAGS) -c net.c


clean :
	rm *.o $(PROGS) core
