

all: golf-club


golf-club: main.cpp v4ldevice.cpp
	$(CXX) -I /usr/local/include/opencv2 -Wall -g -O0 -o golf-club main.cpp v4ldevice.cpp $(shell pkg-config opencv --libs libv4l2)

clean:
	$(RM) *.o
	$(RM) golf-club
