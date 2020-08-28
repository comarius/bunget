/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef SCO_SOCKET_H
#define SCO_SOCKET_H

#include "bt_socket.h"

class hci_data_eater;
class sco_socket : public bt_socket
{
public:
     sco_socket(hci_data_eater* hci):bt_socket(hci){}
     ~sco_socket(){};

     void create();
     void bind(const bdaddr_t& addr, uint8_t error, uint16_t psm=0, uint8_t cid=0);
};

#endif // SCO_SOCKET_H
