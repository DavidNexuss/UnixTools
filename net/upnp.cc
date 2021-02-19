#include "upnp.h"
#include <vector>
#include <memory>
#include <arpa/inet.h> //INET6_ADDRSTRLEN
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <cstdio>
#include <cstring>

using namespace std;

struct UPnPController
{
    struct UPNPDev* deviceController = nullptr;
    struct UPNPUrls upnp_urls;
    struct IGDdatas upnp_data;

    int status;
    char lan_address[INET6_ADDRSTRLEN];

    UPnPController(struct UPNPDev* _deviceController) : deviceController(_deviceController) 
    { 
        status = UPNP_GetValidIGD(deviceController, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address));
    }
    ~UPnPController()
    {
        dprintf(2,"UPnP disposing controller resources\n");

        freeUPNPDevlist(deviceController);
        FreeUPNPUrls(&upnp_urls);
    }
};

using UPnPControllerPtr = std::shared_ptr<UPnPController>;
vector<UPnPControllerPtr> controllers;

int create_upnp_controller()
{
    int error = 0;
    struct UPNPDev* device = upnpDiscover(1000,NULL,NULL,0,0,2,&error);
    if (device == NULL || error != 0)
    {
        freeUPNPDevlist(device);
        return -1;
    }
    UPnPControllerPtr deviceController = make_shared<UPnPController>(device);
    if (deviceController->status != 1)
    {
        printf("Error creating UPnP deviceController\n");
        return -1;
    }
    printf("Success UPnP deviceController\n");
    controllers.push_back(deviceController);
    return controllers.size() - 1;
}
void debug_dump_upnp_controller_list(int upnp_controller)
{
    if (upnp_controller == -1) return;
    UPnPControllerPtr deviceController = controllers[upnp_controller];

    dprintf(2,"Lan Address\tWAN Port -> LAN Port\tProtocol\tDuration\tEnabled?\tRemote Host\tDescription\n");
    // list all port mappings
    size_t index;
    for(index = 0;;++index) {
        char map_wan_port           [6]  = "";
        char map_lan_address        [16] = "";
        char map_lan_port           [6]  = "";
        char map_protocol           [4]  = "";
        char map_description        [80] = "";
        char map_mapping_enabled    [4]  = "";
        char map_remote_host        [64] = "";
        char map_lease_duration     [16] = ""; // original time, not remaining time :(
    
        char indexStr[10];
        sprintf(indexStr, "%d", index);
        int error = UPNP_GetGenericPortMappingEntry(
                    deviceController->upnp_urls.controlURL            ,
                    deviceController->upnp_data.first.servicetype     ,
                    indexStr                        ,
                    map_wan_port                    ,
                    map_lan_address                 ,
                    map_lan_port                    ,
                    map_protocol                    ,
                    map_description                 ,
                    map_mapping_enabled             ,
                    map_remote_host                 ,
                    map_lease_duration              );
    
        if (error) {
            break; // no more port mappings available
        }
    
        dprintf(2,"%s\t%s -> %s\t%s\t%s\t%s\t%s\t%s\n", 
        map_lan_address,    map_wan_port,        map_lan_port,    map_protocol,
        map_lease_duration, map_mapping_enabled, map_remote_host, map_description);
    }
}
string get_wan_address(int upnp_controller)
{
    if (upnp_controller == -1) return "";
    UPnPControllerPtr deviceController = controllers[upnp_controller];
    char wan_address[80];
    if (UPNP_GetExternalIPAddress( deviceController->upnp_urls.controlURL, deviceController->upnp_data.first.servicetype, wan_address) != 0)
    {
        dprintf(2,"Unable to get wan address");
        return "";
    }
    else
    {
        return string(wan_address);
    }
}
UPnPDevice::UPnPDevice(int _upnp_controller,const std::string& name,const std::string& _localport,const std::string& _externalport, bool remote,int timeout)
: upnp_controller(_upnp_controller), localport(_localport), externalport(_externalport)
{
    if (upnp_controller == -1)
    {
        error = 1;
        return;
    }

    error = UPNP_AddPortMapping(
        controllers[upnp_controller]->upnp_urls.controlURL,
        controllers[upnp_controller]->upnp_data.first.servicetype,
        externalport.c_str(),
        localport.c_str(),
        controllers[upnp_controller]->lan_address,
        name.c_str(),
        "TCP",
        NULL,
        to_string(timeout).c_str()
    );
    dprintf(2,"UPnP mapped port %s to %s %s\n",externalport.c_str(),localport.c_str(),error == 1 ? "ERROR" : "OK");
}
UPnPDevice::~UPnPDevice()
{
    if (error == 0)
    {
        error = UPNP_DeletePortMapping(
            controllers[upnp_controller]->upnp_urls.controlURL,
            controllers[upnp_controller]->upnp_data.first.servicetype,
            externalport.c_str(),
            "TCP",
            NULL);
        
        dprintf(2,"UPnP remove port %s to %s %s\n",externalport.c_str(),localport.c_str(),error == 1 ? "ERROR" : "OK");
    }
}
const string& UPnPDevice::get_external_port() const
{
    return externalport;
}
