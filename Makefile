.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=FtpServer
OBJS=main.o sysutil.o session.o ftp_proto.o ftp_nobody.o strutil.o configure.o parse_conf.o
LIB=-lcrypt
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
