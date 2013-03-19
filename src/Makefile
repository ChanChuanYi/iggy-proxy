server: server.o extra.o iggyssl.o
	gcc -o server server.o extra.o iggyssl.o -lcrypto -lssl
	clear ; rm *.o ; rm *.txt ; rm fap/*

server.o:server.c
	gcc -g -fno-stack-protector -c server.c

extra.o:extra.c
	gcc -g -c extra.c 
	
iggyssl.o:iggyssl.c
	gcc -g -c iggyssl.c -lcrypto -lssl

clean:
	clear ; rm *.o ; rm *.txt ; rm fap/*
