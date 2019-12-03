#ifndef RSA_HPP_
# define RSA_HPP_

# include <fstream>

extern "C"
{
# include "miracl.h"
}

class Rsa
{
private:
  static bool miraclInitialized;

  std::ifstream& file;
  bool publicKeyInitialized = false;
  big RSA_N;
  big RSA_d;
  big RSA_e;

private:
  bool initRsaPublicKey(const unsigned char* crypted_sample);
  bool skipPadding(const unsigned char *data, size_t& i);
  bool checkPadding(const unsigned char *data);
  bool skipPaddingAndCopy(const unsigned char *src, unsigned char *dst, size_t size);
  void DecryptBlock(const unsigned char* src, unsigned char* dst);
  bool Decrypt6432(const unsigned char* src, unsigned char* dst, size_t dstLen);

public:
  // The File object needs to be alive as long as the Rsa object is.
  Rsa(std::ifstream& file);
  ~Rsa();
  bool read(void *buffer, size_t size);

  static void initMiracl();
  static void freeMiracl();

  static const int KEY_SIZE = 512; // in bits
  static const int KEY_BYTESIZE = KEY_SIZE / 8;
};

#endif /* RSA_HPP_ */
