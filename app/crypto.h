/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/

#ifndef CRYPTOS_H
#define CRYPTOS_H

#include <icryptos.h>
using namespace bunget;

class cryptos : public Icryptos
{
public:
    cryptos();
    virtual ~cryptos();
    void gen_random(int ammount, bybuff&  dst)const;
    void s1(bybuff& tk, bybuff& r1, bybuff& r2, bybuff& stk)const;
    void c1(bybuff& k, bybuff& r, bybuff& pres, bybuff& preq, bybuff& iat, bybuff& ia, bybuff& rat, bybuff& ra,  bybuff&  dst)const;

private:
    void _eee(bybuff& key, bybuff& data, bybuff& out)const;
    void _oxor(const bybuff& src1, const bybuff& src2, bybuff& dst)const;
    void _sha_encrypt(const bybuff& key, const bybuff& data, bybuff& out)const;
    void _sha_decrypt(const bybuff& key, const bybuff& data, bybuff& out)const;
};

#endif // CRYPTOS_H
