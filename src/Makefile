server: server.o extra.o
	gcc -o server server.o extra.o
	clear ; rm *.o ; rm *.txt ; rm fap/*

server.o:server.c
	gcc -g -fno-stack-protector -c server.c

extra.o:extra.c
	gcc -g -c extra.c

clean:
	clear ; rm *.o ; rm *.txt ; rm fap/*
