all: main
main: main.o Task.o
	g++ main.o Task.o -lboost_thread -o main
main.o: ./src/main.cpp ./src/Task.h
	g++ -c ./src/main.cpp
Task.o: ./src/Task.h ./src/Task.cpp
	g++ -c ./src/Task.cpp
clean:
	rm -rf *.o ./main
