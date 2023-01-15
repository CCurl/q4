q4: q4.o
	cc -oq4 q4.o

q4.o: q4.c
	cc -oq4.o -O3 -DIsLinux q4.c
