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
	if(sockfd == -1) error_print("socket build error");
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1){ 
		error_print("socket bind error");
	}
    //bind complete
    freeaddrinfo(res);
    
    
    while(TRUE){
      if(break_flag == TRUE){
        close(new_fd);
        break_flag=FALSE;
      }
      
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
      int itr = 0;
      close(sockfd);
      printf("A new connection has been established\n");
      //connection is now accepted, time to send to connected client new_fd
              
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
		     
		     printf("from client on iteration:%d:\n%s\n",itr++,read_pipe);
		     if(strcmp(read_pipe,"exit") ==0){
		      printf("exit received, terminating connection\n");
		      break;
		     }
		     
		     //DEBUG
		     //send a 404 error
		     send_error(new_fd,write_pipe,404,"(Not Found)");
		        
		  }
	 }
     close(new_fd);
     }
       
    return(EXIT_SUCCESS);
}
          
      
