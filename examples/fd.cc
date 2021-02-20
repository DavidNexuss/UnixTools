#include <fd/fd.h>
#include <string>
#include <iostream>
using namespace std;


void streamFD()
{
    FD file = 0; //Assign stdin
    
    //Caution
    //Reading from FD is done using read and write sycalls, no buffering and no serializing
    //Reading a number for example will be done in internal format (binary)
    //Useful for writing non ascii data for more efficient io
    
    int a,b,c;
    char buffer[64];
    while (file >> a >> b >> c >> buffer)
    {
        cout << a << endl;
    }
    cerr << "EOF" << endl;
}
void fileFD()
{
    FD file = open("someFile",O_RDONLY);
    
    lseek(file,26,SEEK_SET); //Regular syscalls work out of the box

    //close(file) executed automatically
}
int main()
{
    streamFD();
}