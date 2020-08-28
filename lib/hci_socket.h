/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef HCI_SOCKET_H
#define HCI_SOCKET_H

#include "bt_socket.h"

class hci_data_eater;
class l2cap_socket;
class hci_socket_ble : public bt_socket
{
public:
     hci_socket_ble(hci_data_eater* hci):bt_socket(hci),_devId(0),_adaptivespeed(LOOP_SLEEP_IDLE),
                        _loops(0),
                        _addressType(0),_bytes(0)
     {
        ::memset((void*)_address,0,sizeof(_address));
     }
     ~hci_socket_ble();
    void create();
    void create_bind(int dev_id);
    void stop();
    void bind(const bdaddr_t& addr, uint8_t channel, uint16_t psm=0, uint8_t cid=0){};
    int bind_raw(int* devId);
    int bind_user(int* devId);
    void bind_ctrl();
    bool is_up();
    void close();
    void set_filter(const uint8_t* data, int length);
    int read(uint8_t* buffer, int sizeb);
    bool pool(int time = 10, bool callhci=true);
#ifdef USE_UVLIB  /// this was not tested !?!
    static void uv_this_cb(uv_poll_t* handle, int status, int events);
    static void uv_this_cb_close(uv_poll_t* handle);
#endif //
private:
    void _tweakHciKernel(int length, uint8_t* data);
    int _resolve_devid(int* pDevId, bool isUp);
    void _notify_read();
	void _send_cmd(uint16_t ogf, uint16_t ocf, uint8_t plen, void *param);
private:
    int         _devId;
    //std::map<uint16_t,l2cap_socket*> _l2sockets;
    std::map<uint16_t,int> _l2sockets;
#ifdef USE_UVLIB
    uv_poll_t   _pollHandle;
#endif //
    int         _checkfreq;
    int         _adaptivespeed;
    int         _loops;
    uint8_t     _address[8];
    int32_t     _addressType;
	uint8_t		_buff[512];
	int			_bytes;
};

#endif // HCI_SOCKET_H
