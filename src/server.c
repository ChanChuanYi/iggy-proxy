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
#include <time.h>
#include "extra.h"
#include "iggyssl.h"


int main(int argc,char** argv){
    int sockfd, new_fd, data_size;
	char read_pipe[PIPE_MAX],write_pipe[PIPE_MAX];
	int start=0, end = 0;
	time_t now = time(0);
	char line[LINE];
	  
	//1st check:user must enter a port number!
	if(argc !=2){error_print("Usage: server [port number]");}
	char* port  = argv[1];
	
	struct sockaddr_storage their_addr;
	struct addrinfo hints, *res;
    socklen_t addr_size;
    
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
	freeaddrinfo(res);
    //bind complete
     
    //load forbidden sites secure sites into memory
    load_site_files();
    
    while(TRUE){
      	listen(sockfd, BACKLOG);
      	// now accept an incoming connection:
      	addr_size = sizeof their_addr;
      
      	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
      	if(new_fd == -1){
        	error_print("unable to accept() a connection");
        	continue;  
      	}
      
      	int child = fork();
      	if(child == 0){
      		close(sockfd);
			//start the log
    		FILE *tmp_out;
   		 	tmp_out = fopen("log.txt","a+");//open output file in read-only
   			if(tmp_out == 0)error_print("unable to open log\n");      		
      		
      		//child variables
      		char method[LINE];
      		char url[LINE];
      		char http[LINE];
      		char arg[LINE];
      		char host[LINE];
      		char buf_for_time[LINE];
      		int out_fd;
      		
      		
			//connection is now accepted, time to send to connected client 
			//new_fd
			//command read
			data_size=0;
			memset(read_pipe,0,PIPE_MAX);
			data_size = read(new_fd,read_pipe,PIPE_MAX);
			if(data_size <= 0){
				call_death(tmp_out,new_fd,500,"initial read pipe returned <= 0",
				write_pipe,"Server error, unable to read from pipe");
			}
		     
			//fill time for log printing
		    strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
		    
		    
		    //grab string from request
		    start = get_next_string(start, read_pipe, line);
		    if(strlen(line)<=0){
		    	call_death(tmp_out,new_fd,500,"did not read a proper line",
				write_pipe,"Server error, not a valid request");
		    }
		    
		    //parse 1st line  	
		    int n = sscanf(line, "%s %s %s", method, url, http);
		    if(n!=3){
		    	call_death(tmp_out,new_fd,500,"not enough tokens read",
				write_pipe,"Server error, unable to parse request");
		    }
		    
		    
		    //printing request information into log
		    strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
		    fprintf(tmp_out,"%s\n",buf_for_time);
		    fprintf(tmp_out,"\tMethod: %s\n",method);
		    fprintf(tmp_out,"\tHTTP Version: %s\n",http); 
		    
		    //checking for valid method
		    if(!valid_method(method)){
		    	call_death(tmp_out,new_fd,505,"received a method not implemented",
				write_pipe,"Server error, not implemented");
		    }
		    
		    start = get_next_string(start, read_pipe, line);
		    if(strlen(line)<=0){
		    	call_death(tmp_out,new_fd,500,"unable to parse host",
				write_pipe,"Server error, host line read error");
		    }
		    
		    n = sscanf(line, "%s %s", arg,host);
		    if(n!=2){
		    	call_death(tmp_out,new_fd,500,"initial read pipe returned < 0",
				write_pipe,"Server error, not enough tokens in host line");
		    }
		    if(strcmp(arg,"Host:")!=0){
		    	call_death(tmp_out,new_fd,500,"read something other than host",
					write_pipe,"Server error, no host detected");
		    }
		    fprintf(tmp_out,"\tHost: %s\n",host);
		    fprintf(tmp_out,"\tURL: %s\n",url);
		    
		    
		    check_host(host,new_fd,tmp_out,write_pipe);
		    
		    int index = check_secure(host);
		    if(index != -1){
		    	//out_fd = create_secure_socket(index,new_fd,write_pipe,tmp_out);
		    	//write_secure_host(out_fd,new_fd,read_pipe,write_pipe,tmp_out);
		    	secure_and_send(new_fd,read_pipe,write_pipe,index,tmp_out);
		    	break;
		    }
		    out_fd = create_host_socket(host,new_fd,write_pipe,tmp_out);
		    if(out_fd < 0){
		    	call_death(tmp_out,new_fd,500,"unable to build host socket",
				write_pipe,"Server error, unable to connect to host");
		    }
		    /////
		    write_to_host(out_fd,new_fd,read_pipe,write_pipe,tmp_out);
		    close(out_fd);
		    close_all(new_fd,tmp_out,EXIT_SUCCESS);
		  	break;
		}
    	close(new_fd);
     }
      
    return(EXIT_SUCCESS);
}
          
      
