/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#include "rfcomm_socket.h"

/****************************************************************************************
*/
void rfcomm_socket::create()
{

    _sock = ::socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(_sock<=0) _THROW(0);
}

/****************************************************************************************
*/
void rfcomm_socket::bind(const bdaddr_t& src, uint8_t channel, uint16_t psm, uint8_t cid)
{
    struct sockaddr_rc addr;

    ::memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    ::bacpy(&addr.rc_bdaddr, &src);
    addr.rc_channel = channel;

    if (::bind(_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        _THROW(0);
    }
}
