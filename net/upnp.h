#pragma once
#include <string>

int create_upnp_controller();
void debug_dump_upnp_controller_list(int upnp_controller);
std::string get_wan_address(int upnp_controller);

class UPnPDevice
{
    int upnp_controller;
    int error;
    std::string localport,externalport;

    public:
    UPnPDevice(int unpn_controller,const std::string& name,const std::string& localport,const std::string& externalport,bool remote,int timeout);
    ~UPnPDevice();
    
    const std::string& get_external_port() const;
};
