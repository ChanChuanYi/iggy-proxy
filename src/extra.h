/* -- extra.h --
Author: Ignacio Llamas Avalos Jr
Class: CMPE 156 / Network Programming
*/
#ifndef __EXTRA_H__
#define __EXTRA_H__

#define FALSE 0
#define TRUE 1
#define PIPE_MAX 8192
#define BACKLOG 10
#define DATE_FORMAT "%a, %d %b %Y %H:%M:%S GMT"
#define LINE 1024

extern char forbidden[1024][1024];
extern char safe_host[1024][1024];
extern char safe_url[1024][1024];
extern char safe_port[1024][1024];

/////
//	aux function that will print an error message to the process debug file
//	then will close the file descritor, followed by terminating the child
//	pre:	assumes debug file, file descriptor are valid
//	post:	error is printed, file descriptor closed and child process has ended
/////
void call_death(FILE* d_out,int fd,int err,char* err_msg,char* write_pipe,char* req_err);

/////
//	checks the global forbidden array to see if the host is allowed to be 
//	connected to.
//	pre:	forbidden array has been filled, host follows pattern www.[site]
//	post:	returns a 403 Forbidden if host is not allowed, else returns
//			without doing anything
/////
void check_host(char* host, int client_fd, FILE* d_out,char* write_pipe);

/////
//	checks the host against the secure-site array to see if the request should 
//	follow a secure path
//	pre:	secure-site arrays are filled and host is formatted www.[site]
//	post:	returns -1 if host is not in the secure host list or the index where
//			port and url for the secure connection is located
/////
int check_secure(char* host);

/////
//	closes client socket feed and process log file
//	pre:	client_fd is active, d_out is still a valid pointer
//	post:	client_fd is closed, d_out is closed and flushed to main log file
/////
void close_all(int client_fd, FILE* d_out,int exit_status);


/////
//	goes through response and looks for Connection: close
//	pre:	write_pipe is a valid response
//	post:	if Connection:close is found, return TRUE else return FALSE
/////
int close_is_true(char* write_pipe);

/////
//	creates a socket for transmission to remote server and returns the socket
//	to the caller of the function
//	pre:	url string is a valid host address
//	post:	socket will pre opened and returned to caller, else -1 for error
/////
int create_host_socket(char* host,int client_fd,char* write_pipe,FILE* d_out);

/////
//	given an index, creates a secure connection to the host and prepares for
//	writing
//	pre:	index is not -1, client socket feed is open
//	post:	secure socket is established and returned
/////
int create_secure_socket(int index,int new_fd,char* write_pipe,FILE* d_out);


/////
//	prints errors to stderr
//	pre: 	valid char* message
//	post: 	will print error to stderr and exit with status(1)
/////
void error_print(char* message);

/////
//	grabs next string from buffer
//  pre: 	assumes incoming buffer ends in a \n
//  post: 	return buffer will contain next string
//	NOTE: 	this will return buffer with a start regardless of it being a string 
//		  	or not. User should check string size via strlen() to validate string
/////
int get_next_string(int start, char* search_buf, char* ret_buf);

/////
//	opens and fills the forbidden list and safe list
//	pre:	files secure-sites and forbidden-sites must be named exaclty and
//			follow the format supplied by project spec
//	post:	char* arrays will be filled with forbidden sites and secure sites
/////
void load_site_files();

/////
//	forms an error HTTP request that will be sent to the user
//	pre: 	requires write_pipe, error type, message to 
//       	user
//  post: 	user will be alerted of error type
/////
void send_error(int new_fd,char* write_pipe,int error,char* err_msg);

/////
//	catches the raised alarm signal
//	pre: 	required SIGALRM to be raised
//	post:	the process will be killed
void sig_handle(int sig); 

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
/////
int valid_method(char* method); 


void write_secure_host(int out_fd,int new_fd,char* read_pipe,char* write_pipe,FILE* d_out);

/////
//	this function will write to the internet and respond back to the client
//	pre:	host is connected, client is connected, read and write are valid
//	post:	write to host performed and client received response or error
/////
char* write_to_host(int out_fd,int new_fd,char* read_pipe,char* write_pipe,FILE* out);

#endif
