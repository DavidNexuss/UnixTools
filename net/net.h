#pragma once

#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define SA struct sockaddr

const int max_client_queue = 5;

int create_inet_server(int port_number)
{
    struct sockaddr_in servaddr = { 0 };
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        return -1;
    }
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port_number);
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        return -1;
    }

    if ((listen(sockfd, max_client_queue)) != 0) {
        return -1;
    } 
    return sockfd;
}

int create_inet_connection(const char* name,int port_number)
{
    int sock = 0, valread; 
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1; 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port_number); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, name, &serv_addr.sin_addr)<=0)   return -1; 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) return -1; 

    return sock;
}

int unix_build_address(const char *path, struct sockaddr_un *addr)
{
    if (addr == NULL || path == NULL) {
        errno = EINVAL;
        return -1;
    }

    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    if (strlen(path) < sizeof(addr->sun_path)) {
        strncpy(addr->sun_path, path, sizeof(addr->sun_path) - 1);
        return 0;
    } else {
        errno = ENAMETOOLONG;
        return -1;
    }
}

int create_unix_server(const char *path)
{
    int sd;
    struct sockaddr_un addr;

    if (unix_build_address(path, &addr) == -1)
        return -1;

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd == -1)
        return -1;

    if (bind(sd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        close(sd); 
        return -1;
    }
    if ((listen(sd, max_client_queue)) != 0) {
        return -1;
    }
    return sd;
}

int create_unix_connection(const char *path)
{
    int sd;
    struct sockaddr_un addr;
    if (unix_build_address(path, &addr) == -1)
        return -1;

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd == -1)
        return -1;

    if (connect(sd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1) {
        close(sd);
        return -1;
    }

    return sd;
}
//Handles a server given a TCP socket file descriptor, new connection will return as a new process
int create_server(int sfd)  
{
    if (sfd == -1) {
        perror("create_server");
        exit(1);
    }

    int nfd;
    while((nfd = accept(sfd,NULL,NULL)) != -1) {
        int f = fork();
        if (f == 0) {
            return nfd;
        }
        close(nfd);
    }

    perror("accept");
    return -1;
}

#ifdef __cplusplus
#include <thread>
#include <vector>
#include <unordered_map>

using ConnectionStateIdentifier = size_t;

struct ConnectionState { 
    int inputFd;
    int outputFd;
};

template <typename ConnectionState>
struct ServerState { 
    ConnectionStateIdentifier nextIdentifier = 0;
    std::unordered_map<ConnectionStateIdentifier,ConnectionState> connections;

    template <typename T>
    ConnectionStateIdentifier createConnection(ConnectionState state,T&& function) {

        ConnectionStateIdentifier currentId = nextIdentifier;
        connections[currentId] = state;

        std::thread([function,currentId,this](){
            function(currentId, this->connections[currentId],*this);
            this->finalizeConnection(currentId);
        }).detach();

        nextIdentifier++;
        return currentId;
    } 

    void finalizeConnection(ConnectionStateIdentifier identifier) { 
        connections.erase(identifier);
    }


    size_t connectionCount() const { return connections.size(); }

    template <typename T>
    void foreachConnection(T&& function) {
        for (const auto& kv : connections) { 
            function(kv.first,kv.second);
        }
    } 
};  

template <typename ConnectionState,typename T>
void create_server(int sfd,T&& function) { 
    if (sfd == -1) {
        perror("create_server");
        exit(1);
    }


    ServerState<ConnectionState> serverState;
    int nfd;
    while((nfd = accept(sfd,NULL,NULL)) != -1) { 
        serverState.createConnection({nfd,nfd},function);
    }
    perror("accept");
}
#endif
