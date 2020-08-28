/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/

#ifndef _LEBINENDS_
#define _LEBINENDS_

#include "include/bluetooth.h"

inline int is_little_endian()
{
    short a = 1;
    return *((char*)&a) & 1;
}









#endif //_LEBINENDS_
