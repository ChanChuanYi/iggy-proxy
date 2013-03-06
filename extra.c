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
#include <sys/time.h>
#include "extra.h"

void error_print(char* message){
	fprintf(stderr,"Error:%s\n",message);
	exit(EXIT_FAILURE);
}

void send_error(int new_fd,char* write_pipe, int err, char* err_msg){
	memset(write_pipe,0,PIPE_MAX);
	char buff[PIPE_MAX];
	memset(buff,0,PIPE_MAX);
	
	set_headers(write_pipe,err,err_msg,TRUE);
	sprintf(buff,"\
<!DOCTYPE html>\n\
<html lang=en>\n\
<meta charset=utf-8>\n\
<title>%d %s</title>\n\
<p><b>%d</b> <ins>%s</ins>\n\
</html>\
",
		err,err_msg,err,err_msg);
	strcat(write_pipe,buff);
	int data_size = write(new_fd,write_pipe,PIPE_MAX);
		     if(data_size < 0 ){
		     	error_print("unable to transmit to client");
		     }
} 

void set_headers(char* write_pipe, int status, char* status_msg,int close_bool){
	time_t now = time(0);
	char buf_for_time[100];
	char buff[PIPE_MAX];
	memset(buff,0,PIPE_MAX);
	
	sprintf(buff,"%s %d %s\r\n","HTTP/1.0",status,status_msg);
	strcat(write_pipe,buff);
	strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
	sprintf(buff,"Date: %s\r\n",buf_for_time);
	strcat(write_pipe,buff);	
	if(close_bool == TRUE){
		sprintf(buff,"Connection: close\n");
		strcat(write_pipe,buff);
	}
	sprintf(buff,"Server: http://localhost\r\n");
	strcat(write_pipe,buff);
	sprintf(buff,"Content-Type: text/html; charset=UTF-8\r\n");
	strcat(write_pipe,buff);
	sprintf(buff,"Content-Length: 200\r\n");
	strcat(write_pipe,buff);
	sprintf(buff,"\r\n");
	strcat(write_pipe,buff);
}

int get_next_string(int start, char* search_buf, char* ret_buf){
	//printf("get_next_string search_buf:\n%s\n",search_buf);
	int char_count = 0, end = 0;
	for(end = start;end<PIPE_MAX;end++){
		if(search_buf[end] == '\0') break;
		if(search_buf[end] == '\r'){
			ret_buf[char_count]='\0';
			continue;
		}
		if(search_buf[end] == '\n'){
			ret_buf[char_count] = '\0';
			break;
		}
		ret_buf[char_count] = search_buf[end];
		char_count++;
	}
	
	start = ++end;
	//DEBUB PRINT STATEMENT
	//printf("new start:%d, next char:%c string:\n%s\n",start,search_buf[start],ret_buf);
	return start;
	
}

int valid_method(char* method){
	if(strcmp(method,"GET")==0)return TRUE;
	if(strcmp(method,"HEAD")==0)return TRUE;
	return FALSE;
}

int create_host_socket(char* host){
	int sockfd, data_size,rv;
	char* port = "80";
	struct addrinfo hints, *servinfo;
	  
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	 
	
	  
	  
	//from beej.us online pdf document
	if( (rv = getaddrinfo(host,port,&hints,&servinfo)) !=0){
		fprintf(stderr,"addrinfo:%s\n",gai_strerror(rv));
		return EXIT_FAILURE;
	}
	  
	  
	sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
	if(sockfd == -1) error_print("socket build error");
	if(connect(sockfd,servinfo->ai_addr,servinfo->ai_addrlen) == -1){
		close(sockfd);
		error_print("connect fail");
	}
	freeaddrinfo(servinfo);
	return sockfd;
}

void write_to_host(int out_fd,int new_fd,char* read_pipe,char* write_pipe){
	int write_data;
	int data_read = write(out_fd,read_pipe,PIPE_MAX);
	if(data_read < 0){
		send_error(new_fd,write_pipe,500,"server error");
		error_print("Unable to write to host");
	}
	
	
	while(data_read != 0){
		memset(read_pipe,0,PIPE_MAX);
		data_read = read(out_fd,read_pipe,PIPE_MAX);
		if(data_read < 0){
			send_error(new_fd,write_pipe,500,"server error");
			error_print("Unable to receive from host");
		}
		printf("server response:\n%s\n",read_pipe);
	
		write_data = write(new_fd,read_pipe,PIPE_MAX);
		if(write_data < 0){
			//send_error(new_fd,write_pipe,500,"server error");
			error_print("Unable to respond to host");
		}
	}
}


  
  
  
