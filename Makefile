CC = mpicc
CFLAGS = -Wall -Wextra -DUSE_MPI -pthread -Iinclude
BIN = ./bin/
SRC = src/
LOG = logs/
INCLUDE = include/

OBJS = $(BIN)main.o $(BIN)coordinator.o $(BIN)client_cli.o $(BIN)queue.o

$(BIN)client: $(BIN) $(OBJS) $(LOG)
	$(CC) $(OBJS) -o $(BIN)client_cli $(CFLAGS)

$(BIN):
	mkdir -p $(BIN)

$(LOG):
	mkdir -p $(LOG)

$(BIN)main.o: $(SRC)main.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h $(INCLUDE)client_cli.h 
	$(CC) $(CFLAGS) -c $(SRC)main.c -o $(BIN)main.o

$(BIN)coordinator.o: $(SRC)coordinator.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)coordinator.c -o $(BIN)coordinator.o

$(BIN)client_cli.o: $(SRC)client_cli.c $(INCLUDE)client_cli.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)client_cli.c -o $(BIN)client_cli.o

$(BIN)queue.o: $(SRC)queue.c $(INCLUDE)queue.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)queue.c -o $(BIN)queue.o

run: $(BIN)client
	mpirun -np 5 xterm $(BIN)client_cli

clean:
	rm -rf $(BIN) $(LOG)