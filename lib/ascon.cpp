/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#include "bu_hci.h"
#include "ascon.h"
#include "libbungetpriv.h"

/****************************************************************************************
*/
bu_asc::bu_asc(Icryptos* pcrypt, bu_hci* hci, uint16_t handle,
                            const bdaddr_t& l,
                            int ltyp,
                            const bdaddr_t& r,
                            int rtyp):_handle(handle),_hci(hci)
{
    _secman = new secmanp(pcrypt, this, hci, l, ltyp, r, rtyp);
}

/****************************************************************************************
*/
bu_asc::~bu_asc()
{
    delete _secman;
}

/****************************************************************************************
*/
void bu_asc::write(uint16_t cid, const bybuff& data)
{
    sdata sd;
    sd.data = data.buffer();
    sd.len = uint16_t(data.length());
#ifdef ACL_MTU_FRAG
    _hci->enque_acl(_handle, cid, sd);
#else
    _hci->write_ack_packet(_handle, cid, sd);
#endif

}

/****************************************************************************************
*/
void bu_asc::push(uint16_t cid,  const sdata& data)
{
    if (data.len)
    {
        _hci->srv()->feed_them(cid, data);
    }
}

/****************************************************************************************
*/
void  bu_asc::set_he(uint16_t handle, uint8_t encrypt)
{
    _encrypted = encrypt ? 1 : 0;
    _secman->on_encryption_changed(_encrypted);
};

/****************************************************************************************
*/
void bu_asc::set_ltk_r()
{
    _secman->on_ltk_neg();
}
