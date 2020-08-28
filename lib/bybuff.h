/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef BYBUFF_H
#define BYBUFF_H


#include <stdint.h>
#include <vector>
#include <iostream>
#include <bitset>
#include <sstream>
#include <algorithm>
#include "uguid.h"
#include "hci_socket.h"

//=============================================================================
#define _PAACK8     __attribute__ ((__packed__))


typedef uint8_t u8_ptr[0];

template <class T>
const uint8_t*  t2o(T data, uint8_t* arr)
{
    uint8_t *bytes = ((uint8_t*)(&data));
    for(unsigned int i = 0; i < sizeof(data); i++)
    {
#if __BYTE_ORDER == __BIG_ENDIAN
        arr[i] = bytes[sizeof(data)-1-i];
#else
    	arr[i] = bytes[i];
#endif
    }
    return arr;
}

template <class T>
T oa2t(const uint8_t *bytes, size_t offset)
{
    union{
        uint8_t input[8];
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
    }tu;

    for(unsigned int i = 0; i < sizeof(T); i++)
    {
#if __BYTE_ORDER == __BIG_ENDIAN
        tu.input[i] = bytes[sizeof(T)-1-i+offset];
#else
        tu.input[i] = bytes[i+offset];
#endif
    }
    switch(sizeof(T))
    {
        case 1:
            return tu.u8;
        case 2:
            return tu.u16;
        case 4:
            return tu.u32;
        case 8:
            return tu.u32;
    }
    return tu.u32;
}

template <class T>
void dump_conv(T &data)
{
    std::stringstream output;
    std::cout << (std::bitset<sizeof(data) * 8>(data)) << "\n";
}

/*

*/


class bybuff
{
public:
    bybuff(size_t sz=0){
        _buff.clear();
    };

    bybuff(const uint8_t* buff, int len){
        _buff.clear();        
        for(int k=0;k<len;k++)
            _buff.push_back(buff[k]);
        _readindex=0;
    }

     bybuff(const bybuff& r){
        _buff.clear();        
        for(size_t k=0;k<r.length();k++)
            _buff.push_back(r._buff[k]);
        _readindex=0;
    }

    template <typename T>
    void set(T t, int index)
    {
        while(_buff.size()<index+sizeof(T))
        {
            _buff.push_back(0);
        }
        uint8_t* p = (uint8_t*)&t;
        for(int i=0;i<sizeof(T);i++)
        {
        #if __BYTE_ORDER == __BIG_ENDIAN
            _buff[i]=p[sizeof(T)-i-1];
        #else
            _buff[i]=p[i];
        #endif
        }
    }

    void append(const uint8_t* buff, int len)
    {
        for(int k=0;k<len;k++)
            _buff.push_back(buff[k]);
    }
    
    size_t getcount(size_t count){
         return std::min(count, _buff.size());
    }

    size_t transfer(bybuff& to, size_t count)
    {
		size_t maxcount = std::min(count, _buff.size());
    	for(size_t i=0;i < maxcount; i++){
		    to << (this->_buff[i]);
		}
    	while(maxcount-- && this->_buff.size()){
	    	this->_buff.erase(this->_buff.begin()); 
        }
		return _buff.size();	
    }

    void pad(int octets)
    {
        while(--octets>=0)
            _buff.push_back(0);
    }

    void reset(){
        _buff.clear();
        _readindex=0;
    }

    void rewind(){
        _readindex=0;
    }
    virtual ~bybuff(){
    }

    bybuff& operator << (const uint8_t val)
    {
        _buff.push_back(val);
        return *this;
    }

    template <typename T>
    bybuff& operator <<(const T& t)
    {
        uint8_t arr[8];
        switch(sizeof(T))
        {
             case 2:
                t2o<T>(t,arr);
                _buff.push_back(arr[0]);
                _buff.push_back(arr[1]);
                break;
            case 4:
                t2o<T>(t,arr);
                _buff.push_back(arr[0]);
                _buff.push_back(arr[1]);
                _buff.push_back(arr[2]);
                _buff.push_back(arr[3]);
                break;
            case 8:
                 t2o<T>(t,arr);
                _buff.push_back(arr[0]);
                _buff.push_back(arr[1]);
                _buff.push_back(arr[2]);
                _buff.push_back(arr[3]);
                _buff.push_back(arr[4]);
                _buff.push_back(arr[5]);
                _buff.push_back(arr[6]);
                _buff.push_back(arr[7]);
                break;
            default:
                _THROW(0);
                break;
        }
        return *this;
    }



    bybuff&  operator <<(const bdaddr_t& t)
    {
         bdaddr_t r;
         baswap(&r, &t);
        _buff.push_back(r.b[0]);
        _buff.push_back(r.b[1]);
        _buff.push_back(r.b[2]);
        _buff.push_back(r.b[3]);
        _buff.push_back(r.b[4]);
        _buff.push_back(r.b[5]);
        return *this;
    }

    bybuff&  operator <<(const bybuff& refo)
    {
        for(auto  &octet : refo._buff)
        {
            _buff.push_back(octet);
        }
        return *this;
    }

    bybuff&  operator <<(const uint128_t& t)
    {
        uint128_t dst;
        ntoh128(&t,&dst);
        for(int i=0;i<16;i++)
            _buff.push_back(dst.data[i]);
        return *this;
    }

    bybuff&  operator <<(const Cguid& t)
    {
        if(t.is_16())
        {
            uint8_t  arr[8];
            uint16_t v = t.as16();
             t2o<uint16_t>(v,arr);
            _buff.push_back(arr[0]);
            _buff.push_back(arr[1]);

            return *this;
        }
        uint128_t dst;
        uint128_t src;
        ::memcpy(src.data, t._u128.value.u128.data, sizeof(src.data));
        ntoh128(&src,&dst);
        for(int i=0;i<16;i++)
            _buff.push_back(dst.data[i]);
        return *this;
    }

    bybuff&  operator <<(const bt_uuid_t& t)
    {
        uint128_t dst;
        uint128_t src;
        ::memcpy(src.data,t.value.u128.data,sizeof(src.data));
        ntoh128(&src,&dst);
        for(int i=0;i<16;i++)
            _buff.push_back(dst.data[i]);
        return *this;
    }

    bybuff& operator=(const bybuff& r){
        reset();
        for(size_t i=0;i<r.length();i++)
            _buff.push_back(r[i]);
        return *this;
    }

    bybuff& operator=(const std::string& r){
        reset();
        for(size_t i=0;i<r.length();i++)
            _buff.push_back(r.at(i));
        return *this;
    }

    void fromhex(const char* hex)
    {
        int  v;
        char by[3] = {0};
        size_t len = ::strlen(hex);
        _buff.clear();
        for(size_t l = 0 ; l < len; l+=2)
        {

            ::strncpy(by, hex+l, 2);
            ::sscanf(by,"%2X", &v);
            _buff.push_back(uint8_t(v&0xFF));
        }
    }

    bybuff& operator <<(const std::string& t)
    {
        for(size_t i=0;i<t.length();i++)
            _buff.push_back(t.at(i));
        return *this;
    }

    bybuff& operator <<(const char* t)
    {
        for(int i=0; t[i]!='\0'; i++)
            _buff.push_back(t[i]);
        return *this;
    }

    const uint8_t* buffer()const{
        return (const uint8_t*)&_buff[0];
    }

    void slice(size_t from){
        while(from--)
            _buff.erase(_buff.begin());
    }

    size_t length()const{
        return _buff.size();
    }

    template <typename T>
    const T read()
    {
        uint8_t* parr = &_buff[_readindex];
        T t = *((T*)parr);
#if __BYTE_ORDER == __BIG_ENDIAN
        switch(sizeof(T))
        {
            case 2:
                t = bswap_16(t);
                break;
            case 4:
                t = bswap_32(t);
                break;
            case 8:
                t = bswap_64(t);
                break;
            default:
                break;
        }
#endif //
        _readindex+=sizeof(T);
        return t;
    }

     void readguid(bt_uuid_t& uid)
    {
        uint8_t* src = &_buff[_readindex];
        ntoh128((uint128_t*)src,(uint128_t*)&uid);
        _readindex+=sizeof(bt_uuid_t);
    }
    void readbaddr(bdaddr_t& ret)
    {
        uint8_t* parr = &_buff[_readindex];
        baswap(&ret, (bdaddr_t*)parr);
        _readindex+=sizeof(bdaddr_t);
    }

    uint8_t operator[](int index)const
    {
        return _buff[index];
    }

    void reverse()
    {
        std::reverse(_buff.begin(),_buff.end());
    }

    bool operator==(const bybuff& r)
    {
        return !::memcmp(&_buff[0], &r._buff[0], std::min(r.length(),length()));
    }

    std::string to_string()const
    {
        std::string ret;
        char by[4];
        for(const auto& e : _buff)
        {
            ::sprintf(by,"%02X",e);
            ret.append(by);
        }
        return ret;
    }

private:
    std::vector<uint8_t> _buff;
    int                  _readindex;
};


#endif // BYBUFF_H
