
CXX = g++ -O3 -std=c++11

INC = -I./src/
LIB = -lpthread

SRCS = 
OBJS = 

TARGETS = demo.bin \
		  demo2.bin \
		  demo3.bin \
		  test_queue.bin \
		  test_threadpool.bin 

.PHONY: default clean

default: $(TARGETS)

%.bin : examples/%.cpp $(OBJS)
	$(CXX) $(INC) $<  -o $@ $(LIB) $(OBJS)

%.o : %.cpp
	$(CXX) $(INC) -c $<

clean:
	rm -rf $(OBJS)
	rm -rf $(TARGETS)