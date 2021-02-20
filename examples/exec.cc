#include <iostream>
#include <exec/exec.h>
#include <fd/fd.h>
using namespace std;

int main(int argc, char *argv[])
{
    int fd = execute_pipe(execute_pipe(0,{"cat","/proc/cpuinfo"}),{"grep","MHz"});
    int fd2 = execute_pipe_line(0,{
                {"cat","/proc/cpuinfo"},{"grep","MHz"},{"grep","200"}
            });
    cpy(fd,1);
    cpy(fd2,1);
}
