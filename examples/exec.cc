#include <iostream>
#include <exec/exec.h>
#include <fd/fd.h>
using namespace std;

int main(int argc, char *argv[])
{
    int fd = execute_pipe(execute_pipe(0,{"cat","/proc/cpuinfo"}),{"grep","MHz"});
    cpy(fd,1);
}
