all: main
main: main.o Task.o
	g++ main.o Task.o -lboost_thread -o main        
main.o: main.cpp Task.h 
	g++ -c main.cpp 
Task.o: Task.h Task.cpp
	g++ -c Task.cpp 
clean:
	rm -rf *.o ./main
