/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef UCUID_H
#define UCUID_H


#include <iostream>
#include <string.h>
#include "include/uuid.h"
#include "include/bluetooth.h"

extern uint128_t GAT_MASK;

//=============================================================================


class Cguid
{
public:
    Cguid(){
        ::memset(&_u128, 0, sizeof(_u128));
    };
    Cguid(const Cguid& uuid){
        ::memcpy(&_u128, &uuid ,sizeof(uuid));
    };
    Cguid(const bt_uuid_t& uuid){
        ::memcpy(&_u128, &uuid, sizeof(_u128));
    }
    Cguid(const uint8_t* uuid){

#if __BYTE_ORDER == __BIG_ENDIAN
        bswap_128(uuid, _u128.value.u128.data);
#else
        ::memcpy(&_u128.value.u128, uuid ,sizeof(_u128.value.u128));
#endif
    };

    std::string to_string()
    {
        char out[64];
        struct{
            uint32_t    a1;
            uint16_t    a2;
            uint16_t    a3;
            uint16_t    a4;
            uint8_t     a12[6];
        }  sfmt;
        ::memcpy(&sfmt, _u128.value.u128.data, sizeof(sfmt));

        if(is_16())
        {
            ::sprintf(out,"%08X", sfmt.a1);
        }
        else
        {
            // 99999999-9999-9999-9999-999999999999
            ::sprintf(out, "UUID:%08X-%04X-%04X-%04X-%0X%0X%0X%0X%0X%0X",
                    sfmt.a1,
                    sfmt.a2,
                    sfmt.a3,
                    sfmt.a4,
                    sfmt.a12[0],sfmt.a12[1],sfmt.a12[2],sfmt.a12[3],sfmt.a12[4],sfmt.a12[5]);
        }
        return std::string(out);
    }


    Cguid(uint16_t base16){
        ::memset(&_u128, 0, sizeof(_u128));
        _u128.value.u16 |= base16;
    }

    Cguid(uint32_t base32){
        ::memset(&_u128, 0, sizeof(_u128));
        _u128.value.u32 |= base32;
    }
    ~Cguid(){};

    bool operator==(const Cguid& u128)
    {
        return !::memcmp(&_u128 , &u128, sizeof(Cguid));
    }
    bool operator==(const bt_uuid_t& u128)
    {
        return !::memcmp(&_u128.value.u128 , &u128, sizeof(bt_uuid_t));
    }
    bool operator==(uint16_t u16t)
    {
        return _u128.value.u16==u16t;
    }

    Cguid& operator=(const Cguid& uuid){
        ::memcpy(&_u128, &uuid ,sizeof(_u128));
        return *this;
    }

    Cguid& operator=(const bt_uuid_t& uuid){
        ::memcpy(&_u128.value.u128, &uuid, sizeof(bt_uuid_t));
        return *this;
    }

    void reset(){
        ::memset(this,0,sizeof(*this));
    }

    Cguid& operator=(uint16_t uu16){
         reset();
        _u128.value.u16 = uu16;
        return *this;
    }

    Cguid& operator=(const uint128_t& uu128){
        ::memcpy(_u128.value.u128.data, uu128.data, sizeof(uu128.data));
        return *this;
    }

    Cguid& operator=(uint32_t uu32){
         reset();
        _u128.value.u32 = uu32;
        return *this;
    }
    bool is_16()const
    {
        return 0 == _u128.value.au16[1];
    }
    uint16_t as16()const
    {
        return _u128.value.u16;
    }

     uint32_t as32()const
    {
        return _u128.value.u32;
    }

    const uint128_t& as128()const
    {
        return _u128.value.u128;
    }

    const bt_uuid_t& asbt_uuid_t()const
    {
        return _u128;
    }
    uint8_t operator[](int index){
#if __BYTE_ORDER == __BIG_ENDIAN
        return _u128.value.u128.data[index];
#else
        return _u128.value.u128.data[15-index];
#endif
    }

    static bt_uuid_t from_string(const char* suid)
    {
        bt_uuid_t u;
        bt_string_to_uuid(&u, suid);
        return u;
    }


    bt_uuid_t  _u128;
};


#endif // UGUID_H
