#include "util.h"
#include <sstream>
#include <unistd.h>
#include <iostream>

vector<string> split(const string& line)
{
    stringstream ss(line);
    vector<string> res;
    string word;
    while(ss >> word) res.push_back(word);
    return res;
}
void execute(const vector<string>& args)
{
    int n = args.size();
    char* argv[n + 1];

    for(int i = 0; i < n; i++) argv[i] = (char*)args[i].c_str();

    argv[n] = NULL;
    execvp(argv[0],argv);
    perror("execvp");
    exit(27);
}

int execute_pipe(int fd,const vector<string>& args)
{
    int pipe_fd[2];
    pipe(pipe_fd);

    int f = fork();
    if (f == 0)
    {
        if (fd != 0)
        {
            dup2(fd,0);
            close(fd);
        }

        dup2(pipe_fd[1],1);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execute(args);
    }

    //close(fd) Hauria de funcionar 
    close(pipe_fd[1]);
    return pipe_fd[0];
}
void send_notification(const string& title,const string& message)
{
    int f = fork();
    if (f > 0) return;
    execute({"notify-send","-i","starred",title,message});
}

void set_signal_handler(int signal,struct sigaction* sa)
{
    if (sigaction(signal,sa,0) == -1)
    {
        perror("sigaction");
        exit(1);
    }
}
