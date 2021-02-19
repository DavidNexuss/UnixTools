#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
template <typename FDA,typename FDB>
ssize_t cpy(FDA src,FDB dst)
#else
ssize_t cpy(int src,int dst)
#endif
{
    char buffer[1024];
    ssize_t n;
    while((n = read(src,buffer,1024)) > 0)
    {
        write(dst,buffer,n);
    }
    if (n < 0) perror("cpy");
    return n;
}
#ifdef __cplusplus
template <typename VFD>
ssize_t read_buffer(VFD fd,uint8_t* buffer,ssize_t maxsize,ssize_t minsize = -1)
#else
ssize_t read_buffer(int fd,unsigned char* buffer,ssize_t maxsize,ssize_t minsize)
#endif
{
    if (minsize == -1) minsize = maxsize;

    ssize_t current_size = 0;
    while(current_size < minsize)
    {
        ssize_t n = read(fd,buffer + current_size,maxsize - current_size);
        if (n <= 0) return current_size >= minsize ? current_size : 0;
        current_size += n;

    }
    return current_size;
}


#ifdef __cplusplus
#include <memory>
struct FD
{
    struct FDHandler
    {
        int fd = -1;
        FDHandler() { }
        FDHandler(int _fd) : fd(_fd) { }
        ~FDHandler()
        {
            close(fd);
        }
    };

    std::shared_ptr<FDHandler> fd;
    inline FD() { }
    inline FD(int _fd)
    {
        fd = std::make_shared<FDHandler>(_fd);
    }
    inline FD(const FD& other) {
        fd = other.fd;
    }
    
    inline FD& operator=(int _fd) { 
        fd = std::make_shared<FDHandler>(_fd);
        return *this;
    }

    operator int() const { return fd->fd; }
};

inline void pipe(FD* pipe_fd)
{
    int p_pipe_fd[2];
    pipe(p_pipe_fd);
    pipe_fd[0] = p_pipe_fd[0];
    pipe_fd[1] = p_pipe_fd[1];
}
#endif
