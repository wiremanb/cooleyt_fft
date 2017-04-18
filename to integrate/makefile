all: ctfft

ctfft: main.o fft.o
	g++ main.o fft.o -o ctfft

main.o: main.cpp
	g++ -c main.cpp

fft.o: fft.cpp
	g++ -c fft.cpp

clean:
	rm *o ctfft
