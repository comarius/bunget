/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#include "uguid.h"

#include "hci_socket.h"
#include "bu_hci.h"
#include "libbungetpriv.h"

/****************************************************************************************
*/
typedef enum
{
    BT_IO_L2CAP,
    BT_IO_RFCOMM,
    BT_IO_SCO,
    BT_IO_INVALID,
} BtIOType;

/****************************************************************************************
*/
bt_socket::~bt_socket()
{
    close();
}

/****************************************************************************************
*/
void bt_socket::close()
{
    if(_sock>0)
    {
        ::close(_sock);
        _sock = 0;
    }
}

/****************************************************************************************
*/
int bt_socket::read(uint8_t* buffer, int sizeb)
{
    return ::read(this->_sock, buffer, sizeb);
}

/****************************************************************************************
*/
int bt_socket:: writeocts(const uint8_t* buffer, int sizeb)
{

    bybuff  data(buffer,sizeb);
    int r = ::write(this->_sock, buffer, sizeb);
    TRACE("\n<--[" <<int(data.length())<<"/"<<r<<"]" << data.to_string());
    ::fsync(this->_sock);
	return r;
}
