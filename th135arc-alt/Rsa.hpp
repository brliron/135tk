#ifndef RSA_HPP_
# define RSA_HPP_

# include <fstream>
#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/integer.h"

class Rsa
{
public:
  enum class CryptMode {
    TH135,
    TH145
  };

private:
  static bool initialized;

  std::ifstream* ifile;
  std::ofstream* ofile;
  bool publicKeyInitialized = false;
  CryptMode cryptMode;
  CryptoPP::Integer RSA_N;
  CryptoPP::Integer RSA_d;
  CryptoPP::Integer RSA_e;

  void init();
  bool initRsaPublicKey(const unsigned char* crypted_sample);
  bool skipPadding(const unsigned char *data, size_t& i);
  bool checkPadding(const unsigned char *data);
  bool skipPaddingAndCopy(const unsigned char *src, unsigned char *dst, size_t size);
  void DecryptBlock(const unsigned char* src, unsigned char* dst);
  void EncryptBlock(const unsigned char* src, unsigned char* dst);
  bool Decrypt6432(const unsigned char* src, unsigned char* dst, size_t dstLen);
  bool Encrypt3264(const unsigned char* src, unsigned char* dst, size_t srcLen);
  bool read32(void *buffer, size_t size);
  bool write32(const void *buffer, size_t size);

public:
  // The File object needs to be alive as long as the Rsa object is.
  Rsa(std::ifstream& file);
  Rsa(std::ofstream& file, CryptMode mode);
  ~Rsa();
  bool read(void *buffer, size_t size);
  bool write(const void *buffer, size_t size);

  static void initCryptoPP();
  static void freeCryptoPP();
};

#endif /* RSA_HPP_ */
