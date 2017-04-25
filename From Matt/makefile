all: s c

c: fft_client.o fft.o
	g++ -lpthread fft_client.o fft.o -o c

s: fft_server.o fft.o
	g++ fft_server.o fft.o -o s

fft_client.o: fft_client.cpp
	g++ -c fft_client.cpp

fft_server.o: fft_server.cpp
	g++ -c fft_server.cpp

fft.o: fft.cpp
	g++ -c fft.cpp

clean:
	rm *o s c *.txt
