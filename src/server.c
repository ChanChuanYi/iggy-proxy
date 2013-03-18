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


int main(int argc,char** argv){
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd, data_size;
	char read_pipe[PIPE_MAX],write_pipe[PIPE_MAX];
	int break_flag = FALSE;
	int start=0, end = 0;
	time_t now = time(0);
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
    	printf("Parent:%d listening for a new connection\n",(int)getpid());
    	//start the log
    	FILE *tmp_out;
    	tmp_out = fopen("log.txt","a+");//open output file in read-only
    	if(tmp_out == 0)error_print("unable to open log\n");
		
      	listen(sockfd, BACKLOG);
      	// now accept an incoming connection:
      	addr_size = sizeof their_addr;
      
      	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
      	if(new_fd == -1){
        	error_print("unable to accept() a connection");
        	continue;  
      	}
      	printf("Parent:%d New connection has been established\n",(int)getpid());
      
      	int child = fork();
      	if(child == 0){
      		printf("\tPID:%d Fork has been created\n",(int)getpid());
      		close(sockfd);
      		
      		
      		//debug stuff
      		//printf("PID:%d Child close(sockfd)\n",(int)getpid());
      		
      		char filename[LINE];
      		sprintf(filename,"fap/l_debug%d.txt",(int)getpid());
      		
      		FILE *d_out;
    		d_out = fopen(filename,"a+");//open output file
    		if(d_out == 0)error_print("create debug log\n");
      		
      		fprintf(d_out,"PID:%d Child:Fork has been created\n",(int)getpid());
      		//printf("opened %s for debugging\n",filename);
      		
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
				call_death(d_out,new_fd,500,"initial read pipe returned <= 0",
				write_pipe,"Server error, unable to read from pipe");
			}
		     
			//fill time for log printing
		    strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
		    
		    //debug statements 
		    //printf("PID:%d request:\n%s\n",(int)getpid(),read_pipe);
		    fprintf(d_out,"PID:%d request:\n%s\n",(int)getpid(),read_pipe);
		    
		    //grab string from request
		    start = get_next_string(start, read_pipe, line);
		    if(strlen(line)<=0){
		    	call_death(d_out,new_fd,500,"did not read a proper line",
				write_pipe,"Server error, not a valid request");
		    }
		    
		    //parse 1st line  	
		    int n = sscanf(line, "%s %s %s", method, url, http);
		    if(n!=3){
		    	call_death(d_out,new_fd,500,"not enough tokens read",
				write_pipe,"Server error, unable to parse request");
		    }
		    
		    
		    //printing request information into log
		    strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
		    fprintf(tmp_out,"%s\n",buf_for_time);
		    fprintf(tmp_out,"\tMethod: %s\n",method);
		    fprintf(tmp_out,"\tHTTP Version: %s\n",http); 
		    
		    //checking for valid method
		    //printf("PID:%d calling valid_method() with:%s\n",(int)getpid(),method);
		    if(!valid_method(method)){
		    	call_death(d_out,new_fd,505,"received a method not implemented",
				write_pipe,"Server error, not implemented");
		    }
		    
		    start = get_next_string(start, read_pipe, line);
		    if(strlen(line)<=0){
		    	call_death(d_out,new_fd,500,"unable to parse host",
				write_pipe,"Server error, host line read error");
		    }
		    
		    n = sscanf(line, "%s %s", arg,host);
		    if(n!=2){
		    	call_death(d_out,new_fd,500,"initial read pipe returned < 0",
				write_pipe,"Server error, not enough tokens in host line");
		    }
		    if(strcmp(arg,"Host:")!=0){
		    	call_death(d_out,new_fd,500,"read something other than host",
					write_pipe,"Server error, no host detected");
		    }
		    fprintf(tmp_out,"\tHost: %s\n",host);
		    fprintf(tmp_out,"\tURL: %s\n",url);
		    out_fd = create_host_socket(host,new_fd,write_pipe);
		    if(out_fd < 0){
		    	call_death(d_out,new_fd,500,"unable to build host socket",
				write_pipe,"Server error, unable to connect to host");
		    }
		    /////
		    printf("\tPID:%d about to start writing to host\n",(int)getpid());
		    fprintf(d_out,"PID:%d about to start writing to host\n",(int)getpid());
		    /////
		    write_to_host(out_fd,new_fd,read_pipe,write_pipe,d_out);
		  	close(new_fd);
		  	close(out_fd);
		  	//printf("PID:%d closing fork and close(new_fd) close(out_fd)\n",(int)getpid());
		  	fprintf(d_out,"PID%d fork is ending cleanly\n",(int)getpid());
		  	fclose(d_out);
		  	break;
		}
	 	//printf("Parent:%d parent closing new_fd\n",(int)getpid());
    	close(new_fd);
     	fclose(tmp_out); 
     }
      
    return(EXIT_SUCCESS);
}
          
      
