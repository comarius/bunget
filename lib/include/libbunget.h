/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This program, or portions of it cannot be used in commercial
    products without the written consent of the author: marrius9876@gmail.com

*/
#ifndef _LIB_bunget_H_
#define _LIB_bunget_H_

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <cerrno>
#include "uuid.h"
#include "bluetooth.h"

#define  LIBBUNGET_VERSION_STRING           "bunget library, version: 1.2.1, Nov 22 2017, © 2017-Zyrexix.Inc"

/// these are the mumbo jumbo protocol shitty properties
#define  VERSION_LIB_BUNGET                 100
#define  PROPERTY_BROADCAST                 0x1 //
#define  PROPERTY_READ                      0x2 //
#define  PROPERTY_WRITE_NO_RESPONSE         0x4 //
#define  PROPERTY_WRITE                     0x8 //  client chnages values in the server
#define  PROPERTY_NOTIFY                    0x10 // ATT_NOTIFYCATION + value. client no ack back as  PROPERTY_WRITE_NO_RESPONSE
#define  PROPERTY_INDICATE                  0x20 // requires a ack from client, as confirmation. server cannot send another indicator
                                                 // untill receives confirmation
#define  PROPERTY_SIGNED_WRITE              0x40 //
#define  PROPERTY_EXTENDED_PROPS            0x80 //


#define  SECURITY_BROADCAST                 0x1
#define  SECURITY_READ                      0x2
#define  SECURITY_WRITE_NO_RESPONSE         0x4
#define  SECURITY_WRITE                     0x8
#define  SECURITY_NOTIFY                    0x10
#define  SECURITY_INDICATE                  0x20
#define  SECURITY_SIGNED_WRITE              0x40
#define  SECURITY_EXTENDED_PROPS            0x80

#define  PERMISSION_NA                      0
#define  PERMISSION_READ                    1
#define  PERMISSION_READ_ENCRYPTED          2
#define  PERMISSION_READ_ENCRYPTED_MITM     4
#define  PERMISSION_WRITE                   16
#define  PERMISSION_WRITE_ENCRYPTED         32
#define  PERMISSION_WRITE_ENCRYPTED_MITM    64
#define  PERMISSION_WRITE_SIGNED            128
#define  PERMISSION_WRITE_SIGNED_MITM       256

#define  WRITE_TYPE_DEFAULT                 2
#define  WRITE_TYPE_NO_RESPONSE             1
#define  WRITE_TYPE_SIGNED  4

#define  FORMAT_UINT8       17
#define  FORMAT_UINT16      18
#define  FORMAT_UINT32      20
#define  FORMAT_SINT8       33
#define  FORMAT_SINT16      34
#define  FORMAT_SINT32      36
#define  FORMAT_SFLOAT      50
#define  FORMAT_FLOAT       52
#define  FORMAT_RAW         0

#define  FORMAT_UINT8_LEN       1
#define  FORMAT_UINT16_LEN      2
#define  FORMAT_UINT32_LEN      4
#define  FORMAT_SINT8_LEN       1
#define  FORMAT_SINT16_LEN      2
#define  FORMAT_SINT32_LEN      4
#define  FORMAT_SFLOAT_LEN      2
#define  FORMAT_FLOAT_LEN       4
#define  FORMAT_RAW_LEN         20


#define PRIMARY_SERVICE_UUID                       (0x2800)
#define SECONDARY_SERVICE_UUID                     (0x2801)
#define INCLUDE_SERVICE_UUID                       (0x2802)
#define CHARACTERISTIC_UUID                        (0x2803)
#define CHAR_EXTENDED_PROP_DESC_UUID               (0x2900)
#define CHAR_USER_DESC_UUID                        (0x2901)
#define CHAR_CLIENT_CONFIG_DESC_UUID               (0x2902)
#define CHAR_SERVER_CONFIG_DESC_UUID               (0x2903)
#define CHAR_FORMAT_DESC_UUID                      (0x2904)
#define CHAR_AGGR_FMT_DESC_UUID                    (0x2905)
#define GATT_SERVICE_UUID                          (0x1801)
#define GAP_SERVICE_UUID                           (0x1800)
#define SERVICE_CHANGED_UUID                       (0x2A05)


namespace bunget
{
typedef enum H_ATT_TYPE {H_NOTYPE=0,
                         H_SRV=0x1,
                         H_SRV_INC=0x2,
                         H_CHR=0x4,
                         H_CHR_VALUE=0x8,
                         H_DSC=0x10,
                         H_ATTRIB=0x20
                        } H_ATT_TYPE;

enum E_BT_MODE {eCLIENT, eSERVER};


class hexecption
{
public:

    hexecption(const char* f, const char* fu, int err, const char* ex=0):_f(f),_fu(fu),_ex(err)
    {
        if(ex)
        {
            _usr=ex;
        }

    };
    std::string report()
    {
        char se[8];
        ::sprintf(se,"%d",_ex);
        std::string rep = "Exception: ";
        rep += _usr;
        rep +=  " ";
        rep += _f;
        rep += ", ";
        rep += _fu;
        rep += ", ";
        rep += se;
        return rep;
    }
    std::string _f;
    std::string _fu;
    std::string _usr;
    int         _ex;
};

class IService;
class IHandler
{
public:
    virtual ~IHandler() {}

    virtual IHandler* add_descriptor(uint16_t uid, uint8_t prop, uint8_t* value, int len)=0;
    virtual IHandler* add_descriptor(const bt_uuid_t& uid, uint8_t prop, uint8_t* value, int len)=0;
    virtual IService* get_service()const=0;
    virtual uint16_t get_16uid()const=0;
    virtual uint16_t get_handle()const=0;
    virtual const bt_uuid_t& get_128uid()const=0;
    virtual uint8_t get_props()const=0;
    virtual uint8_t get_perms()const=0;
    virtual uint8_t get_format()const=0;
    virtual uint8_t get_length()const=0;
    virtual const uint8_t* get_value()const=0;
    virtual int put_value(const uint8_t* v, size_t len)=0;
    virtual H_ATT_TYPE get_type()const=0;
};

class IService
{
public:
    virtual ~IService() {}
    virtual IHandler*    add_charact(uint16_t uid, uint8_t prop, uint8_t perms, uint8_t format, uint8_t length, uint8_t *val=0)=0;
    virtual IHandler*    add_charact(const bt_uuid_t& srvid, uint8_t prop, uint8_t perms, uint8_t format, uint8_t length, uint8_t *val=0)=0;
    virtual IHandler*    get_charact(const bt_uuid_t& srvid)=0;
    virtual IHandler*    get_charact(uint32_t uid)=0;
    virtual const bt_uuid_t&   get_uid()const=0;
};

class HciDev
{
public:
    std::string _mac;
    std::string _name;
    uint8_t     _props;
};


class ISrvProc;
class IServer
{
public:
    enum S_STATE{
        eOFFLINE,
        eINITALISING,
        eRUNNING,
        eUNKNOWN,
    };

    virtual ~IServer() {}
    virtual void    power_switch(bool on)=0;
    virtual int advertise(int millisecs)=0;
    virtual int adv_beacon(const char* suid, uint16_t minor, uint16_t major, int8_t power, uint16_t manid, const uint8_t* data, uint8_t length)=0;  ///returns device hci# or -1
    virtual IService* add_service(uint16_t srvid, const char* name)=0;
    virtual IService* add_service(const bt_uuid_t& srvid, const char* name)=0;
    virtual IService* get_service(const bt_uuid_t& srvid)=0;
    virtual IService* get_service(uint16_t srvid)=0;
    virtual void run()=0;
    virtual void refresh()=0;
    virtual void get_version(uint8_t& hciver, uint16_t& hcirev, uint8_t& lmpver, uint16_t& man, uint16_t& lmpsubver)=0;
    virtual const bdaddr_t& get_baddr()const = 0;
    virtual void stop()=0;
    virtual uint8_t rssi()const=0;
    virtual uint16_t handle()const=0;
    virtual int set_name(const char* btname)=0;
    virtual IServer::S_STATE status()const = 0;
    virtual size_t  nServices()const     = 0;

};

class Icryptos;
class ISrvProc
{
public:
    virtual ~ISrvProc() {}
    virtual Icryptos* get_crypto()=0;
    virtual bool initHciDevice(int devid, const char* name)=0;
    virtual void onServicesDiscovered(std::vector<IHandler*>& els)=0;
    virtual bool onSpin(IServer* ps, uint16_t notyUuid)=0;
    virtual void onReadRequest(IHandler* pc)=0;
    virtual int  onSubscribesNotify(IHandler* pc, bool b)=0;
    virtual void onWriteRequest(IHandler* pc)=0;
    virtual void onWriteDescriptor(IHandler* pc, IHandler* pd)=0;
    virtual void onIndicate(IHandler* pc)=0;
    virtual void onAdvertized(bool onoff)=0;
    virtual void onDeviceStatus(bool onoff)=0;
    virtual void onStatus(const HciDev* device)=0;
};

class Icryptos;
class BtCtx
{
public:
    BtCtx();
    virtual ~BtCtx();

    static BtCtx* instance();
    virtual IServer* new_server(ISrvProc* proc, int hcidev, const char* name, int tweak_delay=0, bool advall=0, bool defaults = true)=0;
};

/**
    helper for reading writing
*/
class GattRw
{
public:
    GattRw(IHandler* ph):_ph(ph){}

    template <class T>int write(const T& val)
    {
        return _ph->put_value(&val, sizeof(val));
    }

    template <class T>
    T read()
    {
        uint8_t* pval = _ph->get_value();
        uint8_t len = _ph->get_length();
        uint8_t fmt = _ph->get_type();
        switch(fmt)
        {
            case FORMAT_UINT8:
                return uint8_t(*pval);
            case FORMAT_UINT16:
                return uint16_t(*pval);
            case FORMAT_UINT32:
                return uint32_t(*pval);
            case FORMAT_SINT8:
                return int8_t(*pval);
            case FORMAT_SINT16:
                return int16_t(*pval);
            case FORMAT_SINT32:
                return int32_t(*pval);
            case FORMAT_SFLOAT:
                {
                    uint16_t hfl = uint16_t(*pval);
                    float f = ((hfl&0x8000)<<16) | (((hfl&0x7c00)+0x1C000)<<13) | ((hfl&0x03FF)<<13);
                    return f;
                }
            case FORMAT_FLOAT:
                {
                    return float(*pval);
                }
            case FORMAT_RAW:
                return pval;
            default:
                break;
        }
        return uint32_t(0);
    }

private:
    IHandler* _ph;
};


};

#endif //_LIB_bunget_H_
