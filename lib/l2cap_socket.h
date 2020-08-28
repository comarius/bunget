/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef L2CAP_SOCKET_H
#define L2CAP_SOCKET_H

#include "bt_socket.h"

class hci_data_eater;
class l2cap_socket : public bt_socket
{
public:
    l2cap_socket(hci_data_eater* hci):bt_socket(hci){}
    ~l2cap_socket(){};

    void create();
    void bind(const bdaddr_t& addr, uint8_t srctype, uint16_t psm=0, uint8_t cid=0);
    void set(uint8_t src_type, int sec_level,
             uint16_t imtu, uint16_t omtu, uint8_t mode,
             int master, int flushable, uint32_t priority);
    int connect(struct sockaddr *);
private:
    void    _set_l2opts(uint16_t imtu, uint16_t omtu, uint8_t mode);
    void    _set_le_imtu(uint16_t imtu);
    void    _l2cap_set_master(int master);
    void    _l2cap_set_flushable(bool flushable);
    void    _set_priority(uint32_t prio);
    void    _set_sec_level(int level);
    void    _l2cap_set_lm(int level);
};

#endif // L2CAP_SOCKET_H
