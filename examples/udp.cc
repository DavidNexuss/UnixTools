#include <iostream>
#include "udp.h"
#include "crypt.h"
using namespace std;

void test_udp()
{
    string ip = "127.0.0.1";
    int port = 8080;
    
    if (fork() == 0)
    {
        UDP_FD client(ip,port,false);
        cpy(0,client);
        perror("cpy");
        exit(1);
    }
    else
    {
        UDP_FD server(ip,port,true);
        cpy(server,1);
        perror("cpy");
        exit(1);
    }
}
int main()
{
    unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
    /*
    int enc = aes_crypt(0,key,true);
    int dec = aes_crypt(enc,key,false);

    cpy(dec,1);
    */
    const int blockSize = 4096;
    int f = fork();
    if (f == 0)
    {
        int input;
        UDP_FD client;
        if (input = create_udp_server(UDP_FD("127.0.0.1",8080,true),client,blockSize))
        {
            cerr << "Created channel " << input << endl;
            cerr << "With client " << NetworkIdentifier(client) << endl; 
            cpy(input,1);
        }
        perror("create_udp_server_main");
        return 1;
    }


    UDP_FD client("127.0.0.1",8080,false);
    cpy(0,client);
}
