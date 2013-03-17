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
    
    //start the log
    FILE *tmp_out;
    tmp_out = fopen("log.txt","a+");//open output file in read-only
    if(tmp_out == 0)error_print("unable to open log\n");
    
    
    while(TRUE){
		printf("PID:%d listening for a new connection\n",(int)getpid());
      	listen(sockfd, BACKLOG);
      	// now accept an incoming connection:
      	addr_size = sizeof their_addr;
      
      	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
      	if(new_fd == -1){
        	fprintf(stderr,"unable to  on accept establish connection\n");
        	continue;  
      	}
      	printf("PID:%d New connection has been established\n",(int)getpid());
      
      	int child = fork();
      	printf("fork ran\n");
      	if(child == 0){
      		printf("PID:%d Fork has been created\n",(int)getpid());
      		//debug stuff
      		char filename[LINE];
      		sprintf(filename,"fap/l_debug%d.txt",(int)getpid());
      		FILE *d_out;
    		d_out = fopen(filename,"a+");//open output file in read-only
    		if(d_out == 0)error_print("create debug log\n");
      		fprintf(d_out,"PID:%d Child:Fork has been created\n",(int)getpid());
      	
      		//child variables
      		char method[LINE];
      		char url[LINE];
      		char http[LINE];
      		char arg[LINE];
      		char host[LINE];
      		char buf_for_time[LINE];
      		int out_fd;
      		close(sockfd);
      		printf("PID:%d Child: close(sockfd)\n",(int)getpid());
			//connection is now accepted, time to send to connected client 
			//new_fd
			//command read
			data_size=0;
			memset(read_pipe,0,PIPE_MAX);
			data_size = read(new_fd,read_pipe,PIPE_MAX);
			if(data_size < 0){
				call_death(d_out,new_fd,"initial read pipe returned < 0");
			}
		     
			if( (read_pipe == (char*) 0) || (data_size == 0)){
				fprintf(d_out,"PID:%d received a bad request\n",(int)getpid());
				send_error(new_fd,write_pipe,404,"Bad Request");
		    	close(new_fd);
		    	break;
			}
		    strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
		    printf("PID:%d request:\n%s\n",(int)getpid(),read_pipe);
		    fprintf(d_out,"PID:%d request:\n%s\n",(int)getpid(),read_pipe);
		     
		    start = get_next_string(start, read_pipe, line);
		    if(strlen(line)<=0){
		    	fprintf(d_out,"PID:%d get_next_string read less than 0\n",(int)getpid());
		    	send_error(new_fd,write_pipe,404,"Bad Request");
		    	close(new_fd);
		    	break;
		    }
		     	
		    int n = sscanf(line, "%s %s %s", method, url, http);
		    if(n!=3){
		    	fprintf(d_out,"PID:%d bad request, bad form\n%s\n",(int)getpid(),line);
		    	send_error(new_fd,write_pipe,404,"Bad Request");
		    	break;
		    }
		    
		    
		    //printing request information into log
		    strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
		    fprintf(tmp_out,"%s\n",buf_for_time);
		    fprintf(tmp_out,"\tMethod: %s\n",method);
		    fprintf(tmp_out,"\tHTTP Version: %s\n",http); 
		    
		    //checking for valid method
		    if(!valid_method(method)){
		    	fprintf(d_out,"PID:%d bad method detected:\n%s\n",(int)getpid(),method);
		    	send_error(new_fd,write_pipe,501,"Not Implemented");
		    	close(new_fd);
		    	break;
		    }
		    
		    start = get_next_string(start, read_pipe, line);
		    if(strlen(line)<=0){
		    	fprintf(d_out,"PID:%d http read error:\n%s\n",(int)getpid(),line);
		    	send_error(new_fd,write_pipe,404,"Bad Request");
		    	close(new_fd);
		    	break;
		    }
		    
		    n = sscanf(line, "%s %s", arg,host);
		    if(n!=2){
		    	send_error(new_fd,write_pipe,404,"Bad Request - No host");
		    	fprintf(d_out,"PID:%d did not recognize host\n",(int)getpid());
		    }
		    if(strcmp(arg,"Host:")!=0){
		    	send_error(new_fd,write_pipe,404,"Bad Request - No host");
		    	fprintf(d_out,"PID:%d did not recognize host\n",(int)getpid());
		    }
		    fprintf(tmp_out,"\tHost: %s\n",host);
		    fprintf(tmp_out,"\tURL: %s\n",url);
		    out_fd = create_host_socket(host);
		    write_to_host(out_fd,new_fd,read_pipe,write_pipe,d_out);
		  	close(new_fd);
		  	close(out_fd);
		  	printf("PID:%d closing fork and close(new_fd) close(out_fd)\n",(int)getpid());
		  	fprintf(d_out,"PID%d fork is ending cleanly\n",(int)getpid());
		  	fclose(d_out);
		  	break;
		}
	 printf("parent closing new_fd\n");
     close(new_fd);
     }
    fclose(tmp_out);   
    return(EXIT_SUCCESS);
}
          
      
