SYSC 4001 Operating Systems -- Assignment 3

Written by:

Charles Bergeron -- 100883684
Connor Matthews -- 100892794

Compilation Instructions:

	To compile partB.c under Linux, call % g++ partB.c -lpthread

	NOTE: gcc may be used, if you have a version that is compatible with C99, or add a flag adding this effect

Design Decisions:

Three active message queues were implemented in this program.  One for interface output, one for interface input, and one for server input and output.
The reason that the interface has input AND output separated is that occasionally, if using only one message queue, one thread could accidentally read in
the message it had just sent out, breaking the program.  Having dedicated queues is easier to use, and fixes this problem.

Three queues are running in parallel, one for the ATM interface, one for the server, and another for the editor.  They communicate through each other
using IPC message queues, and simply pass strings to one another, as well as an integer representing the message type, relaying the meaning of the
contents of the string message.
