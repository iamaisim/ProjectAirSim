// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifdef __linux__

#warning Setting OPENSSL to v1.1.0 compatibility mode--upgrade to v3 mode when possible
#define OPENSSL_API_COMPAT 0x10100000L

#include <arpa/inet.h>
#include <assert.h>
#include <byteswap.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <stdint.h>
#include <stdio.h>

#include <chrono>
#include <cstring>
#include <string>
#include <vector>

#include "core_sim/client_authorization.hpp"

namespace microsoft {
namespace projectairsim {

// Client authorization handle implementation for Linux
class ImplLinux : public ClientAuthorization::Impl {
 public:
  ImplLinux(void);
  virtual ~ImplLinux();

  virtual bool SetPublicKey(const char* sz, size_t cch) override;
  virtual uint64_t SetToken(const char* sz, size_t cch) override;

 protected:
  virtual std::string GetAuthorizationTokenPublicKey(void) override;

 private:
  typedef std::vector<uint8_t> VecB;  // Vector of bytes
  typedef int errno_t;

 private:
  static std::string GetOpenSSLError(void);

 private:
  bool Base64Decode(const char* pchBase64, size_t cchBase64, VecB* pvecRet);
  bool Base64Encode(const uint8_t* rgb, size_t cb, std::string* pstrBase64Ret);
  bool VerifyECDsaSignature(const VecB& vecbMessage,
                            const VecB& vecbSignatureRFC3279,
                            const VecB vecbASN1PublicKeyInfo);

 private:
  RSA* prsa_token_transport_;   // Encryption key for transporting token
  VecB vecb_ecdsa_public_key_;  // ECDsa public key
};                              // class ImplLinux

ImplLinux::ImplLinux(void)
    : ClientAuthorization::Impl(),
      prsa_token_transport_(nullptr),
      vecb_ecdsa_public_key_() {}

ImplLinux::~ImplLinux() {
  if (prsa_token_transport_ != nullptr) RSA_free(prsa_token_transport_);
}

bool ImplLinux::Base64Decode(const char* rgchBase64, size_t cchBase64,
                             VecB* pvecbRet) {
  bool fOK = true;
  size_t cb;
  BIO* pbioB64 = nullptr;
  BIO* pbioMem = nullptr;

  // Note that BIO_f_base64 requires the first line to be 1024 bytes or less
  // and we don't expect any of our strings to be that long, so we don't handle
  // that case.  If this assert first, that's no true and we'll have to handle
  // inserting a newline after the first 1024 bytes (the remainder of the input
  // can be left as one long line.)
  assert(cchBase64 <= 1024);

  // Create memory buffer with data to decode
  if ((pbioMem = BIO_new_mem_buf(rgchBase64, cchBase64)) == nullptr) {
    logger_.LogError(str_component_name_,
                     "ClientAuthorization::ImplLinux::Base64Decode(): "
                     "BIO_new_mem_buf failed");
    goto LError;
  }

  // Create and attach base64 filter to filter chain so it processes data read
  // from memory buffer
  if ((pbioB64 = BIO_new(BIO_f_base64())) == nullptr) {
    logger_.LogError(str_component_name_,
                     "ClientAuthorization::ImplLinux::Base64Decode(): "
                     "BIO_new(BIO_f_base64()) failed");
    goto LError;
  }
  pbioB64 = BIO_push(pbioB64, pbioMem);
  BIO_set_flags(pbioB64,
                BIO_FLAGS_BASE64_NO_NL);  // The input won't have newlines

  // Read the decoded (binary) data
  pvecbRet->resize(cchBase64);
  cb = BIO_read(pbioB64, pvecbRet->data(), cchBase64);
  if (cb <= 0) {
    logger_.LogError(
        str_component_name_,
        "ClientAuthorization::ImplLinux::Base64Decode(): BIO_read failed");
    goto LError;
  }
  pvecbRet->resize(cb);

LDone:
  // Clean up
  if (pbioB64)
    BIO_free_all(pbioB64);
  else if (pbioMem)
    BIO_free(pbioMem);

  return (fOK);

LError:
  fOK = false;
  goto LDone;
}

bool ImplLinux::Base64Encode(const uint8_t* rgb, size_t cb,
                             std::string* pstrBase64Ret) {
  bool fOK = true;
  BIO* pbioB64 = nullptr;
  BIO* pbioMem = nullptr;

  // Create memory buffer filter to receive the encoded data
  if ((pbioMem = BIO_new(BIO_s_mem())) == nullptr) {
    logger_.LogError(str_component_name_,
                     "ClientAuthorization::ImplLinux::Base64Encode(): "
                     "BIO_new(BIO_s_mem()) failed");
    goto LError;
  }

  // Create and attach base64 filter to filter chain so it writes data to memory
  // buffer
  if ((pbioB64 = BIO_new(BIO_f_base64())) == nullptr) {
    logger_.LogError(str_component_name_,
                     "ClientAuthorization::ImplLinux::Base64Encode(): "
                     "BIO_new(BIO_f_base64()) failed");
    goto LError;
  }
  pbioB64 = BIO_push(pbioB64, pbioMem);
  BIO_set_flags(pbioB64,
                BIO_FLAGS_BASE64_NO_NL);  // The input won't have newlines

  // Write the binary data
  BIO_write(pbioB64, rgb, cb);
  BIO_flush(pbioB64);

  // Get the encoded data
  {
    BUF_MEM* pbufmem;

    BIO_get_mem_ptr(pbioB64, &pbufmem);
    if (pbufmem->length <= 0) {
      logger_.LogError(str_component_name_,
                       "ClientAuthorization::ImplLinux::Base64Encode(): failed "
                       "to decode base-64 data");
      goto LError;
    }
    pstrBase64Ret->append(
        reinterpret_cast<char*>(pbufmem->data),
        reinterpret_cast<char*>(pbufmem->data + pbufmem->length));
  }

LDone:
  // Clean up
  if (pbioB64)
    BIO_free_all(pbioB64);
  else if (pbioMem)
    BIO_free(pbioMem);

  return (fOK);

LError:
  fOK = false;
  goto LDone;
}

std::string ImplLinux::GetAuthorizationTokenPublicKey(void) {
  if (prsa_token_transport_ == nullptr) {
    static const char* c_szAlgorithm = "ssh-rsa";
    std::string strKeyBase64;
    VecB vecbKey;

    auto fnStoreOpenSSHField = [](const uint8_t* pbData, size_t cbData,
                                  VecB* pvecbInOut) {
      size_t cbCur;
      uint32_t cbT;
      uint8_t* pb;

      cbCur = pvecbInOut->size();
      pvecbInOut->resize(cbCur + sizeof(cbT) + cbData);
      pb = pvecbInOut->data() + cbCur;
      cbT = htonl(cbData);
      memcpy(pb, reinterpret_cast<uint8_t*>(&cbT), sizeof(cbT));
      pb += sizeof(cbT);
      memcpy(pb, pbData, cbData);
    };
    auto fnStoreOpenSSHNumber = [fnStoreOpenSSHField](const BIGNUM* pbignum,
                                                      VecB* pvecbInOut) {
      VecB vecbT;
      int cb;
      size_t cbData;

      cb = BN_num_bytes(pbignum);
      vecbT.resize(cb);
      BN_bn2bin(pbignum, vecbT.data());

      if (vecbT[0] > 0x7f) vecbT.insert(vecbT.begin(), 0);

      fnStoreOpenSSHField(vecbT.data(), vecbT.size(), pvecbInOut);
    };

    // Generate RSA keys
    {
      BIGNUM* pbnE = BN_new();
      int iResult;

      prsa_token_transport_ = RSA_new();

      BN_set_word(pbnE, RSA_F4);
      iResult = RSA_generate_key_ex(
          prsa_token_transport_,
          ClientAuthorization::TokenV1::kRSAKeySizeTokenTransport, pbnE,
          nullptr);
      BN_free(pbnE);
      if (iResult == 0) {
        char sz[256];

        sprintf(sz, "RSA_generate_key_ex() returned error: %s",
                GetOpenSSLError().c_str());
        throw std::logic_error(sz);
      }
    }

    fnStoreOpenSSHField(reinterpret_cast<const uint8_t*>(c_szAlgorithm),
                        strlen(c_szAlgorithm), &vecbKey);
    fnStoreOpenSSHNumber(RSA_get0_e(prsa_token_transport_), &vecbKey);
    fnStoreOpenSSHNumber(RSA_get0_n(prsa_token_transport_), &vecbKey);

    if (!Base64Encode(vecbKey.data(), vecbKey.size(), &strKeyBase64))
      throw std::logic_error("Base64Encode() failed");

    str_base64_token_encryption_public_key_ = c_szAlgorithm;
    str_base64_token_encryption_public_key_ += " ";
    str_base64_token_encryption_public_key_ += strKeyBase64;
  }

  return (str_base64_token_encryption_public_key_);
}

std::string ImplLinux::GetOpenSSLError(void) {
  char sz[256];

  return (ERR_error_string(ERR_get_error(), sz));
}

bool ImplLinux::VerifyECDsaSignature(const VecB& vecbMessage,
                                     const VecB& vecbSignatureRFC3279,
                                     const VecB vecbASN1PublicKeyInfo) {
  bool fOK = true;
  EVP_PKEY* pkey = nullptr;
  const EVP_MD* pmd = nullptr;
  EVP_MD_CTX* pmdctx = nullptr;
  int iVerify;
  EVP_PKEY* pkeyBuf = nullptr;

  // Load the key
  {
    const unsigned char* puch = vecbASN1PublicKeyInfo.data();

    if ((pkey = d2i_PUBKEY(&pkeyBuf, &puch, vecbASN1PublicKeyInfo.size())) ==
        nullptr)
      goto LError;
  }

  // Create message digest context
  if ((pmdctx = EVP_MD_CTX_create()) == nullptr) goto LError;

  // Get the digest hash function--this is the function used to create the
  // signature
  if ((pmd = EVP_get_digestbyname("sha512")) == nullptr) goto LError;

  // Set the context to use the hash function
  if (!EVP_VerifyInit_ex(pmdctx, pmd, NULL)) goto LError;

  // Set the message data into the digest context
  if (!EVP_VerifyUpdate(pmdctx, vecbMessage.data(), vecbMessage.size()))
    goto LError;

  // Verify the signature matches the message data and hash function
  iVerify = EVP_VerifyFinal(pmdctx, vecbSignatureRFC3279.data(),
                            vecbSignatureRFC3279.size(), pkey);
  if (iVerify < 0) goto LError;
  fOK = !!iVerify;

LDone:
  // Cleanup
  if (pmdctx) EVP_MD_CTX_free(pmdctx);
  if (pkey) EVP_PKEY_free(pkey);

  return (fOK);

LError:
  fOK = false;
  goto LDone;
}

bool ImplLinux::SetPublicKey(const char* sz, size_t cch) {
  Impl::SetPublicKey(sz, cch);
  if (cch == 0) {
    vecb_ecdsa_public_key_.clear();
    logger_.LogVerbose(str_component_name_,
                       "Client authorization public key cleared--all clients "
                       "accepted and authorization tokens not required");
    return (true);
  } else {
    bool fOK = Base64Decode(sz, cch, &vecb_ecdsa_public_key_);

    logger_.LogVerbose(str_component_name_,
                       "Setting public key to (%d chars) \"%s\"", cch, sz);
    if (fOK)
      logger_.LogVerbose(str_component_name_,
                         "Client authorization public key set--all clients "
                         "required to present authorization token");
    else
      logger_.LogError(str_component_name_,
                       "Failed to set client authorization public key");

    return (fOK);
  }
}

uint64_t ImplLinux::SetToken(const char* rgch, size_t cch) {
  uint64_t timestamp_expiration;
  ClientAuthorization::TokenV1 tokenv1;
  VecB vecb_token;

  // If we don't have a valid public key, return a timestamp reflecting the
  // current expiration point
  if (vecb_ecdsa_public_key_.empty()) {
    if (time_point_when_expired_ == kTimePointAlwaysAuthorized) {
      logger_.LogVerbose(str_component_name_,
                         "Client authorization token accepted (no client "
                         "authorization token public key)");
      return (ClientAuthorization::TokenV1::kTimestampExpirationNone);
    } else {
      logger_.LogVerbose(str_component_name_,
                         "Client authorization token rejected (invalid client "
                         "authorization token public key)");
      return (0);
    }
  }

  // If no token given, clear the client authorization
  if (cch == 0) {
    time_point_when_expired_ = kTimePointNotAuthorized;
    logger_.LogVerbose(str_component_name_, "Client authorization removed");
    return (0);
  }

  if (prsa_token_transport_ == nullptr) {
    logger_.LogError(str_component_name_, "No token transport encryption key");
    throw std::invalid_argument("token is not valid");
  }

  // Decrypt the token
  {
    int cb;
    VecB vecb_encrypted;
    VecB vecb_token_base64;

    // Get the binary ciphertext
    if (!Base64Decode(rgch, cch, &vecb_encrypted)) {
      logger_.LogError(
          str_component_name_,
          "Client authorization token could not be base-64 decoded");
      throw std::invalid_argument("token is not valid");
    }

    // Decode the ciphertext
    vecb_token_base64.resize(vecb_encrypted.size());
    cb = RSA_private_decrypt(vecb_encrypted.size(), vecb_encrypted.data(),
                             vecb_token_base64.data(), prsa_token_transport_,
                             RSA_PKCS1_OAEP_PADDING);
    if (cb < 0) {
      logger_.LogError(str_component_name_,
                       "Client authorization token could not be decrypted: %s",
                       GetOpenSSLError().c_str());
      throw std::runtime_error("encrypted token is not valid");
    }
    vecb_token_base64.resize(cb);

    if (!Base64Decode(reinterpret_cast<char*>(vecb_token_base64.data()),
                      vecb_token_base64.size(), &vecb_token)) {
      logger_.LogError(str_component_name_,
                       "Client authorization token after decryption could not "
                       "be base-64 decoded");
      throw std::invalid_argument("token is not valid");
    }
  }

  // Get the token data and verify the signature
  {
    auto it = vecb_token.begin();

    // Get the binary token data
    tokenv1.version = *it++;
    if ((vecb_token.end() - it) < sizeof(tokenv1.timestamp_expiration)) {
      logger_.LogError(str_component_name_,
                       "Client authorization token is too short");
      throw std::length_error("signature is too short");
    }

    memcpy(&tokenv1.timestamp_expiration, &*it,
           sizeof(tokenv1.timestamp_expiration));
    it += sizeof(tokenv1.timestamp_expiration);

    // Verify we support the token version
    if (tokenv1.version != 1) {
      logger_.LogError(str_component_name_,
                       "Client authorization token version %d is not supported",
                       tokenv1.version);
      throw std::invalid_argument("token version is not supported");
    }

    // Verify the signature
    if (!VerifyECDsaSignature(VecB(vecb_token.begin(), it),
                              VecB(it, vecb_token.end()),
                              vecb_ecdsa_public_key_)) {
      logger_.LogError(str_component_name_,
                       "Client authorization token signature does not verify");
      throw std::invalid_argument("token is not valid");
    }
  }

  // Get the timestamp, converting from big-endian to platform's byte order
  memcpy(&timestamp_expiration, &tokenv1.timestamp_expiration,
         sizeof(tokenv1.timestamp_expiration));
  timestamp_expiration = bswap_64(timestamp_expiration);

  // Convert the timestamp to a time point and save
  time_point_when_expired_ =
      kTimePointEpochStart + std::chrono::seconds(timestamp_expiration);

  {
    char sz[128];
    std::time_t time_expiration =
        std::chrono::system_clock::to_time_t(time_point_when_expired_);
    std::tm tm = *std::gmtime(&time_expiration);

    std::strftime(sz, sizeof(sz), "%c", &tm);
    logger_.LogVerbose(
        str_component_name_,
        "Client authorization token accepted--client authorized until %s UTC",
        sz);
  }

  return (timestamp_expiration);
}

}  // namespace projectairsim
}  // namespace microsoft

#endif  //__linux__
