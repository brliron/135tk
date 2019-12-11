#ifndef RSA_HPP_
# define RSA_HPP_

# include <fstream>

extern "C"
{
# include "miracl.h"
}

class Rsa
{
public:
  enum class CryptMode {
    TH135,
    TH145
  };

private:
  static bool miraclInitialized;

  std::ifstream* ifile;
  std::ofstream* ofile;
  bool publicKeyInitialized = false;
  CryptMode cryptMode;
  big RSA_N;
  big RSA_d;
  big RSA_e;

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

  static void initMiracl();
  static void freeMiracl();
};

#endif /* RSA_HPP_ */
