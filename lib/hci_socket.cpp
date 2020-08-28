/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/
#include "uguid.h"
#include "bu_hci.h"
#include "hci_socket.h"
#include "l2cap_socket.h"
#include "include/libbunget.h"


/****************************************************************************************
*/
hci_socket_ble::~hci_socket_ble()
{
#ifdef USE_UVLIB
    ::uv_close((uv_handle_t*)&this->_pollHandle, (uv_close_cb)hci_socket_ble::uv_this_cb_close);
#endif //
}

/****************************************************************************************
*/
void hci_socket_ble::stop()
{
#ifdef USE_UVLIB
    ::uv_poll_stop(&this->_pollHandle);
#endif //
}

/****************************************************************************************
*/
void hci_socket_ble::create()
{
    _checkfreq = 0;
    _sock = ::socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if(_sock<=0) _THROW(0);
#ifdef USE_UVLIB
    ::uv_poll_init(uv_default_loop(), &this->_pollHandle, this->_sock);
    this->_pollHandle.data = this;
#endif //
}

/****************************************************************************************
*/
void hci_socket_ble::create_bind(int dev_id)
{
	if (dev_id < 0)
	{
		_THROW(0);
	}

	_sock= socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
 	if(_sock<=0) _THROW(0);

	struct sockaddr_hci a;
	::memset(&a, 0, sizeof(a));
	a.hci_family = AF_BLUETOOTH;
	a.hci_dev = dev_id;
	if (::bind(_sock, (struct sockaddr *) &a, sizeof(a)) < 0)
	{
        close();
        _THROW(0);
	}
}

/****************************************************************************************
*/

#ifdef USE_UVLIB
void hci_socket_ble::uv_this_cb_close(uv_poll_t* handle)
{
    delete handle;
}


/****************************************************************************************
*/
void hci_socket_ble::uv_this_cb(uv_poll_t* handle, int status, int events)
{
    hci_socket_ble *p = (hci_socket_ble*)handle->data;
	_bytes = this->read(_buff, sizeof(_buff);
    p->_notify_read();
}
#endif //

/****************************************************************************************
*/
int hci_socket_ble::bind_raw(int* devId)
{
    struct sockaddr_hci a;

    ::memset(&a, 0, sizeof(a));
    a.hci_family = AF_BLUETOOTH;
    a.hci_dev = this->_resolve_devid(devId, true);
    a.hci_channel = HCI_CHANNEL_RAW;
    this->_devId = a.hci_dev;
    this->_mode = HCI_CHANNEL_RAW;

    if(::bind(this->_sock, (struct sockaddr *) &a, sizeof(a))<0)
    {
        if(_loops==0)
        {
            _THROW("Device unplugged !!");
        }
        else
        {
            hci_error e;
            e.nerror = errno;
            e.message="unplugged";
            _hci->on_error(e);
        }
    }
    _loops++;
    //added from BLuetoothHciSocket.cpp
    // get the local address and address type
    struct hci_dev_info di;
    ::memset(&di, 0x00, sizeof(di));
    di.dev_id = this->_devId;
    ::memset(_address, 0, sizeof(_address));
    _addressType = 0;
    if (::ioctl(this->_sock, HCIGETDEVINFO, (void *)&di) > -1)
    {
        ::memcpy(_address, &di.bdaddr, sizeof(di.bdaddr));
        _addressType = int(di.type);
        if (_addressType == 3)//hack
            _addressType = 1;

		if (hci_test_bit(HCI_INQUIRY, &di.flags))
			_send_cmd(OCF_INQUIRY_CANCEL, OGF_LINK_CTL,0,0);
		else
			_send_cmd(OCF_EXIT_PERIODIC_INQUIRY, OGF_LINK_CTL,0,0);

    }
    return this->_devId;
}

void hci_socket_ble::_send_cmd(uint16_t ocf, uint16_t ogf, uint8_t plen, void *param)
{
    uint8_t loco[512];
    hci_command_hdr hc;

    hc.opcode   = CMD_OPCODE_PACK(ocf, ogf);
    hc.plen     = plen;

    memcpy(loco, &hc, sizeof(hc));
    if(plen)
        memcpy(loco+sizeof(hc), param, plen);
    writeocts(loco, plen+sizeof(hc));
}

/****************************************************************************************
*/
int hci_socket_ble::bind_user(int* devId)
{
    struct sockaddr_hci a;

    ::memset(&a, 0, sizeof(a));
    a.hci_family = AF_BLUETOOTH;
    a.hci_dev = this->_resolve_devid(devId, false);
    a.hci_channel = HCI_CHANNEL_USER;
    this->_devId = a.hci_dev;
    this->_mode = HCI_CHANNEL_USER;
    if(::bind(this->_sock, (struct sockaddr *) &a, sizeof(a))<0)
    {
        _THROW(0);
    }
    return this->_devId;
}

/****************************************************************************************
*/
void hci_socket_ble::bind_ctrl()
{
    struct sockaddr_hci a;

    ::memset(&a, 0, sizeof(a));
    a.hci_family = AF_BLUETOOTH;
    a.hci_dev = HCI_DEV_NONE;
    a.hci_channel = HCI_CHANNEL_CONTROL;
    this->_mode = HCI_CHANNEL_CONTROL;
    if(::bind(this->_sock, (struct sockaddr *) &a, sizeof(a))<0)
    {
        _THROW(0);
    }
}

/****************************************************************************************
*/
bool hci_socket_ble::is_up()
{
    struct hci_dev_info di;
    bool cacheup = false;

    ::memset(&di, 0x00, sizeof(di));
    di.dev_id = this->_devId;
    if (::ioctl(this->_sock, HCIGETDEVINFO, (void *)&di) > -1)
    {
        cacheup = (di.flags & (1 << HCI_UP)) != 0;
    }
    return cacheup;
}

/****************************************************************************************
*/
void hci_socket_ble::set_filter(const uint8_t* data, int length)
{
    bybuff buffer(data, length);
    if (::setsockopt(this->_sock, SOL_HCI, HCI_FILTER, data, length) < 0)
    {
        _THROW(0);
    }
}

/****************************************************************************************
*/
int hci_socket_ble::read(uint8_t* buffer, int sizeb)
{
    int length = bt_socket::read(buffer, sizeb);
    if (length > 0 && this->_mode == HCI_CHANNEL_RAW)
    {
        this->_tweakHciKernel(length, buffer);
    }
    return length;
}

/****************************************************************************************
*/
int hci_socket_ble::_resolve_devid(int* pDevId, bool isUp)
{
    int devId = 0; // default

    if (*pDevId == -1)/* pick first one*/
    {
        struct hci_dev_list_req *dl;
        struct hci_dev_req *dr;

        dl = (hci_dev_list_req*)calloc(HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl), 1);
        dr = dl->dev_req;

        dl->dev_num = HCI_MAX_DEV;

        if (::ioctl(this->_sock, HCIGETDEVLIST, dl) > -1)
        {
            for (int i = 0; i < dl->dev_num; i++, dr++)
            {
                bool devUp = dr->dev_opt & (1 << HCI_UP);
                bool match = isUp ? devUp : !devUp;

                if (match)
                {
                    // choose the first device that is match
                    // later on, it would be good to also HCIGETDEVINFO and check the HCI_RAW flag
                    devId = dr->dev_id;
                    *pDevId = devId;
                    break;
                }
            }
        }

        free(dl);
    }
    else
    {
        devId = *pDevId;
    }
    return devId;
}

/****************************************************************************************
*/
bool hci_socket_ble::pool(int ttp, bool callhci)
{
    fd_set          read;
    struct timeval  to;
    int             tout = ttp==-1 ? 16 : ttp;
    bool            got = false;

    do{
        FD_ZERO (&read);
        FD_SET (_sock, &read);
        to.tv_sec  = 0;
        to.tv_usec = 1024;
        int rv = ::select(_sock+1, &read, 0, 0, &to);
        if(rv < 0)
        {
            hci_error e;
            e.nerror = errno;
            e.message="network-error";
            _hci->on_error(e);
            return false;
        }
        if(FD_ISSET(_sock, &read))
        {
            int len = this->read(_buff, sizeof(_buff));
            if(len > 0){
                _bytes=len;
                got=true;
            }
        }
        FD_CLR (_sock, &read);
        if(_bytes)
	    _notify_read();
        if(callhci)
	    _hci->onSpin(this);
    }while(--ttp>0);
    return got;
}

/****************************************************************************************
*/
void hci_socket_ble::_notify_read()
{
	sdata    packed;
	packed.data = _buff;
	packed.len = (uint16_t)_bytes;
	_hci->on_sock_data(0, packed);
	_bytes = 0;
}

void hci_socket_ble::close()
{
//_l2sockets
	std::map<uint16_t,int>::iterator i =  _l2sockets.begin();
	for(;i!=_l2sockets.end();++i)
		::close(i->first);
	_l2sockets.clear();
	bt_socket::close();
}

// taken from BluetoothHciSocket.cpp
void hci_socket_ble::_tweakHciKernel(int length, uint8_t* data)
{
    
    if (length == 22 )
    {
        if(data[0] == 0x04 &&
            data[1] == 0x3e &&
            data[2] == 0x13 &&
            data[3] == 0x01 &&
            data[4] == 0x00)
        {
            _TRACE(__FUNCTION__);
            int l2socket;
            struct sockaddr_l2 l2a;
            unsigned short l2cid;
            unsigned short handle = *((unsigned short*)(&data[5]));

    #if __BYTE_ORDER == __LITTLE_ENDIAN
        l2cid = ATT_CID;
    #elif __BYTE_ORDER == __BIG_ENDIAN
        l2cid = bswap_16(ATT_CID);
    #endif

            l2socket = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

            ::memset(&l2a, 0, sizeof(l2a));
            l2a.l2_family = AF_BLUETOOTH;
            l2a.l2_cid = l2cid;
            ::memcpy(&l2a.l2_bdaddr, _address, sizeof(l2a.l2_bdaddr));
            l2a.l2_bdaddr_type = (uint8_t)_addressType;
            ::bind(l2socket, (struct sockaddr*)&l2a, sizeof(l2a));

            ::memset(&l2a, 0, sizeof(l2a));
            l2a.l2_family = AF_BLUETOOTH;
            ::memcpy(&l2a.l2_bdaddr, &data[9], sizeof(l2a.l2_bdaddr));
            l2a.l2_cid = l2cid;
            l2a.l2_bdaddr_type = data[8] + 1; // BDADDR_LE_PUBLIC (0x01), BDADDR_LE_RANDOM (0x02)

            ::connect(l2socket, (struct sockaddr *)&l2a, sizeof(l2a));

            this->_l2sockets[handle] = l2socket;
        }
    }
    else if (length == 7 && 
            data[0] == 0x04 && 
            data[1] == 0x05 && 
            data[2] == 0x04 
            && data[3] == 0x00)
    {
        unsigned short handle = *((unsigned short*)(&data[4]));

        if (this->_l2sockets.count(handle) > 0)
        {
            ::close(this->_l2sockets[handle]);
            this->_l2sockets.erase(handle);
        }
    }
}
