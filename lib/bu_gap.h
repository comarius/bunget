/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef NO_GAP_H
#define NO_GAP_H

#include "include/uuid.h"
#include <vector>
#include "hci_socket.h"
#include "bu_hci.h"
#include "include/libbunget.h"

using namespace bunget;
#define USE_FIXED_PIN_FOR_PAIRING		(0x0)
#define DONOT_USE_FIXED_PIN_FOR_PAIRING	(0x01)
#define MITM_PROTECTION_NOT_REQUIRED	(0x00)
#define MITM_PROTECTION_REQUIRED        (0x01)
#define OOB_AUTH_DATA_ABSENT		    (0x00)
#define OOB_AUTH_DATA_PRESENT      		(0x01)
#define BONDING				            (0x01)
#define NO_BONDING				        (0x00)
#define OGF_VENDOR_CMD		            0x3f

#define OCF_GAP_SET_AUTH_REQUIREMENT      0x0086
typedef struct {
    uint8_t	mitm_mode;
    uint8_t     oob_enable;
    uint8_t     oob_data[16];
    uint8_t     min_encryption_key_size;
    uint8_t     max_encryption_key_size;
    uint8_t     use_fixed_pin;
    uint32_t    fixed_pin;
    uint8_t     bonding_mode;
} __attribute__ ((packed)) gap_set_auth_requirement_cp;
#define GAP_SET_AUTH_REQUIREMENT_CP_SIZE 26


class bybuff;
class SrvDevice;
class bu_gap
{
public:
    bu_gap(bu_hci* hci);
    virtual ~bu_gap();
    void advertise(const std::string& name, std::vector<IService*>& srvs, uint32_t pin);
    void adv_beacon(const uint128_t& uuid, uint16_t minor, uint16_t major, int8_t power, uint16_t manid, const bybuff& b);
    void stop_adv();
    void restart_adv();
    void set_pin(uint32_t pin);
    int set_btname(const char* name);
private:
    void _air_waveit(const bybuff& advData, const bybuff& scanData);
    void _idle();
private:
    bu_hci*  _hci;
};

#endif // NO_GAP_H
