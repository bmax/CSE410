all: proj1.o receiver.o
	g++ -g -o proj1 proj1.o 

proj1.o: proj1.cpp proj1.h
	g++ -Wall -c proj1.cpp
receiver.o: receiver.cpp
	g++ -Wall -o receiver receiver.cpp
clean:
	rm -rf proj1 *.o

