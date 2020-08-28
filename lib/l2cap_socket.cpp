/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#include "l2cap_socket.h"

/****************************************************************************************
*/
void l2cap_socket::create()
{

    _sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if(_sock<=0) _THROW (0);
}

/****************************************************************************************
*/
void l2cap_socket::bind(const bdaddr_t& src, uint8_t srctype, uint16_t psm, uint8_t cid)
{
    struct sockaddr_l2 addr;

    ::memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    ::bacpy(&addr.l2_bdaddr, &src);

    if (cid)
        addr.l2_cid = htobs(cid);
    else
        addr.l2_psm = htobs(psm);

    addr.l2_bdaddr_type = srctype;

    if (::bind(_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        _THROW(0);
    }
}

/****************************************************************************************
*/
void l2cap_socket::set(uint8_t src_type, int sec_level,
                       uint16_t imtu, uint16_t omtu, uint8_t mode,
                       int master, int flushable, uint32_t priority)
{
    if (imtu || omtu || mode)
    {

        if (src_type == BDADDR_BREDR)
            _set_l2opts(imtu, omtu, mode);
        else
            _set_le_imtu(imtu);
    }

    if(master>0)
        _l2cap_set_master(master);

    if (flushable >= 0 )
        _l2cap_set_flushable(flushable);

    if (priority > 0)
        _set_priority( priority);

    if (sec_level)
        _set_sec_level(sec_level);
}

/****************************************************************************************
*/
int l2cap_socket::connect(struct sockaddr *addr)
{
    return ::connect(_sock, (struct sockaddr *)addr, sizeof(struct sockaddr));
}

/****************************************************************************************
*/
void l2cap_socket::_set_l2opts(uint16_t imtu, uint16_t omtu, uint8_t mode)
{
    struct l2cap_options l2o;
    socklen_t len;

    ::memset(&l2o, 0, sizeof(l2o));
    len = sizeof(l2o);
    if (::getsockopt(_sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &len) < 0)
    {
        _THROW(0);
    }

    if (imtu)
        l2o.imtu = imtu;
    if (omtu)
        l2o.omtu = omtu;
    if (mode)
        l2o.mode = mode;

    if (::setsockopt(_sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o, sizeof(l2o)) < 0)
    {
        _THROW(0);
    }
}

/****************************************************************************************
*/
void  l2cap_socket::_set_le_imtu(uint16_t imtu)
{
    if (::setsockopt(_sock, SOL_BLUETOOTH, BT_RCVMTU, &imtu,
                     sizeof(imtu)) < 0)
    {
        _THROW(0);
    }
}

/****************************************************************************************
*/
void  l2cap_socket::_l2cap_set_master(int master)
{
    int flags;
    socklen_t len;

    len = sizeof(flags);
    if (::getsockopt(_sock, SOL_L2CAP, L2CAP_LM, &flags, &len) < 0)
        _THROW(0);

    if (master)
    {
        if (flags & L2CAP_LM_MASTER)
            return ;
        flags |= L2CAP_LM_MASTER;
    }
    else
    {
        if (!(flags & L2CAP_LM_MASTER))
            return ;
        flags &= ~L2CAP_LM_MASTER;
    }

    if (::setsockopt(_sock, SOL_L2CAP, L2CAP_LM, &flags, sizeof(flags)) < 0)
        _THROW(0);
}

/****************************************************************************************
*/
void  l2cap_socket::_l2cap_set_flushable(bool flushable)
{
    int f = flushable;
    if (::setsockopt(_sock, SOL_BLUETOOTH, BT_FLUSHABLE, &f, sizeof(f)) < 0)
        _THROW(0);
}

/****************************************************************************************
*/
void  l2cap_socket::_set_priority(uint32_t prio)
{
    if (::setsockopt(_sock, SOL_SOCKET, SO_PRIORITY, &prio, sizeof(prio)) < 0)
        _THROW(0);

}

/****************************************************************************************
*/
void  l2cap_socket::_set_sec_level(int level)
{
    struct bt_security sec;

    if (level < BT_SECURITY_LOW || level > BT_SECURITY_HIGH)
    {
        _THROW(0);
    }

    ::memset(&sec, 0, sizeof(sec));
    sec.level = level;

    if (::setsockopt(_sock, SOL_BLUETOOTH, BT_SECURITY, &sec,sizeof(sec)) == 0)
        return;

    if (errno != ENOPROTOOPT)
    {
        _THROW(0);
    }
    _l2cap_set_lm(level);
}

/****************************************************************************************
*/
void l2cap_socket::_l2cap_set_lm(int level)
{
    int lm_map[] =
    {
        0,
        L2CAP_LM_AUTH,
        L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT,
        L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT | L2CAP_LM_SECURE,
    }, opt = lm_map[level];

    if (::setsockopt(_sock, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0)
        _THROW(0);
}
