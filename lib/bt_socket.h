/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/


#ifndef HCI_SOCKET
#define HCI_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "include/bluetooth.h"
#include "include/hci.h"
#include "include/hci_lib.h"
#include "include/rfcomm.h"
#include "include/l2cap.h"
#include "include/sco.h"
#include "include/libbunget.h"
#ifdef USE_UVLIB
#include <uv.h>
#endif //NO_USE_UVLIB
#include <string>
#include <map>

#define BT_SNDMTU		    12
#define BT_RCVMTU		    13
#define HCI_CHANNEL_USER    1
#define ATT_CID             4


#define _THROW(ex)      {::perror(""); throw bunget::hexecption(__FILE__,__FUNCTION__,errno,ex);}
#define _TRACEF()       std::cout << __FILE__ << " / " << __LINE__ << "\n"
#define LOOP_SLEEP_IDLE      1000
#define LOOP_SLEEP_NORM      100
#define LOOP_SLEEP_FAST      10


class bt_socket;
class hci_error;
class sdata;
class hci_data_eater
{
public:
    virtual ~hci_data_eater(){}
    virtual int on_sock_data(uint8_t code, const sdata& data)=0;
    virtual void on_error(const hci_error& error){};
    virtual bool onSpin(bt_socket* sock){return false;};
};

//================================================================================
class bt_socket
{
public:
    bt_socket(hci_data_eater* hci):_sock(0),_hci(hci){};
    virtual ~bt_socket();

    virtual void create()=0;
    virtual void close();
    virtual void bind(const bdaddr_t& addr, uint8_t srctype, uint16_t psm=0, uint8_t cid=0)=0;
    virtual int read(uint8_t* buffer, int sizeb);
    virtual int connect(struct sockaddr *){return 0;};
    virtual int writeocts(const uint8_t* buffer, int sizeb);
    template <typename T>int write(const T& t, size_t sz=sizeof(T)){
        return this-> writeocts((const uint8_t*)&t, sz);
    }
    bool valid()const{return _sock>0;}
    int sock(){return _sock;}

protected:
    int     _sock;
    int     _mode;
    hci_data_eater* _hci;
};


#endif // HCI_SOCKET
