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

void send_error(char* write_pipe, int err, char* err_msg){
	memset(write_pipe,0,PIPE_MAX);
	set_headers(write_pipe,err,err_msg,TRUE);
	sprintf(write_pipe,"\
		<!DOCTYPE html>\n\
		<html lang=en>\n\
		<meta charset=utf-8>\n\
		<title>%d %s</title>\n\
		<p><b>%d</b> <ins>%s</ins>\n",
		err,err_msg,err,err_msg);
	sprintf(write_pipe,"\r\n");
	printf("to browser msg:\n%s\n",write_pipe);
} 

void set_headers(char* write_pipe, int status, char* status_msg,int close_bool){
	time_t now;
	char buf_for_time[100];
	
	sprintf(write_pipe,"%s %d %s\r\n","HTTP/1.1",status,status_msg);
	sprintf(write_pipe,"Server: http://localhost\r\n");
	strftime(buf_for_time,sizeof(buf_for_time),DATE_FORMAT,gmtime(&now));
	sprintf(write_pipe,"Date: %s\r\n",buf_for_time);	
	if(close_bool == TRUE){
		sprintf(write_pipe,"Connection: close\n");
	}
	sprintf(write_pipe,"Content-Type: text/html; charset=UTF-8\r\n");
	sprintf(write_pipe,"\r\n");
}




  
  
  
