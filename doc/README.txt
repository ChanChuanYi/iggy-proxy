Ignacio Llamas Avalos Jr
illamasa@ucsc.edu
1252148

CMPE 156 Winter 2013
March 6, 2013

**README**
Makefile - basic makefile with a few funtions. easiest way to compile code for testing
server.c - server implementation of 'telnet'. It does not use authentication and is
     by no means a replacement for ssh/telnet/ect.
extra.c	 - helper functions for the server to keep main from becoming cluttered see 
		   extra.h for a full description of te functions	 
WHAT WAS IMPLEMENTED
     -server is able to handle GET and HEAD commands. Unfortunally it will hiccup
      on js and jpg files.
     -this code is tested and known to work on mint linux 14 x64
How to run
     The easiest way to get up and running is to use the included makefile.
     run 'make' to make server
     ./server [port number] will run the generated executable for server
     server will display error messages as errors are detected.
     server will log requests to log.txt. this log was made in a+ mode, so it will
     append new requests to the log in subsequent runs
Credit and Acknowledgements
     - The skeleton to start the connection for both the client and server where used from the
       examples provided in the online book provided at beej.us unix programming guide under
       http://www.beej.us/guide/output/print/bgnet_USLetter_2.pdf pages 26,34,35.
     - code for the error_print function is from a previous assignment from CMPE 150, Fall 2012
