#include <map>
#include <string>
#include <iostream>
#include "udp.h"
using namespace std;

struct NetworkIdentifier
{
    std::string host;
    int port;
    
    NetworkIdentifier() { }
    NetworkIdentifier(const UDP_FD& current)
    {
        char addr_string[64];
        inet_ntop(AF_INET, &(current.get_client_info()->sin_addr), addr_string, INET_ADDRSTRLEN);
        host = std::string(addr_string);
        port = current.get_client_info()->sin_port;
    }

    bool operator <(const NetworkIdentifier& other) const
    {
        return host < other.host || (host == other.host && port < other.port);
    }
};

inline std::ostream& operator<<(std::ostream& stream, const NetworkIdentifier& identifier)
{
    stream << identifier.host << ":" << identifier.port;
    return stream;
}

int create_udp_server(UDP_FD listen,UDP_FD& connection_handler,int buffer_size)
{
    if (listen.get_fd() < 0)
    {
        perror("create_udp_server");
        return 0;
    }
    char buffer[buffer_size];
    int n;
    map<NetworkIdentifier,FD> current_connections;
    while((n = read(listen,buffer,buffer_size)) > 0)
    {
        NetworkIdentifier current(listen);
        auto it = current_connections.find(current);
        if (it != current_connections.end())
        {
            cerr << "Fluhing connection from " << current << " to fd " << it->second << endl;
            write(it->second,buffer,n);
        }
        else
        {
            int pipe_fd[2];
            pipe(pipe_fd);
            int f = fork();
            if (f == 0)
            {
                close(pipe_fd[1]);
                connection_handler = listen;
                return pipe_fd[0];
            }
            
            close(pipe_fd[0]);
            current_connections[current] = pipe_fd[1];
            cerr << "Accepted connection from " << current.host << ":" << current.port << "to fd " << pipe_fd[1] << endl;
            write(pipe_fd[1],buffer,n);
        }
    }
    perror("recvfrom");
    return 0;
}
