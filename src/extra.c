#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include "extra.h"

#define F_GETPATH	(1024 + 7)

char forbidden[1024][1024];
char safe_host[1024][1024];
char safe_url[1024][1024];
char safe_port[1024][1024];

void error_print(char* message){
	fprintf(stderr,"\tPID:%d error:%s\n",(int)getpid(),message);
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
",err,err_msg,err,err_msg);
	strcat(write_pipe,buff);
	int data_size = write(new_fd,write_pipe,PIPE_MAX);
	if(data_size < 0 ){
		error_print("unable to send to client");
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
	int char_count = 0, end = 0;
	for(end = start;;end++){
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
	return start;
	
}

int valid_method(char* method){
	if(strcmp(method,"GET")==0)return TRUE;
	if(strcmp(method,"HEAD")==0)return TRUE;
	return FALSE;
}

int create_host_socket(char* host,int client_fd,char* write_pipe,FILE* d_out){
	int sockfd, data_size,rv;
	char* port = "80";
	struct addrinfo hints, *servinfo;
	  
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	//from beej.us online pdf document
	if( (rv = getaddrinfo(host,port,&hints,&servinfo)) !=0){
		fprintf(stderr,"PID:%d host:%s port:%s addrinfo:%s\n",
			(int)getpid(),host,port,gai_strerror(rv));
		return rv;
	}
	  
	  
	sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
	if(sockfd == -1) error_print("socket build error");
	if(connect(sockfd,servinfo->ai_addr,servinfo->ai_addrlen) == -1){
		error_print("connect fail");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(servinfo);
	return sockfd;
}

char* write_to_host(int out_fd,int new_fd,char* read_pipe,char* write_pipe,FILE* d_out){
	int host_data_out,host_data_in, client_data;
	signal(SIGALRM,sig_handle);
    alarm(10);
	
	host_data_out = write(out_fd, read_pipe,PIPE_MAX);
	if(host_data_out <0){
		call_death(d_out,new_fd,500,"pipe error, unable to write to host",
			write_pipe,"Server error- unable to write to host");
	}
	do{
	memset(write_pipe,0,PIPE_MAX);
	host_data_in = read(out_fd, write_pipe, PIPE_MAX);
	if(host_data_in < 0){
		call_death(d_out,new_fd,500,"pipe error, probably done reading",
			write_pipe,"Server error-pipe error occured");
	}
	client_data = write(new_fd,write_pipe,host_data_in);
	if(client_data < 0){
		call_death(d_out,new_fd,500,"client-data pipe error occured",
			write_pipe,"Server error-pipe error occured writing back to client");
	}
	}while(host_data_in > 0);
}

int close_is_true(char* write_pipe){
	int start;
	char buf[LINE];
	while(start < PIPE_MAX){
		start = get_next_string(start, write_pipe, buf);
		if(strlen(buf)==0)return FALSE;
		if(strcmp(buf,"Connection: close")==0)return TRUE;
	}
}

void call_death(FILE* d_out,int fd,int err,char* err_msg,char* write_pipe,char* req_err){
	send_error(fd,write_pipe,err,req_err);
	close_all(fd,d_out,EXIT_FAILURE);	
}

void sig_handle(int sig){
	kill(getpid(),SIGINT);
}

void load_site_files(){
	memset(forbidden,0,1024);
	memset(safe_host,0,1024);
	memset(safe_url,0,1024);
	memset(safe_port,0,1024);
	char line[LINE];
	
	//start the log
    FILE *tmp_out;
    tmp_out = fopen("log.txt","a+");//open output file in read-only
    if(tmp_out == 0)error_print("unable to open log\n");
    fclose(tmp_out);

	//load forbidden sties
	FILE* forbidden_fp;
	forbidden_fp = fopen("forbidden-sites","r");
	if(forbidden_fp > 0){
		int x=0;
		while (fgets(line, LINE, forbidden_fp) != NULL) {
  			if (sscanf(line, "%s\n",forbidden[x]) == 0)break;
  			x++;
		}
		fclose(forbidden_fp);
	}
	
	//load secure sites
	FILE* secure_fp;
	secure_fp = fopen("secure-sites","r");
	if(secure_fp > 0){
		int x=0;
		while (fgets(line, LINE, secure_fp) != NULL) {
  			if (sscanf(line, "%s %s %s",
  				safe_host[x],safe_url[x],safe_port[x]) != 3)break;
		}
		fclose(secure_fp);
	}
	
	
}

void check_host(char* host, int client_fd, FILE* d_out,char* write_pipe){
	int x=0;
	for(;strcmp(forbidden[x],"")!=0;x++){
		if(strcmp(forbidden[x],host)==0)
			call_death(d_out,client_fd,403,"Forbidden",
			write_pipe,"Forbidden");
	}
}

int check_secure(char* _host){
	char host[LINE];
	int x=0;
	sprintf(host,"%s/",_host);
	for(;strcmp(safe_host[x],"")!=0;x++){
		if(strcmp(safe_host[x],host)==0)return x;
	}
	return -1;
}
  
void close_all(int client_fd, FILE* d_out,int exit_status){
	close(client_fd);
	fclose(d_out);
	system("less p_log_*.txt > log.txt 2>/dev/null");
	system("rm p_log_* 2>/dev/null");
	exit(exit_status);
}
  
  
