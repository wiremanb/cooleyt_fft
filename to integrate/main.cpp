#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <complex>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include "fft.h"
#define BUFSIZE 4
using namespace std;
int main()
{
    ofstream file;
    file.open("1D_FFT_Out.txt");
    
    complex<float>** arr = new complex<float>*[BUFSIZE];
    for(int i = 0; i < BUFSIZE; ++i)
        arr[i] = new complex<float>[BUFSIZE];
        
    clock_t tStart = clock();
    for(int i=0; i<BUFSIZE; i++) //Random Assignment of Values into array
    {
    	for(int j=0; j<BUFSIZE; j++)
    	{
    	    int temp = static_cast<int>(rand()) / (static_cast <int> (RAND_MAX/100000));
    		arr[i][j] = (float)temp;
    		file << "before arr["<<i<<"]["<<j<<"]: "<<arr[i][j]<<endl;
        }
    }
    //FFT loop and print to file
    for(int i=0; i<BUFSIZE; i++)
    {
    	FFT fft(arr[i], BUFSIZE, 1);
       // FFT(arr[i], BUFSIZE, 1);
    	for(int j=0; j<BUFSIZE; j++)
    	{
    		file << "after arr["<<i<<"]["<<j<<"]: "<<arr[i][j]<<endl;
    		//file << arr[i][j] << endl;
        }
    }
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    file.close();
    
    
    //Client Thread side
    string line;
    ifstream infile("1D_FFT_Out.txt");
    if(infile.is_open())
    {
    	complex<float> 
    	while(getline(infile, line))
    	{
    		cout << line << endl;
    	}
    }
    
    return 0;
}

