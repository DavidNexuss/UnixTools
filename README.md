# Unixtools

This is still work in progress so don't expect all of this stuff to work right away.

This is a repository that contains C/C++ headers for wrapping multiple posix syscalls for network and file descriptors that attempts to generalize even
more the system interface providing in some cases C++ wrappers for example


## FD template examples
``` c
    int fd = open("some_file",O_RDONLY);
```

``` c++
    FD fd = open("some_file",O_RDONLY);
```

FD is a c++ template in one of the heades that keeps a smart pointer to a file descriptor, when the last reference to fd is destroyed, the FD destructor will call close syscall automatically. So close will be called automatically when the file descriptor is no longer used.

## Net examples


``` c
    int sfd = create_inet_server("localhost",8080);
    int sfd = create_unix_server("/tmp/mysocket");

    int nfd = create_inet_connection("localhost",8080);
    int nfd = create_unix_connection("/tmp/mysocket");
```

Basic functions for creating sockets, if an error -1 is returned.

``` c
int nfd;
if ((nfd = create_server(create_inet_server("localhost",8080))) >= 0)
{
    //Handle client connection with nfd
}
//Server error
```

create_inet_server will create a inet server socket and return its fd, then create_server receives a socket file descriptor and starts an infinite loop accepting connections, when a connection is accepted process is forked and the nfd is returned. Main process is kept listening for connections, if an error is encountered -1 is returned

## UDP_FD examples

UDP_FD is a C++ wrapper that maps sendto and recvfrom posix calls to write and read calls so you can use UDP_FD as any other file descriptor in a function that accepts a file descriptor (using templates)

``` c++
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
```

UDP is connectionless so either client and server UPD_FD can be used as server or client, the library makes the distinction between using UDP_FD oriented to act as server or client in the moment of its creation for calling the bind function eg (The udp client wont require an especific mapped port, any avaible port should be fine, in contrast to server which requires a well known port).

## AES FD encryption

``` c
    
    int fd_enc = aes_crypt(0,key,1);
    int fd_dec = aes_crypt(fd_enc,key,1);

    cpy(fd_dec,1);

    //For killing the crypt process once we are done
    close(0);
    close(fd_enc);
```

aes_crypt can decrypt or encrypt a fd, the process is forked and the input fd is closed in the main process, the file descriptor of the pipe that will be used to comunicate the main process and the encryption process will be returned, in case of failure -1 will be returned.

In c++ this is implemented as a template so you can use any class or object compatible with write and read calls. Such us UDP_FD.

In theory you could do in c++ to encrypt a UDP_FD using 

``` c++
FD fd_enc = aes_crypt(UDP_FD("192.168.1.24",9776),key,1);

```

Writing to fd_enc will cause the encryption process to encrypt blocks and send them to the fixed address 192.168.1.24:9776 using UDP


## Exec module

This module allows you to execute pipelines and process in c++ without using the function system and depending on the system shell execution.


``` c++
    execute({"cat","/proc/cpuinfo"});   //This will execute the exec call and mutate to the cat process

    int fd = execute_pipe(0,{"cat","/proc/cpuinfo"})    
    //This will fork the process and mutate the child to cat, input will be gathered from fd 0 and set the output to fd so the father process can read it.

    int fd = execute_pipe(execute_pipe(0,{"cat","/proc/cpuinfo"}),{"grep","MHz"});
    //Excute pipe calls can be combined to create a pipeline with its initial input to 0 and final output to fd
```


