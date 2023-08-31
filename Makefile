.PHONY: all

all: 01_producer_consumer 02_reader_writer 03_five_phisolopher 04_agent_smoker

01_producer_consumer:
	g++ 01_producer_consumer.cpp -o 01_producer_consumer.o

02_reader_writer:
	g++ 02_reader_writer.cpp -o 02_reader_writer.o
	g++ 02_reader_writer_no_starving_writer.cpp -o 02_reader_writer_no_starving_writer.o
	g++ 02_reader_writer_writer-priority.cpp -o 02_reader_writer_writer-priority.o

03_five_phisolopher:
	g++ 03_five_phisolopher.cpp -o 03_five_phisolopher.o

04_agent_smoker:
	g++ 04_agent_smoker.cpp -o 04_agent_smoker.o

clean:
	rm -f *.o