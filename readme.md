# MPI Messaging system.

A chat application built on MPI. A coordinator process (rank 0) receives and routes messages to the final receiver. Clients are represnted by MPI processes (rank 1,2,3,4...). The messaging client has two versions, a CLI (Command Line Interface) and a window client using GTK.

## What it does

-   Every client connects to the coordinator process with an username.
-   Using another processe's rank, they can send a direct message, or by leaving the space empty, they can broadcast a message to all connected clients.
-   The coordinator is left out of the chat, it only passes messages around, and logs activities.
-   Each client runs its own interface thread, so that the thread passing the MPI messages does not get stuck waiting fror a message.

## Protocol

A protocol for passing messages was needed to handle process-to-process communication the protocol consists of the following elements:

#### Message types enum:

An enum that houses several tags, passed as MPI tags to communicate the intention of the message passed, the following tags are used:

-   TAG_REGISTER: To register an user into the coordinator process.
-   TAG_DIRECT: To send a direct message from user "A" to user "B"
-   TAG_DIFFUSION:  To send a message to all connected users.
-   TAG_DISCONNECT: To disconnect an user from the coordinator process.

#### Message protocol struct:

A struct containing elements that conform a message:

-   The rank of the process that sent the message.
-   The name of the user that sent the message.
-   The rank of the process that received the message.
-   The length of the message.
-   The message itself (as an array of char).

#### Register protocol struct:

-   The rank of the process that is going to be registered.
-   The name of the user that will be registered.


## Design considerations:

Both versions are relatively similar, and even share some functions, the main difference is the use of GTK. In the CLI version, the main process calls the run_cli function for each secondary process, then each process launches two threads, one to show messages and another to read them, those processes communicate with a thread-safe queue. The reading thread, passes messages to the coordinator process, which routes them to the respective process, and this last one uses the show thread to show them to the user.

The GTK version, only required one thread to read and show data, since GTK has the capacity to do both, this GTK thread uses thread safe queues to route data to the coordinator and this last one send the message to the respective process. Which shows it in its window.

GTK was chosen, mainly because of a desire to learn a new thing, and because of my previous experience using inkscape, which is based on GTK.



## Makefile usage

```bash
make            # compiles both versions, the GUI and CLI

make clean      # deletes binaries, executables and logs
 
make cli N=4    # Runs the CLI version, it uses xterm to represnt each client as its own terminal

make gui N=4    # Runs the GUI version, it uses a GTK window to represent each client
```

'N' refers to the number of processes, if it is not given as an argument, it takes a default value of 3.

To compile, you must install 'mpicc', 'libgtk-3-dev', 'xterm'

## Two binaries

The versions where separated like this, so that the CLI version had more independence from GTK. It uses preprocessor macros to decide which version to compile.

## Development challenges

- The program has only one thread making MPI calls, which made it more clear during design phase, but ended up needing a clear separation between said thread, and the thread that receives input from the user. Which created the need for a thread-safe queue to handle communication.

- GTK required only one thread initializing and running the GTK event loop, which made the original username prompt obsolete, and a window needes to be added to handle the new username input.

- In both implementations, the need for a locking and not locking dequeue arised, when the need for a constantly looping check for new messages needed to be locked to keep checking messages. The first implementation ended up exiting as soon as no messages were on queue, before even the first user could send a message.

- Because mpirun opens multiple copies of the same program, but only opens one terminal, to have a working CLI version, research was needed to find means to represent the clients. Xterm is used to accomplish this task, opening a terminal for each process launches by mpirun.

- Reseacrhing GTK took some effort, even though it's documentation is (in my opinion) well done, with picturesc examples, and clear explanations, the big ammount of widgets and other functionalities slowed the undertanding process of GTK.


---

Francisco Sanchez Moran B87341
