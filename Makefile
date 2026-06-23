
CC = mpicc
CFLAGS = -Wall -Wextra -DUSE_MPI -pthread -Iinclude
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)
 
BIN = bin/
SRC = src/
LOG = logs/
INCLUDE = include/
 
COMMON_OBJS = $(BIN)coordinator.o $(BIN)queue.o
 
CLI_OBJS = $(BIN)main_cli.o $(BIN)client_cli.o $(COMMON_OBJS)
GUI_OBJS = $(BIN)main_gui.o $(BIN)client_gui.o $(COMMON_OBJS)
 
all: $(BIN)client_cli $(BIN)client_gui
 
$(BIN)client_cli: $(BIN) $(LOG) $(CLI_OBJS)
	$(CC) $(CFLAGS) -o $(BIN)client_cli $(CLI_OBJS)
 
$(BIN)main_cli.o: $(SRC)main.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h $(INCLUDE)client_cli.h $(INCLUDE)client_gui.h
	$(CC) $(CFLAGS) -c $(SRC)main.c -o $(BIN)main_cli.o
 
$(BIN)client_cli.o: $(SRC)client_cli.c $(INCLUDE)client_cli.h $(INCLUDE)protocol.h $(INCLUDE)queue.h
	$(CC) $(CFLAGS) -c $(SRC)client_cli.c -o $(BIN)client_cli.o
 
$(BIN)client_gui: $(BIN) $(LOG) $(GUI_OBJS)
	$(CC) $(CFLAGS) -o $(BIN)client_gui $(GUI_OBJS) $(GTK_LIBS)
 
$(BIN)main_gui.o: $(SRC)main.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h $(INCLUDE)client_cli.h $(INCLUDE)client_gui.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -DGUI -c $(SRC)main.c -o $(BIN)main_gui.o
 
$(BIN)client_gui.o: $(SRC)client_gui.c $(INCLUDE)client_gui.h $(INCLUDE)protocol.h $(INCLUDE)queue.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $(SRC)client_gui.c -o $(BIN)client_gui.o
 
$(BIN)coordinator.o: $(SRC)coordinator.c $(INCLUDE)coordinator.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)coordinator.c -o $(BIN)coordinator.o
 
$(BIN)queue.o: $(SRC)queue.c $(INCLUDE)queue.h $(INCLUDE)protocol.h
	$(CC) $(CFLAGS) -c $(SRC)queue.c -o $(BIN)queue.o
 
$(BIN):
	mkdir -p $(BIN)
 
$(LOG):
	mkdir -p $(LOG)
 
N ?= 5
 
gui: $(BIN)client_gui
	mpirun --allow-run-as-root --oversubscribe -np $(N) $(BIN)client_gui
 
cli: $(BIN)client_cli
	mpirun --allow-run-as-root --oversubscribe -np $(N) xterm -hold -e $(BIN)client_cli
 
clean:
	rm -rf $(BIN) $(LOG)
