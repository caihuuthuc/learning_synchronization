.PHONY: all

all:
	g++ 01_producer_consumer.cpp -o 01_producer_consumer.o

clean:
	rm -f *.o