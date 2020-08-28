/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "crypto.h"
#include "bybuff.h"
#include <crypto++/osrng.h> // sudo apt-get install uuid-dev libcrypto++-dev
#include <crypto++/modes.h>
#include <crypto++/aes.h>
#include <crypto++/filters.h>

/****************************************************************************************
*/
cryptos::cryptos()
{
   srand (time(NULL));
}

/****************************************************************************************
*/
cryptos::~cryptos()
{
}

/****************************************************************************************
*/
void cryptos::gen_random(int ammount, bybuff&  dst)const
{
    for (int i=0;i<ammount;i++)
        dst <<  uint8_t(rand()%0xFF);
}

/****************************************************************************************
*/
void cryptos::_oxor(const bybuff& src1, const bybuff& src2, bybuff& dst)const
{
    dst.reset();
    for(size_t i=0;i<src1.length();i++){
        uint8_t a = src1[i];
        uint8_t b = src2[i];
        uint8_t r = a ^ b;
        dst << uint8_t(r);
    }
}

/****************************************************************************************
*/
void cryptos::c1(bybuff& k, bybuff& r, bybuff& pres, bybuff& preq, bybuff& iat, bybuff& ia, bybuff& rat, bybuff& ra,  bybuff&  dst)const
{
    bybuff p1,p2;

    p1 << iat << rat  <<  preq  << pres;
    p2 << ra << ia << 0x00000000;
    bybuff      rez1;
    _oxor(r, p1, rez1);
    bybuff      rez2;
    _eee(k, rez1, rez2);
    rez1.reset();
    _oxor(rez2, p2, rez1);
    _eee(k, rez1, dst);

}

/****************************************************************************************
*/
void cryptos::s1(bybuff& tk, bybuff& r1, bybuff& r2, bybuff& stk)const
{
    bybuff  sr1(r1.buffer()+8,r1.length()-8);
    bybuff  sr2(r2.buffer()+8,r2.length()-8);
    sr1 << sr2;
    _eee(tk, sr1, stk);
}

/****************************************************************************************
*/
void cryptos::_eee(bybuff& key, bybuff& data, bybuff& dst)const
{
    key.reverse();
    data.reverse();
    _sha_encrypt(key, data, dst);
    dst.reverse();
}

/****************************************************************************************
*/
void cryptos::_sha_encrypt(const bybuff& key, const bybuff& data, bybuff& out)const
{

    //Key and IV setup
    //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-
    //bit). This key is secretly exchanged between two parties before communication
    //begins. DEFAULT_KEYLENGTH= 16 bytes
    //uint8_t iv[ CryptoPP::AES::BLOCKSIZE ] = {0};
    std::string encoded;
    try{
#if 0
        CryptoPP::AES::Encryption                       aesEncryption(key.buffer(), key.length());
        CryptoPP::CBC_Mode_ExternalCipher::Encryption   cbcEncryption( aesEncryption, iv );
        CryptoPP::StreamTransformationFilter            stfEncryptor(cbcEncryption, new CryptoPP::StringSink( encoded ) );
        stfEncryptor.Put( data.buffer(), (size_t)data.length() );
        stfEncryptor.MessageEnd();
        out = encoded;
        _TRACE("_sha_e(" << key.to_string() <<","<< data.to_string() <<")=" << out.to_string());
        bybuff r;
        _sha_decrypt(key, out, r);
#endif //0

        CryptoPP::ECB_Mode< CryptoPP::AES >::Encryption aes_128_ecb;//(key.buffer(), key.length(), iv);
        aes_128_ecb.SetKey( key.buffer(), key.length() );
        CryptoPP::StreamTransformationFilter encryptor(aes_128_ecb, 0, CryptoPP::BlockPaddingSchemeDef::NO_PADDING);

        for(size_t j = 0; j < data.length(); j++)
        {
            encryptor.Put((byte)data[j]);
        }
        encryptor.MessageEnd();

        size_t ready = encryptor.MaxRetrievable();
        byte  outa[32] = {0};
        encryptor.Get((byte*) outa, ready);
        out.append(outa, ready);
    }
    catch( CryptoPP::Exception& e )
    {
        ERROR( e.what());
        assert(0);
    }
   // bybuff r;
    //_sha_decrypt(key, out, r);
}

/****************************************************************************************
*/
void cryptos::_sha_decrypt(const bybuff& key, const bybuff& data, bybuff& out)const
{
    std::string  decryptedtext;
    uint8_t iv[ CryptoPP::AES::BLOCKSIZE ] = {0};
    //
    // Decrypt
    //
/**
    CryptoPP::AES::Decryption aesDecryption(key.buffer(), key.length());
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );

    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedtext ) );
    stfDecryptor.Put( data.buffer(), (size_t)data.length() );
    stfDecryptor.MessageEnd();
    out = decryptedtext;
*/
    try{
        CryptoPP::ECB_Mode< CryptoPP::AES >::Decryption aes_128_ecb(key.buffer(), key.length(), iv);
        //aes_128_ecb.SetKey( );
        CryptoPP::StreamTransformationFilter decryptor(aes_128_ecb, 0, CryptoPP::BlockPaddingSchemeDef::NO_PADDING);

        for(size_t j = 0; j < data.length(); j++)
        {
            decryptor.Put((byte)data[j]);
        }
        decryptor.MessageEnd();

        size_t ready = decryptor.MaxRetrievable();
        byte  outa[32] = {0};
        decryptor.Get((byte*) outa, ready);
        out.append(outa, ready);
    }
    catch( CryptoPP::Exception& e )
    {
        ERROR( e.what());
    }
}
