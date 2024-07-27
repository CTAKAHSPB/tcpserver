SERVER_OBJECTS = server.o
CLIENT_OBJECTS = client.o
MYDIR=/home/pit/Projects/tcpserver/tests

default: all

all: client server

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

server: $(SERVER_OBJECTS)
	gcc $(SERVER_OBJECTS) -o $@

client: $(CLIENT_OBJECTS)
	gcc $(CLIENT_OBJECTS) -o $@

clean:
	rm -f $(CLIENT_OBJECTS) $(SERVER_OBJECTS)
	rm -f client server

tests: client server
	@bash -c "source ./tests/server_test.sh"
	@bash -c "source ./tests/client_test.sh"

.PHONY: tests
