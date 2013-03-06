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
    printf("server is now bound to port:%s\n",port);
    
    while(TRUE){
      printf("listening for a new connection\n");
      listen(sockfd, BACKLOG);
      // now accept an incoming connection:
      addr_size = sizeof their_addr;
      
      new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
      if(new_fd == -1){
        fprintf(stderr,"unable to  on accept establish connection\n");
        continue;  
      }
      printf("new connection has been established, preparing to fork, current fd:%d\n",new_fd);
      
      int child = fork();
      if(child == 0){
      	printf("I am in the fork, current fd:%d\n",new_fd);
      	//child variables
      	char method[LINE];
      	char url[LINE];
      	char http[LINE];
      	char arg[LINE];
      	char host[LINE];
      	int out_fd;
		  close(sockfd);
		  //printf("A new connection has been established in child\n");
		  //connection is now accepted, time to send to connected client 
		  //new_fd
		          
		  //while loop will handle commands
		  while(TRUE){
		  	printf("top of child while loop\n");
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
		     printf("from the client:\n%s\n",read_pipe);
		     
		     	start = get_next_string(start, read_pipe, line);
		     	if(strlen(line)<=0){
		     		send_error(new_fd,write_pipe,404,"Bad Request");
		     		continue;
		     	}
		     	
		     	int n = sscanf(line, "%s %s %s", method, url, http);
		     	if(n!=3){
		     		send_error(new_fd,write_pipe,404,"Bad Request");
		     		continue;
		     	}
		     	if(!valid_method(method)){
		     		send_error(new_fd,write_pipe,501,"Not Implemented");
		     		continue;
		     	}
		     	
		     	start = get_next_string(start, read_pipe, line);
		     	if(strlen(line)<=0){
		     		send_error(new_fd,write_pipe,404,"Bad Request");
		     		continue;
		     	}
		     	
		     	n = sscanf(line, "%s %s", arg,host);
		     	if(n!=2){
		     		send_error(new_fd,write_pipe,404,"Bad Request - No host");
		     		continue;
		     	}
		     	if(strcmp(arg,"Host:")!=0){
		     		send_error(new_fd,write_pipe,404,"Bad Request - No host");
		     		continue;
		     	}
		     	printf("host:%s\n",host);
		     	printf("attemptig to create connection to host\n");		     	
		     	out_fd = create_host_socket(host);
		     	printf("connection to host established\n");
		     	
		     	write_to_host(out_fd,new_fd,read_pipe,write_pipe);
		     	close(out_fd);
		        
		  }
	 }
	 //printf("parent closing new_fd\n");
     close(new_fd);
     }
       
    return(EXIT_SUCCESS);
}
          
      
