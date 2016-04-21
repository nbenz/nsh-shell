client: defs.h client.c
	gcc -o client client.c

server: defs.h server.o shell.o getch.o
	gcc -lm -o server server.o shell.o getch.o

server.o: server.c
	gcc -c server.c

shell.o: shell.c
	gcc -c shell.c

getch.o: getch.c
	gcc -c getch.c

debugs:
	gcc -g -o d_server server.c

debugc:
	gcc -g -o d_client client.c

clean: 
	rm -f server d_server client d_client server.o client.o
