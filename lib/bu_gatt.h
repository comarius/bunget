/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef NO_GATT_H
#define NO_GATT_H


#include <vector>
#include <map>
#include "include/uuid.h"
#include <vector>
#include "bybuff.h"
#include "hci_socket.h"
#include "bu_hci.h"
#include "gattdefs.h"

struct g_handler;
class bu_hci;
class bu_asc;

struct WriteRequest
{
    WriteRequest():_hndl(0){}
    uint16_t    _hndl;
    uint16_t    _valhhdl;
    uint16_t    _offset;
    bybuff      _data;
};



class bu_gatt : public hci_data_eater
{
public:
    bu_gatt(bu_hci* hci);
    virtual ~bu_gatt();
    int on_sock_data(uint8_t code, const sdata& data);
    void setAclPtr(bu_asc* pacls);
    SrvDevice* srv(){return _hci->srv();}
    int writeCharaterisitc(GHandler* ph);
    int write_descr(GHandler* pel);
    bu_asc*  pacl(){return  _pacls;}
    void    reset();
    void setMaxMtu(int mtu){_maxMtu=mtu;}
private:
    int _dummy_q(const sdata& data, bybuff& ret);
    int _group_q(const sdata& data, bybuff& ret);
    int _type_q(const sdata& data, bybuff& ret);
    int _info_q(const sdata& data, bybuff& ret);
    int _write_cmd(const sdata& data, bybuff& ret);
    int _write_q(const sdata& data, bybuff& ret);
    int _find_type_q(const sdata& data, bybuff& ret);
    int _read_q(const sdata& data, bybuff& ret);
    int _read_blob(const sdata& data, bybuff& ret);
    int _prep_wq(const sdata& indata, bybuff& ret);
    int _exec_wq(const sdata& data, bybuff& ret);
    int _indic_confirm(const sdata& data, bybuff& ret);
    int _send(const bybuff& data);
    int _reply_err(uint8_t code, uint16_t handle, uint8_t status, bybuff& buff);

private:
    bu_hci*         _hci;
    int             _maxMtu;
    int             _mtu;
    int             _preparedWriteRequest;
    bu_asc*         _pacls;
    int             _uidsize;
    int             _reqsz;
    WriteRequest    _prepareWQ;
    GHandler*       _indicator;
    bool            _bread_request;
};

#endif // NO_GATT_H
