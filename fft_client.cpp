#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define COLUMNS 4096

struct PACKET {
    unsigned char header; //0xAA - New client connection
                          //0xBB - Server to client get file
                          //0xCC - Client to server return fft value
    char name[256];
    float fft;
}__attribute__((packed));

void FFT()
{

}

int main(int argc, char *argv[])
{
    int clientSocket, portNum, nBytes;
    struct sockaddr_in serverAddr;
    struct PACKET _dataBuf;
    socklen_t addr_size;

    if(argc < 2)
    {
        std::cout << "Suggested use: ./s <client name>" << std::endl;
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
    while(1)
    {
        sendto(clientSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverAddr,addr_size);

        /*Receive message from server*/
        nBytes = recvfrom(clientSocket,&_dataBuf,sizeof(_dataBuf),0,NULL, NULL);
        if(nBytes > 0 && _dataBuf.header == 0xBB)
        {
            std::cout << "fft" << std::endl << std::flush;
            FFT();
        }


    }

    return 0;
}
