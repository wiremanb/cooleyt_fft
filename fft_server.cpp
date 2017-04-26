#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include "fft.h"
#define BUFSIZE 8
//#define COLUMNS 4096

struct PACKET {
    unsigned char header; //0xAA - New client connection
    //0xBB - Server to client get file
    //0xCC - Client to server return fft value
    //0xDD - Server got fft values and the client can stop
    char name[256];
    float fft;
}__attribute__((packed));

void CREATE_FILE(std::vector<std::string> *fileNames, std::string clientName)
{
    char tmp[sizeof(clientName)];
    sprintf(&tmp[0], "%s_vals.txt", clientName.c_str());
    fileNames->push_back(tmp);
}

void Send_To_Clients(int s, PACKET p, std::vector<sockaddr_storage> c)
{
    strcpy(p.name, "GO");
    for(size_t i=0; i<c.size(); i++)
    {
        p.header = 0xBB;
        sendto(s,&p,sizeof(p),0,(struct sockaddr *)&c.at(i),sizeof(c.at(i)));
    }
}

/*
 * The purpose of this function is to write the servers FFT
 * To a file 'Server_FFT.txt' to be used for comparing between
 * The clients FFT's and the servers. Called already from
 * init_clients_arrays.
 */
void Server_FFT(complex<float>** arr)
{
    ofstream server_file;
    server_file.open("Server_FFT.txt");
    if(server_file.is_open())
    {
        clock_t tStart = clock(); //Run FFT processing loop
        for(int i=0; i<BUFSIZE; i++)
        {
            FFT fft(arr[i], BUFSIZE, 1);
            for(int j=0; j<BUFSIZE; j++)
                server_file << arr[i][j] << endl;
        }
        printf("Server FFT time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    }
    else
    {
        cout << "Server FFT File failed to open\n";
        return;
    }

    //Deallocate array
    server_file.close();
    for (int i = 0; i < BUFSIZE; i++)
    {
        delete [] arr[i];
    }
    delete [] arr;
}

/*
 * The purpose of this function is create two float arrays
 * written to two seperate files to be read by the appropriate
 * client and then call Server_FFT(). This should be called once
 * we have both clients connected
 */
void init_clients_arrays(int s, PACKET p, std::vector<sockaddr_storage> c)
{
    ofstream client1_file, client2_file;
    client1_file.open("Client_1_Servers_Array.txt");
    client2_file.open("Client_2_Servers_Array.txt");

    complex<float>** arr = new complex<float>*[BUFSIZE];
    for(int i = 0; i < BUFSIZE; ++i)
        arr[i] = new complex<float>[BUFSIZE];

    if(client1_file.is_open() && client2_file.is_open())
    {
        for(int i=0; i<BUFSIZE; i++) //Random Assignment of Values into array
        {
            for(int j=0; j<BUFSIZE; j++)
            {
                int temp = static_cast<int>(rand()) / (static_cast <int> (RAND_MAX/100000));
                if(i >= BUFSIZE/2)
                {
                    arr[i][j] = (float)temp;
                    client2_file << arr[i][j] << endl; //Write array to file
                }
                else //Write first half to client 1 file
                {
                    arr[i][j] = (float)temp;
                    client1_file << arr[i][j] << endl; //Write array to file
                }
            }
        }
    }
    else
    {
        cout << "File failed to open\n";
        return;
    }
    client1_file.close();
    client2_file.close();

    // Send message to clients
    Send_To_Clients(s, p, c);

    //Write compute server FFT
    Server_FFT(arr);
}
/*
 * This function is called after we have finished our server FFT
 * and we are ready to read the clients two FFT files and compare
 */
void combine_and_compare_FFTs()
{
    //Server side read 2 client output files, combine them and check with servers FFT
    complex<float> fft_buf[BUFSIZE][BUFSIZE];
    string client_line, server_line;
    std::string cName = "1";
    ifstream infile_server("Server_FFT.txt");
    for(int j = 0; j < 2; j++) //2 for num of clients
    {
        string filename = "Client_" + cName + "_FFT_Out.txt";
        ifstream infile_client(filename.c_str());
        cName = "2"; //Change client_id to 2 to read the second client after run through
        if(infile_client.is_open() && infile_server.is_open())
        {
            while((getline(infile_client, client_line)) && (getline(infile_server, server_line))) //Read file for client and store in buffer
            {
                static int j = 0;
                if(client_line != server_line)
                {
                    cout << "FFT's are not equivalent . . ." << endl;
                    cout << "Terminating program . . ." << endl;
                    return;
                }
                j++;
            }
        }
        else
        {
            cout << "Client FFT Outfile failed to open" << endl;
        }
        infile_client.close();
    }
    infile_server.close();
    cout << "FFT's are equivalent" << endl;
}

int main(int argc, char *argv[])
{
    int udpSocket, nBytes;
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    std::vector<sockaddr_storage> clients;
    std::vector<std::string> fileNames;
    struct PACKET _dataBuf;
    socklen_t addr_size, client_addr_size;

    if(argc < 2)
    {
        printf("Suggest use: ./s <number of clients>");
        return -1;
    }
    int numberOfClients = atoi(argv[1]);
    //    unsigned char clientHeaders[numberOfClients]; // Create client header array size of how many clients will connect
    //    memset(&clientHeaders[0], NULL, numberOfClients);

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Create socket timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
   if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
       perror("Socket timeout error");

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof(serverStorage);

    srand((unsigned int)time(NULL));
    static int clientCount = 0;
    static int done = 0;
    bool sendToClients = true;
    bool doneWithFFTs = false;
    bool quit = false;
    while(!quit)
    {
        nBytes = recvfrom(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage, &addr_size);

        if(clientCount == numberOfClients && clients.size() == numberOfClients && sendToClients == true)
        {
            init_clients_arrays(udpSocket, _dataBuf, clients);
            sendToClients = false;
        }
        else if(nBytes > 0 && _dataBuf.header == 0xAA && clientCount < numberOfClients)
        {
            printf("Client name: %s\n", _dataBuf.name);
            clientCount++;
            //            CREATE_FILE(&fileNames, std::string(_dataBuf.name));
            clients.push_back(serverStorage);
            _dataBuf.header = 0xFF;
            strcpy(_dataBuf.name, "Server");
            sendto(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage,addr_size);
        }


        nBytes = recvfrom(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage, &addr_size);
        if(done == numberOfClients && doneWithFFTs == false)
        {
            doneWithFFTs = true;
            combine_and_compare_FFTs();
            quit = true;
        }
        else if(nBytes > 0 && _dataBuf.header == 0xCC)
        {
            _dataBuf.header = 0xDD;
            done++;
            std::cout << std::string(_dataBuf.name) + " is done with fft!" << std::endl;
            sendto(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage,addr_size);
        }
    }

    return 0;
}


