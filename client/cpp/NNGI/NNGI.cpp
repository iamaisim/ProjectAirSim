// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "pch.h"
#include "NNGI.h"

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include <nng/protocol/reqrep0/req.h>


namespace nngi
{

NNGI_DECL int nng_close(nng_socket s)
{
    return (::nng_close(*reinterpret_cast<::nng_socket*>(&s)));
}



NNGI_DECL int nng_dial(nng_socket sid, const char *addr, nng_dialer *dp, int flags)
{
    return (::nng_dial(*reinterpret_cast<::nng_socket*>(&sid), addr, reinterpret_cast<::nng_dialer*>(dp), flags));
}



NNGI_DECL void nng_free(void *buf, size_t sz)
{
    return (::nng_free(buf, sz));
}



NNGI_DECL int nng_pair0_open(nng_socket *sid)
{
    return (::nng_pair0_open(reinterpret_cast<::nng_socket*>(sid)));
}



NNGI_DECL int nng_recv(nng_socket s, void *buf, size_t *szp, int flags)
{
    return (::nng_recv(*reinterpret_cast<::nng_socket*>(&s), buf, szp, flags));
}



NNGI_DECL int nng_req0_open(nng_socket *sid)
{
    return (::nng_req0_open(reinterpret_cast<::nng_socket*>(sid)));
}



NNGI_DECL int nng_send(nng_socket s, void *buf, size_t len, int flags)
{
    return (::nng_send(*reinterpret_cast<::nng_socket*>(&s), buf, len, flags));
}



NNGI_DECL int nng_socket_set_ms(nng_socket id, const char *n, nng_duration v)
{
    return (::nng_socket_set_ms(*reinterpret_cast<::nng_socket*>(&id), n, v));
}



NNGI_DECL const char *nng_strerror(int num)
{
    return (::nng_strerror(num));
}



NNGI_DECL const char* nng_version(void)
{
    return (::nng_version());
}


} //namespace nngisolated
