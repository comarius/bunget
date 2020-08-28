/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef LIB_BULEX_PRIV
#define LIB_BULEX_PRIV


#include <string>
#include <vector>
#include "gattdefs.h"
#include "bu_hci.h"
#include "include/libbunget.h"
#include "include/icryptos.h"


/******************************************************************************
*/
class   SrvDevice;
class   ascon;
class   bu_gap;
class   bu_asc;

/******************************************************************************
*/
typedef std::vector<GHandler*>    Harray;

/******************************************************************************
*/
enum {
    INTERN_SUBSCRIBED = 0x1,
};

/******************************************************************************
*/

class SrvDevice : public IServer, public hci_event
{
public:
    // defaults enables the default services 1800 and 1801. This just works with 16 bit UUID services
    SrvDevice(ISrvProc* proc, int& hcid, const char* name, int delay=0, bool advall=false, bool defaults = true);
    ~SrvDevice();
    void   power_switch(bool on);
    int    advertise(int millis);
    IService* add_service(uint16_t srvid, const char* name);
    IService* add_service(const bt_uuid_t& srvid, const char* name);

    int adv_beacon(const char* suid, uint16_t minor, uint16_t major, int8_t power, uint16_t manid, const uint8_t* data, uint8_t length);

    IService* get_service(const bt_uuid_t& srvid);
    IService* get_service(uint16_t srvid);
    void run();//uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    int write_charact(GHandler* ph);
    int write_descr(GHandler* ph);
    void get_version(uint8_t& hciver, uint16_t& hcirev, uint8_t& lmpver, uint16_t& man, uint16_t& lmpsubver);
    const bdaddr_t& get_baddr()const;

    virtual uint8_t rssi()const{return _rssi;}
    virtual uint16_t handle()const{return  _handle;}
    virtual void refresh();
    virtual void stop();
    virtual int set_name(const char* btname);
    virtual void set_adv_interval(int intetv){_advinterval = intetv;}
    /// hci_event //
    virtual void on_disconnect(const evt_disconn_complete* evdc);
    virtual void on_encrypt_chnage(const evt_encrypt_change* pecc);
    virtual void on_acl_packet(uint16_t handle, uint16_t cid, const sdata& data);
    virtual void on_read_version(uint8_t hciver, uint16_t hcirev, uint8_t lmpver, uint16_t man, uint16_t lmpsubver);
    virtual void on_mac_change(const bdaddr_t& addr);
    virtual void on_adv_status(HCI_STATE status);
    virtual void on_adv_data_status(uint8_t status);
    virtual void on_scan_resp_datat_status(uint8_t status);
    virtual void on_adv_enable(uint8_t status);
    virtual void on_rssi(uint16_t handle, uint8_t rssi);
    virtual void le_ltk_neg_reply(uint16_t handle);
    virtual void le_get_adv_interval(int& interval)const;
    virtual void on_le_connected(uint8_t status,
                                uint16_t handle, uint8_t role,
                                HCI_ADDRTYPE addressType,
                                const bdaddr_t& address,
                                uint16_t interval,
                                uint16_t latency,
                                uint16_t  supervisionTimeout=0,
                                uint8_t masterClockAccuracy=0);
    virtual void on_le_conn_update_complette_shit(uint8_t status, uint16_t handle, uint16_t interval, uint16_t latency, uint16_t supervisionTimeout);
    virtual void on_dev_status(bool onoff);
    virtual bool onSpin();
    virtual void onAdvertized(bool onoff);
    virtual size_t  nServices()const;
    virtual S_STATE status()const{return _status;};
//======================================
    virtual void on_configure_device(int devid);
    GattSrv* first_service(){
        if(_services.size())
            return (GattSrv*)_services[0];
        return 0;
    };
    GHandler* gatel(uint16_t handler)
    {
        size_t index= size_t(handler-1);
        if(index < _handles.size())
        {
            return _handles[index];
        }
        return 0;
    }

    size_t add_gattel(GHandler* pel){
        _handles.push_back(pel);
        return _handles.size();
    }

    const std::vector<IService*> & services()const {return _services;};
    Harray& handlers(){return _handles;};

    void    data_subscribe(hci_data_eater* eat){
        data_unsubscribe(eat);
        _eaters.push_back(eat);
    }
    void    data_unsubscribe(hci_data_eater* eat){
        if(_eaters.size()==0)
            return;
        std::vector<hci_data_eater*>::iterator b = _eaters.begin();
        for( ; b!=_eaters.end();b++)
        {
            if(*b==eat)
            {
                _eaters.erase(b);
                break;
            }
        }
    }
    int    feed_them(uint8_t opt, const sdata& data);
    uint16_t add(GHandler* g);
    void add_default_service();

private:
    uint16_t    _poolNextNotyHndl();

public:
    ISrvProc*    _cb_proc;

private:
    bool         _def;
    bu_gap*      _gapp;
    bu_gatt*     _gatt;
    bu_hci*      _hci;
    Icryptos*   _pcrypt;
    int         _hcidev;
    std::string _name;
    int         _pin;
    bdaddr_t    _address;
    uint16_t    _handle;
    bool        _running;
    bu_asc*     _pacl;
    Harray      _handles;
    uint8_t     _hciver;
    uint16_t    _hcirev;
    uint8_t     _lmpver;
    uint16_t    _man;
    uint16_t    _lmpsubver;
    uint8_t     _rssi;
    bdaddr_t    _baddr;
    HCI_STATE   _hcistatus;
    uint8_t     _advstatus;
    uint8_t     _scanrespdatastatus;
    HciDev      _remote;
    bool        _defaults;
    S_STATE     _status;
    int         _respdelay;
    int         _advinterval;
    int         _maxMtu;
    bool        _advall;
    size_t      _notyinterval;
    size_t      _notytime;
    uint16_t    _curnoty;
    std::vector<hci_data_eater*> _eaters;
    std::vector<IService*>      _services;
};


class ContextImpl:public BtCtx
{
public:
    ContextImpl(){}
    virtual ~ContextImpl(){

        for(auto& a : _adapters)
            delete a.second;

    }
    virtual IServer* new_server(ISrvProc* proc, int hcidev, const char* name, int tweak_delay=0, bool advall=false, bool defaults = true);
 private:
    std::map<int,IServer*> _adapters;
};
extern ContextImpl* Ctx;


#endif //LIB_BULEX_PRIC
