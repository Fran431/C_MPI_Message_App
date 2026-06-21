CC = mpicc
CFLAGS = -Wall -Wextra -DUSE_MPI -pthread -Iinclude
BIN = bin/
SRC = src/
INCLUDE = include/

OBJS = $(BIN)main.o $(BIN)coordinator.o

$(BIN)client: $(BIN) $(OBJS)
	$(CC) $(OBJS) -o $(BIN)client $(CFLAGS)

$(BIN):
	mkdir -p $(BIN)

$(BIN)main.o: $(SRC)main.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)main.c -o $(BIN)main.o

$(BIN)coordinator.o: $(SRC)coordinator.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)coordinator.c -o $(BIN)coordinator.o

run: $(BIN)client
	mpirun -np 3 $(BIN)client

clean:
	rm -rf $(BIN)