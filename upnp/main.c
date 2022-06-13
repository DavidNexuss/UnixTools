#include <stdio.h>
#include <arpa/inet.h> //INET6_ADDRSTRLEN
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>

int main(int argc, char *argv[]) {
    int error = 0;
    //get a list of upnp devices (asks on the broadcast address and returns the responses)
    struct UPNPDev *upnp_dev = upnpDiscover(1000,    //timeout in milliseconds
                                            NULL,    //multicast address, default = "239.255.255.250"
                                            NULL,    //minissdpd socket, default = "/var/run/minissdpd.sock"
                                            0,       //source port, default = 1900
                                            0,       //0 = IPv4, 1 = IPv6
                                            2,
                                            &error); //error output

    if(upnp_dev == NULL || error != 0) {
        fprintf(stderr, "Could not discover upnp device\n");
        freeUPNPDevlist(upnp_dev);
        return 1;
    }
    
    char lan_address[INET6_ADDRSTRLEN]; //maximum length of an ipv6 address string
    struct UPNPUrls upnp_urls;
    struct IGDdatas upnp_data;
    int status = UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address));
    
    if(status != 1) { //there are more status codes in minupnpc.c but 1 is success all others are different failures
        fprintf(stderr, "No valid Internet Gateway Device could be connected to");
        FreeUPNPUrls(&upnp_urls);
        freeUPNPDevlist(upnp_dev);
        return -1;
    }

    // get the external (WAN) IP address
    char wan_address[INET6_ADDRSTRLEN];
    if(UPNP_GetExternalIPAddress(upnp_urls.controlURL, upnp_data.first.servicetype, wan_address) != 0) {
        fprintf(stderr, "Could not get external IP address");
    }else{
        printf("External IP: %s\n", wan_address);
    }
    
    // add a new TCP port mapping from WAN port 12345 to local host port 24680
    error = UPNP_AddPortMapping(
            upnp_urls.controlURL,
            upnp_data.first.servicetype,
            "12345"     ,  // external (WAN) port requested
            "24680"     ,  // internal (LAN) port to which packets will be redirected
            lan_address , // internal (LAN) address to which packets will be redirected
            "FooBar server for XYZ", // text description to indicate why or who is responsible for the port mapping
            "TCP"       , // protocol must be either TCP or UDP
            NULL        , // remote (peer) host address or nullptr for no restriction
            "86400"     ); // port map lease duration (in seconds) or zero for "as long as possible"

    if(error) {
        fprintf(stderr, "Failed to map ports\n");
    }else{
        printf("Successfully mapped ports\n");
    }
    
    printf("Lan Address\tWAN Port -> LAN Port\tProtocol\tDuration\tEnabled?\tRemote Host\tDescription\n");
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
        error = UPNP_GetGenericPortMappingEntry(
                    upnp_urls.controlURL            ,
                    upnp_data.first.servicetype     ,
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
    
        printf("%s\t%s -> %s\t%s\t%s\t%s\t%s\t%s\n", 
        map_lan_address,    map_wan_port,        map_lan_port,    map_protocol,
        map_lease_duration, map_mapping_enabled, map_remote_host, map_description);
    }

    FreeUPNPUrls(&upnp_urls);
    freeUPNPDevlist(upnp_dev);
}
