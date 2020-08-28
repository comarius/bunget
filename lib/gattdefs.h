/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef GATT_DEFS_H
#define GATT_DEFS_H

#include <stdint.h>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include "include/libbunget.h"
#include "uguid.h"
#include "bybuff.h"


#define ATT_OP_ERROR                     0x01
#define ATT_OP_MTU_REQ                   0x02
#define ATT_OP_MTU_RESP                  0x03
#define ATT_OP_FIND_INFO_REQ             0x04
#define ATT_OP_FIND_INFO_RESP            0x05
#define ATT_OP_FIND_BY_TYPE_REQ          0x06
#define ATT_OP_FIND_BY_TYPE_RESP         0x07
#define ATT_OP_READ_BY_TYPE_REQ          0x08
#define ATT_OP_READ_BY_TYPE_RESP         0x09
#define ATT_OP_READ_REQ                  0x0a
#define ATT_OP_READ_RESP                 0x0b
#define ATT_OP_READ_BLOB_REQ             0x0c
#define ATT_OP_READ_BLOB_RESP            0x0d
#define ATT_OP_READ_MULTI_REQ            0x0e
#define ATT_OP_READ_MULTI_RESP           0x0f
#define ATT_OP_READ_BY_GROUP_REQ         0x10
#define ATT_OP_READ_BY_GROUP_RESP        0x11
#define ATT_OP_WRITE_REQ                 0x12
#define ATT_OP_WRITE_RESP                0x13
#define ATT_OP_WRITE_CMD                 0x52
#define ATT_OP_PREP_WRITE_REQ            0x16
#define ATT_OP_PREP_WRITE_RESP           0x17
#define ATT_OP_EXEC_WRITE_REQ            0x18
#define ATT_OP_EXEC_WRITE_RESP           0x19
#define ATT_OP_HANDLE_NOTIFY             0x1b
#define ATT_OP_HANDLE_IND                0x1d
#define ATT_OP_HANDLE_CNF                0x1e
#define ATT_OP_SIGNED_WRITE_CMD          0xd2

#define GATT_PRIM_SVC_UUID               0x2800
#define GATT_INCLUDE_UUID                0x2802
#define GATT_CHARAC_UUID                 0x2803

#define GATT_CLIENT_CHARAC_CFG_UUID      0x2902
#define GATT_SERVER_CHARAC_CFG_UUID      0x2903

#define ATT_ECODE_SUCCESS                0x00
#define ATT_ECODE_INVALID_HANDLE         0x01
#define ATT_ECODE_READ_NOT_PERM          0x02
#define ATT_ECODE_WRITE_NOT_PERM         0x03
#define ATT_ECODE_INVALID_PDU            0x04
#define ATT_ECODE_AUTHENTICATION         0x05
#define ATT_ECODE_REQ_NOT_SUPP           0x06
#define ATT_ECODE_INVALID_OFFSET         0x07
#define ATT_ECODE_AUTHORIZATION          0x08
#define ATT_ECODE_PREP_QUEUE_FULL        0x09
#define ATT_ECODE_ATTR_NOT_FOUND         0x0a
#define ATT_ECODE_ATTR_NOT_LONG          0x0b
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE   0x0c
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN   0x0d
#define ATT_ECODE_UNLIKELY               0x0e
#define ATT_ECODE_INSUFF_ENC             0x0f
#define ATT_ECODE_UNSUPP_GRP_TYPE        0x10
#define ATT_ECODE_INSUFF_RESOURCES       0x11
#define MAX_GATTLEN                      32
using namespace bunget;

/******************************************************************************
*/
struct Beacon
{
    Cguid       uid;
    uint16_t    minor;
    uint16_t    major;
    uint8_t     power;
    uint8_t     data[32];
    uint8_t     length;
};


/******************************************************************************
*/
class SrvDevice;
class GHandler : public IHandler
{
    friend class bu_gatt;
public:
    GHandler(){::memset(this,0,sizeof(*this));};
    GHandler(H_ATT_TYPE typ, SrvDevice* ps, uint16_t p, uint16_t uuid);
    GHandler(H_ATT_TYPE typ, SrvDevice* ps, uint16_t p, const bt_uuid_t& uuid);
    virtual    ~GHandler();
    IHandler* add_descriptor(uint16_t uid, uint8_t prop, uint8_t* value, int len);
    IHandler* add_descriptor(const bt_uuid_t& uid, uint8_t prop, uint8_t* value, int len);
    IHandler* get_parent()const;
    IService* get_service()const;
    H_ATT_TYPE get_type()const;
    uint16_t get_16uid()const;
    uint16_t get_handle()const;
    const bt_uuid_t& get_128uid()const;
    uint8_t get_props()const;
    uint8_t get_perms()const;
    uint8_t get_format()const;
    uint8_t get_length()const;
    const uint8_t* get_value()const;
    int put_value(const uint8_t* v, size_t len);

private:
    int _put_value(const uint8_t* v, size_t len);
    int _put_value(const bybuff& buff);

public:
    H_ATT_TYPE      _type;
    uint16_t        _hparent;
    bool            _is128;
    uint16_t        _hndl;
    Cguid           _cuid;
    uint8_t         _props;
    uint8_t         _secure;
    uint8_t         _format;
    uint16_t        _hvalue;
    uint8_t         _length;
    uint8_t*        _value;
    uint32_t        _internal;
    SrvDevice*      _srv;
};

/******************************************************************************
*/
class GattSrv : public IService, public GHandler
{
public:
    GattSrv(uint16_t uuid, SrvDevice* p):GHandler(H_SRV, p, 0, uuid),_default(false){}
    GattSrv(const bt_uuid_t& uuid, SrvDevice* p):GHandler(H_SRV, p, 0, uuid),_default(false){}
    virtual ~GattSrv(){}
    IHandler*    add_charact(uint16_t uid, uint8_t prop, uint8_t perms, uint8_t format, uint8_t length, uint8_t *val=0);
    IHandler*    add_charact(const bt_uuid_t& uid, uint8_t prop, uint8_t perms, uint8_t format, uint8_t length, uint8_t *val=0);
    IHandler*    get_charact(const bt_uuid_t& uid);
    IHandler*    get_charact(uint32_t uid);
    const bt_uuid_t&   get_uid()const;
    void        debug();
private:
    GHandler*    _add_charct_value(uint16_t handle, uint32_t uid,  uint8_t length, uint8_t *val=0);
    GHandler*    _add_charct_value(uint16_t handle, const bt_uuid_t& uuid, uint8_t length, uint8_t *val=0);
    void         _ctor(GHandler* pci, uint8_t props, uint8_t secure, uint8_t format, uint8_t length, uint8_t *val);

public:
    uint16_t        _lasthndl;
    std::string     _name;
    bool            _default;
};


#endif //GATT_DEFS_H
