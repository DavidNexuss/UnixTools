#include <iostream>
#include <net/upnp.h>

using namespace std;
int main()
{
    int upnp_controller = create_upnp_controller();
    if (upnp_controller == -1)
    {
        cerr << "Could not create upnp controller" << endl;
        return 1;
    }
    UPnPDevice device(upnp_controller,"Ejemplo","8080","7653",false,86400);
    debug_dump_upnp_controller_list(upnp_controller);
    cout << "Wan address: " << get_wan_address(upnp_controller) << endl;
}
