#include "net.h"
#include <iostream>
using namespace std;


struct MyState : public ConnectionState { 

};

int main() { 
    create_server<MyState>(create_unix_server("myserver"),
            [](ConnectionStateIdentifier id,MyState& connectionState,ServerState<MyState>& serverState){
                cout << "New connection: " << id << endl;
                cout << "Connection count: " << serverState.connectionCount() << endl;
                cout << "Connection ids: ";
                serverState.foreachConnection([](ConnectionStateIdentifier other_id, ConnectionState connectionState){
                    cout << other_id << " ";
                });

                cout << endl;

                cout << "Writing greeting message..." << endl;

                string greetingMessage = "Hello user with id: " + to_string(id) + "\n";
                string otherMessage = "Another user with id: " + to_string(id) + " connected to server\n";

                write(connectionState.outputFd,greetingMessage.c_str(),greetingMessage.size());

                serverState.foreachConnection([&](ConnectionStateIdentifier other_id,ConnectionState connectionState){ 
                    if (other_id != id) { 

                        write(connectionState.outputFd,otherMessage.c_str(),otherMessage.size());
                    }
                });

                cout << "Reading client response..." << id << endl;
                char buffer[1024];
                read(connectionState.inputFd,buffer,sizeof(buffer));

                cout << "Finalizing connection with client... " << id << endl;
                
            });
}
