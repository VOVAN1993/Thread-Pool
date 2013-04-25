./bin/main: ./bin/main.o ./bin/Pool.o ./bin/Task.o
	g++ ./bin/main.o ./bin/Pool.o ./bin/Task.o -lboost_thread -o ./bin/main
./bin/main.o: ./src/main.cpp ./src/Pool.h
	g++ -c ./src/main.cpp -o ./bin/main.o
./bin/Pool.o: ./src/Pool.cpp ./src/Pool.h ./src/Task.h
	g++ -c ./src/Pool.cpp -o ./bin/Pool.o
./bin/Task.o: ./src/Task.cpp ./src/Task.h
	g++ -c ./src/Task.cpp -o ./bin/Task.o
clean:	
	rm -rf bin/*.o
