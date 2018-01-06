#ifndef H__TASOFROCRYPT
#define H__TASOFROCRYPT

const int KEY_SIZE = 512; // in bits
const int BLOCK_SIZE = 512; // in bits
const int KEY_BYTESIZE = KEY_SIZE/8;
const int KEY_WORDSIZE = KEY_BYTESIZE/sizeof(unsigned short);

int InitRSAKeyPair(int);
int EncryptBlock(const unsigned char* src,unsigned char* dst);
int DecryptBlock(const unsigned char* src,unsigned char* dst);
int Decrypt6432(const unsigned char* src,unsigned char* dst,size_t dstLen=0x20);
int Encrypt3264(const unsigned char* src,unsigned char* dst,size_t srcLen=0x20);

#endif // !H__TASOFROCRYPT
