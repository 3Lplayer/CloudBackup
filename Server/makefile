.PHONY:server
server:server.cpp
	g++ -o $@ $^ -lpthread -lstdc++fs -ljsoncpp -L. -lbundle -std=c++17
.PHONY:clean
clean:
	rm -rf server
