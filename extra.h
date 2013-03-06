#ifndef __EXTRA_H__
#define __EXTRA_H__

#define FALSE 0
#define TRUE 1
#define PIPE_MAX 4096
#define BACKLOG 10
#define DATE_FORMAT "%a, %d %b %Y %H:%M:%S GMT"
#define LINE 1024

/////
//	creates a socket for transmission to remote server and returns the socket
//	to the caller of the function
//	pre:	url string is a valid host address
//	post:	socket will pre opened and returned to caller, else -1 for error
/////
int create_client_socket(char* url);

/////
//	prints errors to stderr
//	pre: valid char* message
//	post: will print error to stderr and exit with status(1)
/////
void error_print(char* message);

/////
//	grabs next string from buffer
//  pre: assumes incoming buffer ends in a \n
//  post: return buffer will contain next string
//	NOTE: this will return buffer with a start regardless of it being a string 
//		  or not. User should check string size via strlen() to validate string
/////
int get_next_string(int start, char* search_buf, char* ret_buf);  

/////
//	forms an error HTTP request that will be sent to the user
//	pre: requires write_pipe, error type, message to 
//       user
//  post: user will be alerted of error type
/////
void send_error(int new_fd,char* write_pipe,int error,char* err_msg);

/////
//	sets the content headers
//	pre: 	requires write_pipe
//  post:	headers will be set for the write_pipe
/////
void set_headers(char* write_pipe, int status, char* status_msg,int close_bool);

/////
//	simple function that checks if http method is GET or HEAD. It will return
//  TRUE if method is HEAD or GET and FALSE if anything else
//	pre: 	valid method string is assumed
//	post: 	no data is mutated, simple int return value
int valid_method(char* method); 

#endif
