#pragma once
#include <vector>
#include <unistd.h>
#include <iostream>

static void execute(const std::vector<std::string>& args)
{
    int n = args.size();
    char* argv[n + 1];

    for(int i = 0; i < n; i++) argv[i] = (char*)args[i].c_str();

    argv[n] = NULL;
    execvp(argv[0],argv);
    perror("execvp");
    exit(27);
}

static int execute_pipe(int fd,const std::vector<std::string>& args)
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

static int execute_pipe_line(int input_fd,const std::vector<std::vector<std::string>>& args)
{
    int current_input = input_fd;
    for(int i = 0; i < args.size(); i++)
    {
        current_input = execute_pipe(current_input,args[i]);
    }
    return current_input;
}
