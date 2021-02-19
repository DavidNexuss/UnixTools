#pragma once
#include <fd/fd.h>
#include <net/net.h>

class UDP_FD
{
    struct sockaddr_in client_info;
    FD socket_fd;
    bool errored = false;

    void bind_client_info()
    {
        int status = bind(socket_fd,(struct sockaddr*)&client_info,sizeof(sockaddr_in));
        if (status != 0)
        {
            perror("Error binding client_info");
            errored = true;
        }
    }
    public:

    UDP_FD() {} 
    UDP_FD(const std::string& host,int port,bool server) : socket_fd(socket(AF_INET,SOCK_DGRAM,0))
    {
        client_info = { 0 };
        client_info.sin_family = AF_INET;
        client_info.sin_port = 0;
        client_info.sin_addr.s_addr = INADDR_ANY;

        if (!server) bind_client_info();

        client_info.sin_port = htons(port);
        inet_aton(host.c_str(), &client_info.sin_addr);

        if (server) bind_client_info();
    }

    struct sockaddr_in* get_client_info() { return &client_info; }
    const struct sockaddr_in* get_client_info() const { return &client_info; }  
    const size_t client_size() const { return sizeof(struct sockaddr_in); }
    const FD get_fd() const { return errored ? FD(-1) : socket_fd; }
    operator int() const { return socket_fd; }
};
inline int write(UDP_FD fd,const void* buffer,size_t len)
{
    return sendto(fd.get_fd(),buffer,len,0,(struct sockaddr*)fd.get_client_info(),fd.client_size());
}
inline int read(UDP_FD fd,void* buffer,size_t len)
{
    socklen_t client_info_size;
    int status = recvfrom(fd.get_fd(),buffer,len,0,(struct sockaddr*)fd.get_client_info(),&client_info_size);
    return status;
}
int create_udp_server(UDP_FD listen,UDP_FD& client_handler,int buffer_size);
