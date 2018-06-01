#ifndef RSA_HPP_
# define RSA_HPP

# include "File.hpp"

extern "C"
{
# include "miracl.h"
}

class Rsa
{
private:
  static bool miraclInitialized;

  File& file;
  bool publicKeyInitialized = false;
  big RSA_N;
  big RSA_d;
  big RSA_e;

private:
  bool initRsaPublicKey(const unsigned char* crypted_sample);
  void DecryptBlock(const unsigned char* src, unsigned char* dst);
  bool Decrypt6432(const unsigned char* src, unsigned char* dst, size_t dstLen);

public:
  // The File object needs to be alive as long as the Rsa object is.
  Rsa(File& file);
  ~Rsa();
  bool read(void *buffer, size_t size);
  // Return the internal File object used.
  // You can use it for seek, errors etc.
  // This function may be removed in the future and replaced with functions in this class.
  File& getFile();

  static void initMiracl();
  static void freeMiracl();

  static const int KEY_SIZE = 512; // in bits
  static const int KEY_BYTESIZE = KEY_SIZE / 8;
};

#endif /* RSA_HPP_ */
