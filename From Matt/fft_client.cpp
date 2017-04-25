#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <complex>
#include <stdlib.h>
#include <sstream>
#include "fft.h"
#define BUFSIZE 8
#define NUM_THREADS 2

bool debug = false;

struct PACKET {
    unsigned char header; //0xAA - New client connection
    //0xBB - Server to client get file
    //0xCC - Client to server return fft value
    char name[256];
    float fft;
}__attribute__((packed));

struct thread_info
{
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int thread_num;       /* Application-defined thread # */
    string filename;
    string name;
    complex<float> arr[BUFSIZE/NUM_THREADS][BUFSIZE];
};

void *pthreadFunction(void *arg)
{
    thread_info *threadinfo = (struct thread_info*)arg;
    std::stringstream sstm;
    sstm << threadinfo->thread_num;
    string thread_num = sstm.str();
    threadinfo->filename += "Client_" + threadinfo->name + "_Thread" + thread_num + ".txt";
    ofstream outfile(threadinfo->filename.c_str());
    if(outfile.is_open())
    {
        for(int i=0; i<(BUFSIZE/2)/NUM_THREADS; i++)
        {
            FFT fft(threadinfo->arr[i], BUFSIZE, 1);
            for(int j=0; j<BUFSIZE; j++)
            {
                outfile << threadinfo->arr[i][j] << endl; //Write FFT to its file
            }
        }
    }
    else
    {
        cout << "Failed to open file\n";
    }

    pthread_exit(NULL);
}

/*
 * This function is a subfunction called after the threads
 * are complete and we have our thread subfiles ready to be
 * combined into one subfile for the server to read and compare
 * with
 */
void merge_thread_files(std::string cName, thread_info tinfo[NUM_THREADS])
{
    //Client side reading from thread subfiles and making one file for server to read
    //    complex<float> buf[BUFSIZE/2][BUFSIZE];
    string line_buf;
    string outfilename = "Client_" + cName + "_FFT_Out.txt";
    ofstream outfile(outfilename.c_str());
    for(int j = 0; j < NUM_THREADS; j++)
    {
        std::stringstream sstm;
        sstm << tinfo[j].thread_num;
        string thread_num = sstm.str();
        string filename = "Client_"+ cName + "_Thread"+ thread_num + ".txt";

        ifstream infile(filename.c_str());
        if(infile.is_open())
        {
            while(getline(infile, line_buf)) //Read file for client and store in buffer
            {
                outfile << line_buf << endl;
            }
        }
        else
        {
            cout << "Client Thread File failed to open" << endl;
            return;
        }
        infile.close();
    }
    outfile.close();
}


/*
 * This is a subfunction called from read_array_file_and_fft
 * to perform the multithreaded fft
 */

void threaded_fft(complex<float> client_array[BUFSIZE/2][BUFSIZE], std::string cName)
{
    //Thread Portion to divide the array into sub arrays for threads and start FFTs in threads
    thread_info tinfo[NUM_THREADS];
    int h = 0;
    for(int k = 0; k < NUM_THREADS; k++)
    {
        //i < (BUFSIZE/2)/NUM_THREADS because we want half the array for one client
        //and then that divided into the num of threads
        for(int i = 0; i < (BUFSIZE/2)/NUM_THREADS; i++)
        {
            for(int j = 0; j < BUFSIZE; j++)
            {
                tinfo[k].arr[i][j] = client_array[h][j];
            }
            h++;
        }


        tinfo[k].thread_num = k+1;
        tinfo[k].name = cName;
        pthread_create(&tinfo[k].thread_id, NULL, &pthreadFunction, &tinfo[k]);
    }

    for(int h = 0; h < NUM_THREADS; h++)
        pthread_join(tinfo[h].thread_id, NULL);

    merge_thread_files(cName, tinfo);
}

/*
 * This function is to be called once we have received a
 * message from the server that our array file is ready to
 * be read and processed
 */
void read_array_file_and_fft(std::string cName)
{
    if(debug)
        std::cout << "read_array_file_and_fft 1" << std::endl << std::flush;
    //Client Side Reading from file and storing to buffer

    /*complex<float>** client_array = new complex<float>*[BUFSIZE/2]; // array: [2048] x [4096]
        for(int i = 0; i < BUFSIZE; ++i)
                client_array[i] = new complex<float>[BUFSIZE]; //Init client side array to prepare read*/

    complex<float> client_array[BUFSIZE/2][BUFSIZE];
    //CHECK FOR CLIENT 1 or 2 and then assign the  client_id to the threads
    string line;
    string filename = "Client_" + cName + "_Servers_Array.txt";
    ifstream infile(filename.c_str());
    if(infile.is_open())
    {
        if(debug)
            std::cout << "read_array_file_and_fft 2" << std::endl << std::flush;
        int i = 0, j = 0;
        while(getline(infile, line)) //Read file for client and store in buffer
        {
            if(debug)
                std::cout << "read_array_file_and_fft 3" << std::endl << std::flush;
            int o_paren = line.find("(");
            int comma = line.find(",");
            line = line.substr(o_paren+1, comma-1);
            int temp = atoi(line.c_str());
            if(j == BUFSIZE)
            {
                if(debug)
                    std::cout << "read_array_file_and_fft 4" << std::endl << std::flush;
                j = 0;
                i++;
            }
            client_array[i][j] = (float)temp;
            if(debug)
                std::cout << "read_array_file_and_fft 5" << std::endl << std::flush;
            //            std::cout << client_array[i][j] << std::endl << std::flush;
            j++;

        }
    }
    else
    {
        if(debug)
            std::cout << "read_array_file_and_fft ELSE" << std::endl << std::flush;
        cout << "Client Servers Array File Failed to open file\n";
    }
    infile.close();
    threaded_fft(client_array, cName);
}

int main(int argc, char *argv[])
{
    int clientSocket, portNum, nBytes;
    struct sockaddr_in serverAddr;
    struct PACKET _dataBuf;
    socklen_t addr_size;

    if(argc < 2)
    {
        std::cout << "Suggested use: ./c <client name>" << std::endl;
        return -1;
    }
    else
        strcpy(_dataBuf.name, argv[1]);

    /*Create UDP socket*/
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Create socket timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
        perror("Socket timeout error");

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverAddr;
    _dataBuf.header = 0xAA;
    bool calcFFT = false;
    bool quit = false;
    static int firstTime = 0;
    while(1)
    {
        sendto(clientSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverAddr,addr_size);

        /*Receive message from server*/
        nBytes = recvfrom(clientSocket,&_dataBuf,sizeof(_dataBuf),0,NULL, NULL);

        if(nBytes > 0 && _dataBuf.header == 0xBB && strcmp(_dataBuf.name, "GO")==0)
        {
            calcFFT = true;
        }
        else
            calcFFT = false;

        if(calcFFT == true)
        {
            std::cout << "Server msg: " << _dataBuf.name << std::endl << std::flush;
            read_array_file_and_fft(std::string(argv[1]));
            strcpy(_dataBuf.name, argv[1]);
            _dataBuf.header = 0xCC;
            calcFFT = false;
            sendto(clientSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverAddr,addr_size);
        }

        if(nBytes > 0 && _dataBuf.header == 0xDD && strcmp(_dataBuf.name, argv[1])==0)
            return 0;

    }

    return 0;
}
