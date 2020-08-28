/**
    Copyright: O-Marius Chincisan 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    products without the written consent of the author.

*/
#include <iostream>
#include <assert.h>
#include "gattdefs.h"
#include "bu_gatt.h"
#include "ascon.h"
#include "libbungetpriv.h"

/****************************************************************************************
*/
typedef void  (bu_gatt::*phndl)(const sdata& data);

/****************************************************************************************
*/
bu_gatt::bu_gatt(bu_hci* hci):_hci(hci),_indicator(0)
{
    reset();
}

/****************************************************************************************
*/
void    bu_gatt::reset()
{
    _maxMtu  =  256;
    _mtu     =  23;
    _preparedWriteRequest  =  0;
    _uidsize = 2;
    _bread_request=false;
}

/****************************************************************************************
*/
bu_gatt::~bu_gatt()
{
    //Ctx->Srv(hci->dev_id())->data_unsubscribe(this);
}

/****************************************************************************************
*/
static const char* type2string(uint8_t type)
{
    switch(type)
    {
    case ATT_OP_ERROR             : //  0x01;
        return " ATT_OP_ERROR";
    case ATT_OP_MTU_REQ           : //  0x02;
        return "ATT_OP_MTU_REQ";
    case ATT_OP_MTU_RESP          : //  0x03;
        return "ATT_OP_MTU_RESP";
    case ATT_OP_FIND_INFO_REQ     : //  0x04;
        return "ATT_OP_FIND_INFO_REQ";
    case ATT_OP_FIND_INFO_RESP    : //  0x05;
        return "ATT_OP_FIND_INFO_RESP";
    case ATT_OP_FIND_BY_TYPE_REQ  : //  0x06;
        return "OP_FIND_BY_TYPE_REQ";
    case ATT_OP_FIND_BY_TYPE_RESP : //  0x07;
        return "ATT_OP_FIND_BY_TYPE_RESP";
    case ATT_OP_READ_BY_TYPE_REQ  : //  0x08;
        return "ATT_OP_READ_BY_TYPE_REQ";
    case ATT_OP_READ_BY_TYPE_RESP : //  0x09;
        return "ATT_OP_READ_BY_TYPE_RESP";
    case ATT_OP_READ_REQ          : //  0x0a;
        return "ATT_OP_READ_REQ";
    case ATT_OP_READ_RESP         : //  0x0b;
        return "ATT_OP_READ_RESP";
    case ATT_OP_READ_BLOB_REQ     : //  0x0c;
        return "ATT_OP_READ_BLOB_REQ";
    case ATT_OP_READ_BLOB_RESP    : //  0x0d;
        return "ATT_OP_READ_BLOB_RESP";
    case ATT_OP_READ_MULTI_REQ    : //  0x0e;
        return "ATT_OP_READ_MULTI_REQ";
    case ATT_OP_READ_MULTI_RESP   : //  0x0f;
        return "ATT_OP_READ_MULTI_RESP ";
    case ATT_OP_READ_BY_GROUP_REQ : //  0x10;
        return "ATT_OP_READ_BY_GROUP_REQ ";
    case ATT_OP_READ_BY_GROUP_RESP: //  0x11;
        return "ATT_OP_READ_BY_GROUP_RESP";
    case ATT_OP_WRITE_REQ         : //  0x12;
        return "ATT_OP_WRITE_REQ";
    case ATT_OP_WRITE_RESP        : //  0x13;
        return "ATT_OP_WRITE_RESP";
    case ATT_OP_WRITE_CMD         : //  0x52;
        return "ATT_OP_WRITE_CMD";
    case ATT_OP_PREP_WRITE_REQ    : //  0x16;
        return "ATT_OP_PREP_WRITE_REQ";
    case ATT_OP_PREP_WRITE_RESP   : //  0x17;
        return "ATT_OP_PREP_WRITE_RESP";
    case ATT_OP_EXEC_WRITE_REQ    : //  0x18;
        return "ATT_OP_EXEC_WRITE_RE";
    case ATT_OP_EXEC_WRITE_RESP   : //  0x19;
        return "ATT_OP_EXEC_WRITE_RESP";
    case ATT_OP_HANDLE_NOTIFY     : //  0x1b;
        return "ATT_OP_HANDLE_NOTIFY";
    case ATT_OP_HANDLE_IND        : //  0x1d;
        return "ATT_OP_HANDLE_IND";
    case ATT_OP_HANDLE_CNF        : //  0x1e;
        return "ATT_OP_HANDLE_CNF";
    default:
        break;
    };
    return "ATT_NOT_KNOWN";
}

/****************************************************************************************
*/
void bu_gatt::setAclPtr(bu_asc* pacls)
{
    _pacls = pacls;
    if(pacls){
        _hci->srv()->data_subscribe(this);
    }
    else
        _hci->srv()->data_unsubscribe(this);
}

/****************************************************************************************
*/
int  bu_gatt::_dummy_q(const sdata& data, bybuff& b)
{
    return 0;
}

/****************************************************************************************
*/
int bu_gatt::on_sock_data(uint8_t code, const sdata& data)
{
    uint8_t rqt = data.data[0];
    bybuff  ret;
    bybuff   trace(data.data,data.len);

    switch(rqt)
    {
        case ATT_OP_MTU_REQ://2
        {
            uint16_t mtu = oa2t<uint16_t>(data.data,1);
	    uint16_t mtu2 = htobs(mtu);
            if (mtu2 < 23)
                mtu2 = 23;
            else if (mtu2 > _maxMtu)
                mtu2 = _maxMtu;
            _mtu = mtu2;
	    TRACE("mtu = " << int(_mtu));
            ret << uint8_t(ATT_OP_MTU_RESP);
            ret << uint16_t(htobs(_mtu));
        }
        break;

    case ATT_OP_FIND_INFO_REQ:
        _info_q(data, ret);
        break;
    case ATT_OP_FIND_BY_TYPE_REQ:
        _find_type_q(data, ret);
        break;
    case ATT_OP_READ_BY_TYPE_REQ:
        _type_q(data,ret);
        break;
    case ATT_OP_READ_REQ:
        _read_q(data, ret);
        break;
    case ATT_OP_READ_BLOB_REQ:
        _read_blob(data, ret);
        break;
    case ATT_OP_READ_BY_GROUP_REQ:
    case ATT_OP_READ_BY_GROUP_RESP:
        _group_q(data, ret);
        break;
    case ATT_OP_WRITE_REQ:
        _write_q(data,ret);
        break;
    case ATT_OP_WRITE_CMD:
        _write_cmd(data,ret);
        break;
    case ATT_OP_PREP_WRITE_REQ:
        _prep_wq(data,ret);
        break;
    case ATT_OP_EXEC_WRITE_REQ:
        _exec_wq(data,ret);
        break;
    case ATT_OP_HANDLE_CNF:
        break;
    case ATT_OP_READ_MULTI_REQ:
    case ATT_OP_SIGNED_WRITE_CMD:
    default:
        break;
    }

    if (ret.length())
    {
        this->_send(ret);
        return 1;
    }
    return 0;
}

/****************************************************************************************
*/
int bu_gatt::_send(const bybuff& data)
{
    _pacls->write(ATT_CID, data);
    return 0;
};

/****************************************************************************************
*/
int  bu_gatt::_reply_err(uint8_t code, uint16_t hndl, uint8_t status, bybuff& buff)
{
    buff.reset();
    buff << uint8_t(ATT_OP_ERROR) << code << hndl << status;
    return 1;
}

/****************************************************************************************
*/
int bu_gatt::_group_q(const sdata& data, bybuff& ret)
{
    uint16_t hs = oa2t<uint16_t>(data.data,1);
    uint16_t he = oa2t<uint16_t>(data.data,3);

    TRACE(__FUNCTION__ << " start h = " << int(hs) << "," << int(he) << "\n");

    Cguid    g(data.data+5);
    std::vector<GattSrv*>   srvs;
    Harray&  handlers = srv()->handlers();
    int      subservices = H_SRV;

	ret.reset();
	if(g.as16() != GATT_PRIM_SVC_UUID)
        	subservices =  H_SRV_INC;

	for(const auto& e : handlers)
	{
        	if(e->_hndl>he)break;
		if(e->_hndl<hs)continue;
		        if(e->_type==subservices)
		{
	            TRACE("adding handler: " << int(e->_hndl) << " type = " << e->_type);
		    srvs.push_back((GattSrv*)e);
		}
	}
	size_t nsrvs = srvs.size();
	if(nsrvs==0)
	{
		return _reply_err(ATT_OP_READ_BY_GROUP_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
	}
	if(g.as16() == GATT_PRIM_SVC_UUID || g.as16() == GATT_INCLUDE_UUID)
    {
        GattSrv* pgel = srvs[0];
        uint8_t  lengthPerService = pgel->_is128 ? 20 : 6;
        size_t   elems = std::min(size_t(( _mtu - 2 ) / lengthPerService), nsrvs);
        elems = std::min(elems, srvs.size());

        ret << uint8_t(ATT_OP_READ_BY_GROUP_RESP);
        ret << uint8_t(lengthPerService);
        TRACE("writing service:" << elems );
        for(const auto& ps : srvs)
        {
            ret << uint16_t(ps->_hndl);
            ret << uint16_t(ps->_lasthndl);

            TRACE(" adding service " << std::hex << int(ps->_hndl) << std::dec<<"\n");

            // can not mix up 128 bit uuids with 16 bit
            assert((lengthPerService == 20 && !ps->_cuid.is_16()) || (lengthPerService == 6 && ps->_cuid.is_16()));
            if(ps->_cuid.is_16()){
                ret << ps->_cuid.as16();
            } else {
                ret << ps->_cuid.as128();
            }


            if(--elems==0)
                break;
        }
    }
    else
    {
        return _reply_err(ATT_OP_READ_BY_GROUP_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
    }

    std::vector<IHandler*> els;
    for(auto& e : handlers)
    {
        els.push_back(e);
    }
    srv()->_cb_proc->onServicesDiscovered(els);
    return 1;
}

/****************************************************************************************
*/
int bu_gatt::_type_q(const sdata& data, bybuff& ret)
{
    uint16_t hs = oa2t<uint16_t>(data.data,1);
    uint16_t he = oa2t<uint16_t>(data.data,3);
    Cguid    g(data.data+5);



    TRACE(__FUNCTION__ << "start h = " << int(hs) << "," << int(he) << "\n");

    ret.reset();
    if (g.as16() == GATT_CHARAC_UUID)
    {
        std::vector<GHandler*>     chrs;
        Harray&                    handlers = srv()->handlers();

        for(const auto& e : handlers)
        {
            if(e->_hndl>he)break;
            if(e->_hndl<hs)continue;
            if(e->_type==H_CHR )
            {
                TRACE("adding characteristics1: " << e->_hndl);
                chrs.push_back(e);
            }
        }
        if(chrs.size()==0)
        {
            return _reply_err(ATT_OP_READ_BY_TYPE_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
        }
        GHandler* pbegin = (GHandler*)chrs[0];
        uint8_t   lengthPerCharacteristic = pbegin->_is128 ? 21 : 7;
        size_t    elems = std::min(chrs.size(), size_t(( _mtu -2 ) / lengthPerCharacteristic));

        ret <<  uint8_t(ATT_OP_READ_BY_TYPE_RESP);
        ret <<  uint8_t(lengthPerCharacteristic);

        for(const auto& pc : chrs)
        {
            ret << uint16_t(pc->_hndl);
            ret << uint8_t(pc->_props);
            ret << uint16_t(pc->_hvalue);

            if(lengthPerCharacteristic==7)
            {
                ret << uint16_t(pc->_cuid.as16());
            }
            else
                ret << pc->_cuid;
            if(--elems==0)
                break;
        }
        return 1;
    }

    bool        atrb = false;
    uint16_t    hvalue = 0;
    bool        secure = false;
    uint16_t    handler = 0;
    Harray&     handlers = srv()->handlers();

    for(const auto& e : handlers)
    {
        if(e->_hndl>he)break;
        if(e->_hndl<hs)continue;
        if((e->_type==H_CHR || e->_type==H_DSC) && g == e->_cuid)
        {
            TRACE("adding characteristics: " << e->_hndl);
            handler = e->_hndl;
            break;
        }
    }
    if(handler==0)
    {
        return _reply_err(ATT_OP_READ_BY_TYPE_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
    }
    GHandler* element = (GHandler*)srv()->gatel(handler);
    if(element->_type==H_CHR)
    {
        atrb = true;
        hvalue = element->_hvalue;
        secure = element->_secure & 0x02;
        element = (GHandler*)srv()->gatel(element->_hvalue);
    }
    else if(element->_type==H_DSC)
    {
        hvalue = element->_hndl;
        secure = element->_secure & 0x02;
    }
    if(element==0)
    {
        return _reply_err(ATT_OP_READ_BY_TYPE_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
    }
    if (secure && _pacls->is_encrypted())
    {
        return _reply_err(ATT_OP_READ_BY_TYPE_REQ, hs, ATT_ECODE_AUTHENTICATION, ret);
    }
    else if (element->_length)
    {
        uint8_t datalen  = std::min((int)element->_length, (int)(this->_mtu-4));
        ret << uint8_t(ATT_OP_READ_BY_TYPE_RESP);
        ret << uint8_t(datalen+2);
        ret << uint16_t(hvalue);
        ret.append(element->_value, element->_length);
    }
    else if(atrb)
    {
        _bread_request=true;
        srv()->_cb_proc->onReadRequest((GHandler*)element);
        _bread_request=false;
        if (element->_length)
        {
            uint8_t datalen  = std::min((int)element->_length, (int)(this->_mtu-4));
            ret << uint8_t(ATT_OP_READ_BY_TYPE_RESP);
            ret << uint8_t(datalen+2);
            ret << uint16_t(hvalue);
            ret.append(element->_value, element->_length);
        }
        return 1;
    }
    return _reply_err(ATT_OP_READ_BY_TYPE_REQ, hs, ATT_ECODE_UNLIKELY, ret);
}

/****************************************************************************************
*/
int bu_gatt::_info_q(const sdata& data, bybuff& ret)
{
    uint16_t hs = oa2t<uint16_t>(data.data,1);
    uint16_t he = oa2t<uint16_t>(data.data,3);


    Harray&  handlers = srv()->handlers();
    Cguid    uuid;
    GHandler*  prev;
    struct  HU
    {
        uint16_t    hndl;
        Cguid        uuid;
    };
    std::vector<HU> infos;
    int             uuidlen = 2;

    for(const auto& e : handlers)
    {
        if(e->_hndl>he)break;
        if(e->_hndl<hs)continue;
        uuid.reset();
        prev=(GHandler*)e;
        switch(e->_type)
        {
        case H_SRV:
            uuid = (uint16_t)0x2800;
            break;
        case H_SRV_INC:
            uuid = (uint16_t)0x2802;
            break;
        case H_CHR:
            uuid = (uint16_t)0x2803;
            break;
        case H_CHR_VALUE:
            if(prev!=e)
                uuid = prev->_cuid;
            else
                uuid = e->_cuid;
            break;
        case H_DSC:
            uuid = e->_cuid;
            break;
        case H_ATTRIB:
            break;
        default:
            break;
        }
        prev=(GHandler*)e;

        if(uuid==0)
            continue;
        HU hu;
        hu.hndl = e->_hndl;
        hu.uuid   = uuid;
        uuidlen   = std::max(uuidlen, uuid.is_16() ? 2 : 16);
        infos.push_back(hu);
    }

    if (infos.size())
    {
        uint8_t lpi = (uuidlen == 2) ? 4 : 18;
        size_t  elems = ((this->_mtu - 2) / lpi);
        elems = std::min(infos.size(), elems);

        ret << uint8_t(ATT_OP_FIND_INFO_RESP);
        ret << uint8_t( (uuidlen == 2) ? 0x01 : 0x2);
        for(const auto& i : infos)
        {
            ret << uint16_t(i.hndl);
            if(uuidlen==2)
                ret << uint16_t(i.uuid.as16());
            else
                ret << i.uuid;
            if(--elems==0)
                break;
        }
        return 1;
    }
    return _reply_err(ATT_OP_FIND_INFO_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
}

/****************************************************************************************
*/
int bu_gatt::write_descr(GHandler* pel)
{
    _THROW("not implemented");
}

/****************************************************************************************
*/
int bu_gatt::writeCharaterisitc(GHandler* pel)
{
    if(_bread_request)
    {
        return 0;
    }
    bybuff  ndata(pel->_value, pel->_length);
    bybuff  ret;

    if(pel->_props & PROPERTY_NOTIFY)
    {
        ret << uint8_t(ATT_OP_HANDLE_NOTIFY);
        ret << uint16_t(pel->_type == H_CHR ? pel->_hndl+1 : pel->_hndl);
        ret << ndata;
        this->_send(ret);
        ret.reset();
        return pel->_length;
    }
    else if(pel->_props & PROPERTY_INDICATE)
    {
        ret << uint8_t(ATT_OP_HANDLE_IND);
        ret << uint16_t(pel->_type == H_CHR ? pel->_hndl+1 : pel->_hndl);
        ret << ndata;
        _indicator = pel;
        this->_send(ret);
        ret.reset();
        return pel->_length;
    }
    return 0;
}

/****************************************************************************************
*/
int bu_gatt::_write_cmd(const sdata& data, bybuff& ret)
{
    return _write_q(data, ret);
}

/****************************************************************************************
*/
int bu_gatt::_write_q(const sdata& data, bybuff& ret)
{
    uint8_t   rqt =  oa2t<uint8_t>(data.data,0);
    bool      withoutResponse = (rqt == ATT_OP_WRITE_CMD);
    uint16_t  vhh  = oa2t<uint16_t>(data.data,1);
    bybuff    vdata(data.data+3, data.len-3);
    GHandler* pel = srv()->gatel(vhh);

    ret.reset();
    if(pel == 0)
    {
        return _reply_err(rqt, vhh, ATT_ECODE_INVALID_HANDLE, ret);
    }

    if(pel->_type==H_CHR_VALUE)
    {
        pel = _hci->srv()->gatel(pel->_hparent);
        assert(pel->_type==H_CHR);
        if(pel == 0)
        {
            return _reply_err(rqt, vhh, ATT_ECODE_INVALID_HANDLE, ret);
        }
    }
    uint8_t props = pel->_props & (withoutResponse ? PROPERTY_WRITE_NO_RESPONSE : PROPERTY_WRITE);
    uint8_t secs  = pel->_secure & (withoutResponse ? PROPERTY_WRITE_NO_RESPONSE : PROPERTY_WRITE);

    if(props == 0)
    {
        return _reply_err(rqt, vhh, ATT_ECODE_WRITE_NOT_PERM, ret);
    }

    if(secs && !this->_pacls->is_encrypted())
    {
        return _reply_err(rqt, vhh, ATT_ECODE_AUTHENTICATION, ret);
    }
    //============================== descriptor or 2902 ===========================
    else if(pel->_type == H_DSC)
    {
        GHandler* valChr = srv()->gatel(pel->_hparent);
        assert(valChr->_type == H_CHR);

        valChr->_put_value(vdata);
        srv()->_cb_proc->onWriteDescriptor(pel, valChr);

        if(pel->_cuid == GATT_CLIENT_CHARAC_CFG_UUID)
        {
            ///remote data
            uint16_t value = oa2t<uint16_t>(vdata.buffer(),0);

            if(value & uint16_t(0x3))  //android writes 0x2 when subscribes to notification.
            {
                if((valChr->_internal & INTERN_SUBSCRIBED)==0) //sometime comes twice
                {
                    srv()->_cb_proc->onSubscribesNotify(valChr, true);
                    valChr->_internal |= INTERN_SUBSCRIBED;
                }
            }
            else
            {
                if((valChr->_internal & INTERN_SUBSCRIBED)!=0)
                {
                    srv()->_cb_proc->onSubscribesNotify(valChr, false);
                    valChr->_internal &= ~INTERN_SUBSCRIBED;
                }
            }
        }
        ret << uint8_t(ATT_OP_WRITE_RESP);
        _send(ret);
        ret.reset();
    }
    else
    {
        pel->_put_value(vdata);
        srv()->_cb_proc->onWriteRequest(pel);

        ret << uint16_t(ATT_OP_WRITE_RESP);
        _send(ret);
        ret.reset();
    }
    return 1;
}

/****************************************************************************************
*/
int bu_gatt::_find_type_q(const sdata& data, bybuff& ret)
{
    uint16_t hs = oa2t<uint16_t>(data.data,1);
    uint16_t he = oa2t<uint16_t>(data.data,3);

    TRACE(__FUNCTION__ << "start h = " << int(hs) << "," << int(he) << "\n");

    Cguid    uuid(data.data+5);
    uint16_t val16 = oa2t<uint16_t>(data.data,3);
    Harray&  handlers = srv()->handlers();
    std::vector<uint16_t> sehandlers;

    ret.reset();
    for(const auto& e : handlers)
    {
        if(e->_hndl>he)break;
        if(e->_hndl<hs)continue;
        if(uuid==0x2800 && e->_type==H_SRV && e->_cuid == val16)
        {
            sehandlers.push_back(e->_hndl);
            sehandlers.push_back(((GattSrv*)e)->_lasthndl);
        }
    }
    if(handlers.size()==0)
    {
        return _reply_err(ATT_OP_FIND_BY_TYPE_REQ, hs, ATT_ECODE_ATTR_NOT_FOUND, ret);
    }

    size_t elems = 2 * std::min(size_t((_mtu - 1) / 4), handlers.size());

    ret << uint8_t(ATT_OP_FIND_BY_TYPE_RESP);
    for(const auto& e : sehandlers)
    {
        ret << e;
        if(--elems==0)
            break;
    }
    return 1;
}

/****************************************************************************************
*/
int bu_gatt::_read_q(const sdata& data, bybuff& ret)
{
    return _read_blob(data, ret);
}

/****************************************************************************************
*/
int bu_gatt::_read_blob(const sdata& data, bybuff& ret)
{
    uint8_t  rqt = data.data[0];
    uint16_t vhh = oa2t<uint16_t>(data.data,1);
    uint16_t offset = rqt==ATT_OP_READ_BLOB_REQ ?
                                    oa2t<uint16_t>(data.data,3) :
                                    0;
    GHandler* element =srv()->gatel(vhh);
    bybuff    dataRet;
    uint8_t   result = 0xFF;

    ret.reset();
    if(0 == element)
    {
        return _reply_err(rqt, vhh, ATT_ECODE_INVALID_HANDLE, ret);
    }
    if(element->_type==H_SRV || element->_type==H_SRV_INC)
    {
        result = ATT_ECODE_SUCCESS;
        dataRet << element->_cuid;
    }
    else if(element->_type==H_CHR)
    {
        result = ATT_ECODE_SUCCESS;
        dataRet << uint8_t(element->_props);
        dataRet << uint16_t(element->_hvalue);
        dataRet << element->_cuid;
    }
    else if(element->_type==H_CHR_VALUE || element->_type==H_DSC)
    {
        uint8_t props = element->_props;
        uint8_t secure = element->_secure;
        GHandler* charel = element;

        if(element->_type==H_CHR_VALUE)
        {
            charel = srv()->gatel(element->_hparent);
            props = charel->_props;
            secure = charel->_secure;
        }
        if(props & 0x02)
        {
            if(_pacls->is_encrypted() && secure == 0x02)
            {
                result = ATT_ECODE_AUTHENTICATION;
            }
            else
            {
                _bread_request=true;
                srv()->_cb_proc->onReadRequest((GHandler*)charel);
                _bread_request=false;
                dataRet.reset();
                dataRet.append(charel->_value, charel->_length);
                result = ATT_ECODE_SUCCESS;
            }
        }
        else
        {
            result = ATT_ECODE_READ_NOT_PERM;
        }
    }
    if(result == ATT_ECODE_SUCCESS && dataRet.length() && offset)
    {
        if(dataRet.length() < offset)
        {
            return _reply_err(rqt, vhh, ATT_ECODE_INVALID_OFFSET, ret);
        }
        else
        {   /// CHECK THIS
            dataRet.slice(offset);
        }
    }

    if(result != 0xFF)
    {
        if(result == ATT_ECODE_SUCCESS)
        {
            ret << uint8_t((rqt == ATT_OP_READ_BLOB_REQ) ?
                                    ATT_OP_READ_BLOB_RESP :
                                    ATT_OP_READ_RESP);
                                    /// CHECK MTU length
            ret << dataRet;
        }
        else
        {
            return _reply_err(rqt, vhh, result, ret);
        }
    }
    return 1;
}

/****************************************************************************************
*/
int bu_gatt::_prep_wq(const sdata& indata, bybuff& ret)
{
    uint8_t  rqt = indata.data[0];
    uint16_t vhh = oa2t<uint16_t>(indata.data,1);
    uint16_t offset = oa2t<uint16_t>(indata.data,3);
    bybuff   data(indata.data+5, indata.len-5);
    GHandler* element =srv()->gatel(vhh);

    ret.reset();
    if(0 == element)
    {
        return _reply_err(rqt, vhh, ATT_ECODE_INVALID_HANDLE, ret);
    }
    if (element->_type == H_CHR_VALUE)
    {
        element = srv()->gatel(element->_hparent);

        if (element->_props & PROPERTY_WRITE)
        {
            if (element->_secure & PROPERTY_WRITE && !_pacls->is_encrypted())
            {
                return _reply_err(rqt, vhh, ATT_ECODE_AUTHENTICATION, ret);
            }
            else if (_prepareWQ._hndl != 0)
            {
                if (_prepareWQ._hndl != element->_hndl)
                {
                    return _reply_err(rqt, vhh, ATT_ECODE_UNLIKELY, ret);
                }
                else if (offset == (_prepareWQ._offset + _prepareWQ._data.length()))
                {
                    _prepareWQ._data << data;

                    ret << ATT_OP_PREP_WRITE_RESP;
                    ret.append(indata.data+1, indata.len-1);
                }
                else
                {
                    return _reply_err(rqt, vhh, ATT_ECODE_INVALID_OFFSET, ret);
                }
            }
            else
            {
                _prepareWQ._hndl = element->_hndl;
                _prepareWQ._valhhdl = element->_hvalue;
                _prepareWQ._offset = offset;
                _prepareWQ._data << data;

                ret << ATT_OP_PREP_WRITE_RESP;
                ret.append(indata.data+1, indata.len-1);
            }
        }
        else
        {
             return _reply_err(rqt, vhh, ATT_ECODE_WRITE_NOT_PERM, ret);
        }
    }
    else
    {
        return _reply_err(rqt, vhh, ATT_ECODE_ATTR_NOT_LONG, ret);
    }
    return 1;
}

/****************************************************************************************
*/
int bu_gatt::_exec_wq(const sdata& data, bybuff& ret)
{
    uint8_t  rqt = data.data[0];
    uint8_t  flag = data.data[1];

    ret.reset();
    if (_prepareWQ._hndl)
    {
        GHandler* element = srv()->gatel(_prepareWQ._hndl);

        assert(element->_type==H_CHR);

        if (flag == 0x00)
        {
            ret << ATT_OP_EXEC_WRITE_RESP;
        }
        else if (flag == 0x01)
        {
            element->_put_value(_prepareWQ._data);

            srv()->_cb_proc->onWriteRequest(element);
            ret << ATT_OP_EXEC_WRITE_RESP;
        }
        else
        {
            return _reply_err(rqt, 0x0000, ATT_ECODE_UNLIKELY, ret);
        }

        _prepareWQ._hndl = 0;
    }
    else
    {
        return _reply_err(rqt, 0x0000, ATT_ECODE_ATTR_NOT_LONG, ret);
    }
    return 1;
}

/****************************************************************************************
*/
int  bu_gatt::_indic_confirm(const sdata& data, bybuff& ret)
{
    if (_indicator)
    {
        srv()->_cb_proc->onIndicate(_indicator);
        _indicator = 0;
    }
    return 1;
}

