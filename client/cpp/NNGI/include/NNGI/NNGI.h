#pragma once
#include "NNGIDecl.h"
#include <cstdint>



namespace nngi
{

struct nng_dialer;



#define NNG_OPT_RECVTIMEO "recv-timeout"
#define NNG_OPT_SENDTIMEO "send-timeout"
#define NNG_OPT_REQ_RESENDTIME "req:resend-time"
#define NNG_FLAG_ALLOC 1u // Recv to allocate receive buffer
#define NNG_SOCKET_INITIALIZER { 0 }


typedef struct nng_socket_s {
	uint32_t id;
} nng_socket;
typedef std::int32_t nng_duration; // in milliseconds


enum nng_errno_enum {
	NNG_EINTR        = 1,
	NNG_ENOMEM       = 2,
	NNG_EINVAL       = 3,
	NNG_EBUSY        = 4,
	NNG_ETIMEDOUT    = 5,
	NNG_ECONNREFUSED = 6,
	NNG_ECLOSED      = 7,
	NNG_EAGAIN       = 8,
	NNG_ENOTSUP      = 9,
	NNG_EADDRINUSE   = 10,
	NNG_ESTATE       = 11,
	NNG_ENOENT       = 12,
	NNG_EPROTO       = 13,
	NNG_EUNREACHABLE = 14,
	NNG_EADDRINVAL   = 15,
	NNG_EPERM        = 16,
	NNG_EMSGSIZE     = 17,
	NNG_ECONNABORTED = 18,
	NNG_ECONNRESET   = 19,
	NNG_ECANCELED    = 20,
	NNG_ENOFILES     = 21,
	NNG_ENOSPC       = 22,
	NNG_EEXIST       = 23,
	NNG_EREADONLY    = 24,
	NNG_EWRITEONLY   = 25,
	NNG_ECRYPTO      = 26,
	NNG_EPEERAUTH    = 27,
	NNG_ENOARG       = 28,
	NNG_EAMBIGUOUS   = 29,
	NNG_EBADTYPE     = 30,
	NNG_ECONNSHUT    = 31,
	NNG_EINTERNAL    = 1000,
	NNG_ESYSERR      = 0x10000000,
	NNG_ETRANERR     = 0x20000000
};


NNGI_DECL int nng_close(nng_socket s);
NNGI_DECL int nng_dial(nng_socket sid, const char *addr, nng_dialer *dp, int flags);
NNGI_DECL void nng_free(void *buf, size_t sz);
NNGI_DECL int nng_pair0_open(nng_socket *);
NNGI_DECL int nng_recv(nng_socket s, void *buf, size_t *szp, int flags);
NNGI_DECL int nng_req0_open(nng_socket *);
NNGI_DECL int nng_send(nng_socket s, void *buf, size_t len, int flags);
NNGI_DECL int nng_socket_set_ms(nng_socket id, const char *n, nng_duration v);
NNGI_DECL const char *nng_strerror(int);
NNGI_DECL const char* nng_version(void);


} //namespace nngisolated
