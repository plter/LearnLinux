CC=gcc 
CFLAG=-Wall -g -O2
ALL=server client
OBJECT=server.o client.o
all:server client
server:server.o
	$(CC) $(CFLAG) $< -o $@
client:client.o
	$(CC) $(CFLAG) $< -o $@
%*.o:%*.c
	$(CC) $(CFLAG) -c $< -o $@
.PHONY:clean
clean:
	rm -rf $(ALL) $(OBJECT)
