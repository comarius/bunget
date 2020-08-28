/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef SECMANP_H
#define SECMANP_H

#include "hci_socket.h"
#include "bybuff.h"
#include "icryptos.h"
#include "bu_hci.h"
using namespace bunget;

#define XADDR_PUBLIC  0
#define XADDR_RANDOM  1


class bu_asc;
class secmanp : public hci_data_eater
{
public:
    secmanp(Icryptos* pc, bu_asc* acl,  bu_hci*  hci, const bdaddr_t& local, int ltyp,  const bdaddr_t& remote, int rtyp);
    virtual ~secmanp();

    void on_encryption_changed(bool is_encrypted);
    void on_ltk_neg();
    void on_stream_end();

    void _pairing_req(const bybuff& data);
    void _pairing_confirm(const bybuff& data);
    void _pairing_random(const bybuff& data);
    void _pairing_failure(const bybuff& data);
    void _pairing_unknown(uint8_t code, const bybuff& data);
    void write(const bybuff& data);

    int on_sock_data(uint8_t code, const sdata& data);
    void on_error(const hci_error& error);
    bool onSpin(bt_socket* sock);

protected:
private:
    Icryptos* _crypt;
    bu_asc*   _aclbuffer;
    bybuff    _iat;
    bybuff    _ia;
    bybuff    _rat;
    bybuff    _ra;
    bybuff    _preq;
    bybuff    _pres;
    bybuff    _pncf;
    bybuff    _tk;
    bybuff    _stk;
    bybuff    _mangaler;
    bybuff    _random;
    bybuff    _r;
    bybuff    _pcnf;
    bu_hci*   _hci;
};


#define SMP_CID 0x0006
#define SMP_PAIRING_REQUEST 0x01
#define SMP_PAIRING_RESPONSE 0x02
#define SMP_PAIRING_CONFIRM 0x03
#define SMP_PAIRING_RANDOM 0x04
#define SMP_PAIRING_FAILED 0x05
#define SMP_ENCRYPT_INFO 0x06
#define SMP_MASTER_IDENT 0x07
#define SMP_CMD_IDENT_INFO 0x08
#define SMP_CMD_IDENT_ADDR_INFO  0x08
#define SMP_CMD_SIGN_INFO  0x09
#define SMP_CMD_SECURITY_REQ   0x0A
#define SMP_CMD_PUBLIC_KEY 0x0B
#define SMP_CMD_DHKEY_CHECK   0x0C
#define SMP_CMD_KEYPRESS_NOTIFY  0x0D


 #define SMP_PASSKEY_ENTRY_FAILED        0x01
 #define SMP_OOB_NOT_AVAIL               0x02
 #define SMP_AUTH_REQUIREMENTS           0x03
 #define SMP_CONFIRM_FAILED              0x04
 #define SMP_PAIRING_NOTSUPP             0x05
 #define SMP_ENC_KEY_SIZE                0x06
 #define SMP_CMD_NOTSUPP                 0x07
 #define SMP_UNSPECIFIED                 0x08
 #define SMP_REPEATED_ATTEMPTS           0x09
 #define SMP_INVALID_PARAMS              0x0a
 #define SMP_DHKEY_CHECK_FAILED          0x0b
 #define SMP_NUMERIC_COMP_FAILED         0x0c
 #define SMP_BREDR_PAIRING_IN_PROGRESS   0x0d
 #define SMP_CROSS_TRANSP_NOT_ALLOWED    0x0e


#endif // SECMANP_H
