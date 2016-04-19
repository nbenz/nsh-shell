nsh : defs.h main.o getch.o 
	gcc -lm -o nsh main.o getch.o

main.o : main.c defs.h 
	gcc -c main.c

getch.o : getch.c
	gcc -c getch.c

debug: defs.h main.c getch.c
	gcc -g -o d_nsh defs.h main.c getch.c

clean:
	rm -f main.o getch.o nsh d_nsh
