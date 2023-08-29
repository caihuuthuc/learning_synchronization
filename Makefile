.PHONY: all

all:
	g++ 01_producer_consumer.cpp -o 01_producer_consumer.o
	g++ 02_reader_writer.cpp -o 02_reader_writer.o

clean:
	rm -f *.o