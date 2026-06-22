CC = mpicc
CFLAGS = -Wall -Wextra -DUSE_MPI -pthread -Iinclude
BIN = ./bin/
SRC = src/
LOG = logs/
INCLUDE = include/

OBJS = $(BIN)main.o $(BIN)coordinator.o $(BIN)client.o $(BIN)queue.o

$(BIN)client: $(BIN) $(OBJS) $(LOG)
	$(CC) $(OBJS) -o $(BIN)client $(CFLAGS)

$(BIN):
	mkdir -p $(BIN)

$(LOG):
	mkdir -p $(LOG)

$(BIN)main.o: $(SRC)main.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h $(INCLUDE)client.h 
	$(CC) $(CFLAGS) -c $(SRC)main.c -o $(BIN)main.o

$(BIN)coordinator.o: $(SRC)coordinator.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)coordinator.c -o $(BIN)coordinator.o

$(BIN)client.o: $(SRC)client.c $(INCLUDE)client.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)client.c -o $(BIN)client.o

$(BIN)queue.o: $(SRC)queue.c $(INCLUDE)queue.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)queue.c -o $(BIN)queue.o

run: $(BIN)client
	mpirun -np 3 xterm $(BIN)client

clean:
	rm -rf $(BIN) $(LOG)