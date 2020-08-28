/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This program, or portions of it cannot be used in commercial
    products without the written consent of the author: marrius9876@gmail.com

*/

#ifndef I_CRYPTOS_H
#define I_CRYPTOS_H


class bybuff;

namespace bunget{
    
class Icryptos
{
public:
    Icryptos(){};
    virtual ~Icryptos(){};
    virtual void gen_random(int ammount, bybuff&  dst)const=0;
    virtual void s1(bybuff& tk, bybuff& r1, bybuff& r2, bybuff& stk)const=0;
    virtual void c1(bybuff& k, bybuff& r, bybuff& pres, bybuff& preq, 
                    bybuff& iat, bybuff& ia, bybuff& rat, bybuff& ra,  
                    bybuff&  dst)const=0;
};

};

#endif // I_CRYPTOS_H
