/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/


#include <vector>
#include "gattdefs.h"
#include "libbungetpriv.h"

/****************************************************************************************
*/
IHandler*    GattSrv::add_charact(uint16_t uid, uint8_t props, uint8_t secure, uint8_t format, uint8_t length, uint8_t *val)
{
    GHandler* pci = new GHandler(H_CHR, _srv, this->_hndl, uid);
    _ctor(pci, props,secure,format,length,val);

    GHandler* pcv = this->_add_charct_value(pci->_hndl, uid, length, val);
    pci->_hvalue = pcv->_hndl;
    pcv->_hparent = pci->_hndl;
    if(props & (PROPERTY_NOTIFY|PROPERTY_INDICATE))
    {
        uint8_t  val[]={0x00,0x00};
        GHandler* pd = (GHandler*)pci->add_descriptor(0x2902,PROPERTY_READ|PROPERTY_WRITE_NO_RESPONSE|PROPERTY_WRITE,(uint8_t*)val,2);
        pd->_hparent = pci->_hndl;
        this->_lasthndl = pd->_hndl; //mcoo
    }
    return pci;
}

/****************************************************************************************
*/
IHandler*    GattSrv::add_charact(const bt_uuid_t& uid, uint8_t props, uint8_t secure, uint8_t format, uint8_t length, uint8_t *val)
{
    GHandler* pci = new GHandler(H_CHR, _srv, this->_hndl, uid);
    _ctor(pci, props,secure,format,length,val);

    GHandler* pcv = this->_add_charct_value(pci->_hndl, uid, length, val);
    pci->_hvalue = pcv->_hndl;
    pcv->_hparent = pci->_hndl;
    if(props & (PROPERTY_NOTIFY|PROPERTY_INDICATE))
    {
        uint8_t  val[]={0x00,0x00};
        GHandler* pd = (GHandler*)pci->add_descriptor(0x2902,PROPERTY_READ|PROPERTY_WRITE_NO_RESPONSE|PROPERTY_WRITE,(uint8_t*)val,2);
        pd->_hparent = pci->_hndl;
    }
    return pci;
}

/****************************************************************************************
*/
void GattSrv::_ctor(GHandler* pci, uint8_t props, uint8_t secure, uint8_t format, uint8_t length, uint8_t *val)
{
    if(pci->_type==0)pci->_type   = H_CHR;
    pci->_srv    = this->_srv;
    pci->_props  = props;
    pci->_secure = secure;
    pci->_format = format;
    pci->_hndl   = _srv->add_gattel(pci);
    this->_lasthndl = pci->_hndl; //mcooo
    pci->_hparent = this->_hndl;
    pci->_length = length;
    pci->_value = new uint8_t[MAX_GATTLEN];
    ::memset(pci->_value,0,MAX_GATTLEN);
    if(val)
    {
        ::memcpy(pci->_value,val,length);
    }
}

/****************************************************************************************
*/
GHandler*    GattSrv::_add_charct_value(uint16_t handle, uint32_t uid, uint8_t length, uint8_t *val)
{
    GHandler* pciv = new GHandler(H_CHR_VALUE, _srv, _hndl, uid);
    if(pciv->_type==0)pciv->_type  = H_CHR_VALUE;
    pciv->_srv   =  this->_srv;
    pciv->_length=length;
    if(length){
        pciv->_value = new uint8_t[MAX_GATTLEN];
        ::memset(pciv->_value,0,MAX_GATTLEN);
        if(val)
        {
            ::memcpy(pciv->_value,val,length);
        }
    }
    pciv->_hndl = _srv->add_gattel(pciv);
    this->_lasthndl = pciv->_hndl; //mcooo
    return pciv;
}

/****************************************************************************************
*/
GHandler*    GattSrv::_add_charct_value(uint16_t handle, const bt_uuid_t& uuid, uint8_t length, uint8_t *val)
{
    GHandler* pciv = new GHandler(H_CHR_VALUE, _srv, _hndl, uuid);
    if(pciv->_type==0)pciv->_type  = H_CHR_VALUE;
    pciv->_srv   =  this->_srv;
    pciv->_length=length;
    if(length)
    {
        pciv->_value = new uint8_t[MAX_GATTLEN];
        ::memset(pciv->_value,0,MAX_GATTLEN);
        if(val)
        {
            ::memcpy(pciv->_value,val,length);
        }
    }
    pciv->_hndl = _srv->add_gattel(pciv);
    this->_lasthndl = pciv->_hndl; //mcooo
    return pciv;
}

/****************************************************************************************
*/
IHandler*    GattSrv::get_charact(const bt_uuid_t& uid)
{
    return 0;
}

/****************************************************************************************
*/
IHandler*    GattSrv::get_charact(uint32_t uid)
{
    return 0;
}

/****************************************************************************************
*/
const bt_uuid_t&   GattSrv::get_uid()const
{
    return _cuid._u128;
}

void        GattSrv::debug()
{
    TRACE("---------------------------------");
    TRACE("service" << _name);
    uint16_t uid = this->_cuid.as16();
    TRACE("UUID:"<< std::hex <<  uid << std::dec);
    TRACE("LAST HANDLE:"<< _lasthndl);
    TRACE("HANDLE:"<< _hndl);

}

/****************************************************************************************
*/
GHandler::GHandler(H_ATT_TYPE typ, SrvDevice* ps, uint16_t hp, uint16_t uuid):_type(typ),
                                                                              _hparent(hp),
                                                                              _is128(false),
                                                                              _hndl(0),
                                                                              _cuid(uuid),
                                                                              _props(0),
                                                                              _secure(0),
                                                                              _format(0),
                                                                              _hvalue(0),
                                                                              _length(0),
                                                                              _value(0),
                                                                              _internal(0),
                                                                              _srv(ps)
{

}

/****************************************************************************************
*/
GHandler::GHandler(H_ATT_TYPE typ, SrvDevice* ps, uint16_t hp, const bt_uuid_t& uuid):_type(typ),
                                                                                      _hparent(hp),
                                                                                      _is128(true),
                                                                                      _hndl(0),
                                                                                      _cuid(uuid),
                                                                                      _props(0),
                                                                                      _secure(0),
                                                                                      _format(0),
                                                                                      _hvalue(0),
                                                                                      _length(0),
                                                                                      _value(0),
                                                                                      _internal(0),
                                                                                      _srv(ps)
{

}

/****************************************************************************************
*/
GHandler::~GHandler()
{
    delete []_value;
}

/****************************************************************************************
*/
H_ATT_TYPE GHandler::get_type()const
{
    return _type;
}

/****************************************************************************************
*/
IHandler* GHandler::get_parent()const
{
    return _srv->gatel(_hparent);

}

/****************************************************************************************
*/
IHandler* GHandler::add_descriptor(uint16_t uid, uint8_t prop, uint8_t* value, int len)
{
    GHandler* p =  new GHandler(H_DSC, _srv, this->_hndl, uid);
    if(len)
    {
        p->_value = new uint8_t[MAX_GATTLEN];
        ::memset(p->_value,0,MAX_GATTLEN);
        if(value)
            ::memcpy(p->_value,value,len);
    }
    p->_length = len;
    p->_props = prop;
    p->_hndl = _srv->add_gattel(p);
    return p;
}

/****************************************************************************************
*/
IHandler* GHandler::add_descriptor(const bt_uuid_t& uid, uint8_t prop, uint8_t* value, int len)
{
    GHandler* p =  new GHandler(H_DSC, _srv, this->_hndl, uid);
    if(len)
    {
        p->_value = new uint8_t[MAX_GATTLEN];
        ::memset(p->_value,0,MAX_GATTLEN);
        if(value)
            ::memcpy(p->_value,value,len);
    }
    p->_length = len;
    p->_props = prop;
    p->_hndl = _srv->add_gattel(p);
    return p;
}

/****************************************************************************************
*/
IService* GHandler::get_service()const
{
    return (IService*)_srv;
}

/****************************************************************************************
*/
uint16_t GHandler::get_16uid()const
{
    return _cuid.as16();
}

/****************************************************************************************
*/
const bt_uuid_t& GHandler::get_128uid()const
{
    return _cuid._u128;
}

/****************************************************************************************
*/
uint8_t GHandler::get_props()const
{
    return _props;
}

uint16_t GHandler::get_handle()const
{
    return this->_hndl;
}

/****************************************************************************************
*/
uint8_t GHandler::get_perms()const
{
    return _secure;
}

/****************************************************************************************
*/
uint8_t GHandler::get_format()const
{
    return _format;
}

/****************************************************************************************
*/
uint8_t GHandler::get_length()const
{
    return _length;
}

/****************************************************************************************
*/
int GHandler::_put_value(const bybuff& buff)
{
    return _put_value(buff.buffer(), buff.length());
}

/****************************************************************************************
*/
int GHandler::_put_value(const uint8_t* v, size_t length)
{
    if(length==0)  return 0;
    if(_value==0)
    {
        _value = new uint8_t[MAX_GATTLEN];
        ::memset(_value,0,MAX_GATTLEN);
    }
    _length = std::min(length, (size_t)MAX_GATTLEN);
    ::memcpy(_value, v, _length);
    return length;
}

/****************************************************************************************
*/
int  GHandler::put_value(const uint8_t* data, size_t length)
{
    if(length==0)  return 0;
    _put_value(data,length);
    if(_type==H_CHR ||_type==H_CHR_VALUE)
    {
        _srv->write_charact(this);
    }
    else
    {
        _THROW(" not supported yet");
        return 0;
    }
    return _length;
}

/****************************************************************************************
*/
const uint8_t* GHandler::get_value()const
{
    return _value;
}

