CC=g++
CFLAGS=-I.



main: main.o CRoom.o
	$(CC) -o $@ $^ $(CFLAGS)

main.o: main.cpp CRoom.h
	$(CC) -c -o $@ $< $(CFLAGS)

CRoom.o: CRoom.cpp CRoom.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm *.o 

