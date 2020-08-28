/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/
#include <dlfcn.h>
#include <vector>
#include <assert.h>
#include "libbungetpriv.h"
#include "bu_gap.h"
#include "uguid.h"
#include "bybuff.h"
#include "gattdefs.h"
#include "include/bluetooth.h"

static Cguid GUID_ZERO;

/****************************************************************************************
*/
bu_gap::bu_gap(bu_hci* hci)
{
    _hci = hci;
}

/****************************************************************************************
*/
bu_gap::~bu_gap()
{
    //dtor
}

/****************************************************************************************
 sets the hci non le name. without this the adv name might be flimbsy
*/
int bu_gap::set_btname(const char* name)
{
    _hci->write_local_name(name);
    return 0;
}

/****************************************************************************************
*/
void bu_gap::advertise(const std::string& name,
                        std::vector<IService*>& srvs,
                        uint32_t pin)
{
    if(srvs.size())
    {
        bybuff  advData;
        bybuff  scn;
        size_t  nservs =  _hci->srv()->nServices();
/*
        for(const auto  &s : srvs)
        {
            GattSrv* ps = dynamic_cast<GattSrv*>(s);
            if(ps->_default)
                continue;
            ++nservs;
        }
*/

        const uint8_t adTypeUUID16Bit = 0x3;
        const uint8_t adTypeUUID128Bit = 0x7;


        advData << uint8_t(0x2) << uint8_t(0x1) << uint8_t(0x6);
        advData<<(uint8_t(1 + 2 * nservs));

        bool is16 = true;
        if(!srvs.empty() && dynamic_cast<GattSrv*>(srvs[0])->_cuid.is_16())
        {
            advData << adTypeUUID16Bit;
        } else {
            advData << adTypeUUID128Bit;
            is16 = false;
        }
        for(const auto  &s : srvs)
        {
            GattSrv* ps = dynamic_cast<GattSrv*>(s);
        //    if(ps->_default)
         //       continue;
            ps->debug();
            if (ps->_cuid.is_16()) {
                assert(is16);
                // can not mix up 16 and 128 bit uuids
                uint16_t uid = ps->_cuid.as16();
                advData << uid;
            } else {
                assert(!is16);
                uint128_t uid = ps->_cuid.as128();
                advData << uid;
            }
        }
        scn<<uint8_t(1+name.length())<<uint8_t(0x8);
        scn<<name;
        _air_waveit(advData, scn);
    }
}

/****************************************************************************************
*/
void bu_gap::restart_adv()
{
    _hci->enable_adv(true);
}

/****************************************************************************************
*/
void bu_gap::stop_adv()
{
    _hci->enable_adv(false);
}

/****************************************************************************************
*/
#define DOUBLE_ADV
void bu_gap::_air_waveit(const bybuff& add, const bybuff& scd)
{
    sdata sd;
#ifdef DOUBLE_ADV
    sd.len = scd.length();
    sd.data = (uint8_t*)scd.buffer();
    _hci->set_sca_res_data(sd);

    sd.len = add.length();
    sd.data = (uint8_t*)add.buffer();
    _hci->set_adv_data(sd);
    ::usleep(1000);
#endif
    _hci->enable_adv(true);
    sd.len = scd.length();
    sd.data = (uint8_t*)scd.buffer();
    _hci->set_sca_res_data(sd);

    sd.len = add.length();
    sd.data = (uint8_t*)add.buffer();
    _hci->set_adv_data(sd);
    _idle();
}

/****************************************************************************************
TODO
*/
void bu_gap::adv_beacon(const uint128_t& uuid,
                        uint16_t minor, uint16_t major,
                        int8_t power, uint16_t manid,
                        const bybuff& b)
{
    bybuff  bed;
    bybuff  add;

    bed << uint128_t(uuid);
    bed << uint16_t(bswap_16(major)) << uint16_t(bswap_16(minor)) << uint8_t(power);

    _TRACE(bed.to_string());
    //1111111111111111111111111111111100040064e000
    //1111111111111111111111111111111100640004E0
    uint8_t dl = bed.length();
    uint8_t mandl = dl + 5;

    add << uint8_t(0x2) << uint8_t(0x1) << uint8_t(0x6);
    add << uint8_t(mandl);
    add << uint8_t(0xFF);
    add << uint16_t(manid); //0x004C APPLE
    add << uint8_t(0x2C);
    add << uint8_t(dl);
    add << bed;
    _TRACE(add.to_string());
    _air_waveit(add, b);
    _idle();
}

/****************************************************************************************
DO NOT USE !!!
*/
void bu_gap::set_pin(uint32_t pin)
{
    gap_set_auth_requirement_cp cp={0,0,{0},0,0,0,0,0};

    if(pin==0)
        cp.mitm_mode = MITM_PROTECTION_NOT_REQUIRED;
    else
        cp.mitm_mode = MITM_PROTECTION_REQUIRED;
    cp.oob_enable = OOB_AUTH_DATA_ABSENT;
    //cp.oob_data = 0;
    cp.min_encryption_key_size = 7;
    cp.max_encryption_key_size = 16;
    cp.use_fixed_pin = USE_FIXED_PIN_FOR_PAIRING;
    cp.fixed_pin = htobl(pin);
    cp.bonding_mode = NO_BONDING;

    _hci->send_cmd( OGF_VENDOR_CMD, OCF_GAP_SET_AUTH_REQUIREMENT, sizeof(cp), &cp);
}

/****************************************************************************************
*/
void bu_gap::_idle()
{
    _hci->pool();
}
