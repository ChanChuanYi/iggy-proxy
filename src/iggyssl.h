#ifndef __IGGYSSL_H__
#define __IGGYSSL_H__

/* 	This header file defines all the aux functions used by the server to
	send/receive http requests over a secured connection
*/

/////
//	creates a secure connection to the host and handles i/o to client
//	pre:	client_fd is still open, read_pipe contains the request,
//			write pipe is valid for writing, index contains valid host
//			index
//	post:	request will be sent and processed over a secured connection to
//			the host over SSL
/////
void secure_and_send(int client_fd, char* read_pipe, char* write_pipe, int index, FILE* d_out);

#endif
