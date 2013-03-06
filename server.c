#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include "extra.h"


int main(int argc,char** argv){
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd, data_size;
	char read_pipe[PIPE_MAX],write_pipe[PIPE_MAX];
	int break_flag = FALSE;
	int start, end = 0;
	char line[LINE];
	  
	//1st check:user must enter a port number!
	if(argc !=2){error_print("Usage: server [port number]");}
	char* port  = argv[1];  
    // !! don't forget your error checking for these calls !!
    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    if(getaddrinfo(NULL, port, &hints, &res) != 0){
		error_print("unable to identify address");
	}
    // make a socket, bind it, and listen on it:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int on = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0){
            error_print("unable to set SO_REUSEADDR option");
        }
	if(sockfd == -1) error_print("socket build error");
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1){ 
		error_print("socket bind error");
	}
    //bind complete
     
    freeaddrinfo(res);
    
    
    while(TRUE){
      
      listen(sockfd, BACKLOG);
      // now accept an incoming connection:
      addr_size = sizeof their_addr;
      
      new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
      if(new_fd == -1){
        fprintf(stderr,"unable to establish connection\n");
        continue;  
      }
      
      
      int child = fork();
      if(child == 0){
      	//child variables
      	char method[LINE];
      	char url[LINE];
      	char http[LINE];
      	int out_fd;
		  close(sockfd);
		  //printf("A new connection has been established in child\n");
		  //connection is now accepted, time to send to connected client 
		  //new_fd
		          
		  //while loop will handle commands
		  while(TRUE){
		    if(break_flag==TRUE){
		      break_flag=FALSE;
		      break;
		    }
		     //command read
		     data_size=0;
		     memset(read_pipe,0,PIPE_MAX);
		     data_size = read(new_fd,read_pipe,PIPE_MAX);
		       if(data_size < 0){
		         fprintf(stderr,"something is wrong with this connection. terminating\n");
		         break_flag=TRUE;
		         continue;
		       }
		     
		     if( (read_pipe == (char*) 0) || (data_size == 0)){
		     	send_error(new_fd,write_pipe,404,"Bad Request");
		     	close(new_fd);
		     	exit(EXIT_FAILURE);
		     }
		     
		     	start = get_next_string(start, read_pipe, line);
		     	if(strlen(line)<=0){
		     		send_error(new_fd,write_pipe,404,"Bad Request");
		     		continue;
		     	}
		     	
		     	printf("valid line, attempting to parse\n");
		     	int n = sscanf(line, "%s %s %s", method, url, http);
		     	if(n!=3){
		     		send_error(new_fd,write_pipe,404,"Bad Request");
		     		continue;
		     	}
		     	if(!valid_method(method)){
		     		printf("not a valid method, sending 501 error\n");
		     		send_error(new_fd,write_pipe,501,"Not Implemented");
		     		continue;
		     	}
		     	
		     out_fd = create_client_socket(url);
		     
		     
		     //DEBUG
		     //send a 404 error
		     send_error(new_fd,write_pipe,418,"I am a teapot");
		     //printf("sending to client:\n%s\n",write_pipe);
		     
		        
		  }
	 }
	 //printf("parent closing new_fd\n");
     close(new_fd);
     }
       
    return(EXIT_SUCCESS);
}
          
      
