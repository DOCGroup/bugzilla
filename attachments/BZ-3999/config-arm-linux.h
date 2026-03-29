/* -*- C++ -*- */
//==========================================================================
/**
 *  @file   config-arm-linux.h
 *
 *  $Id: config-arm-linux.h shuston $
 *
 *  @author Chong Wuk Pak <chong.pak@lmco.com>
 *
 *  This file contains the contents of the config-arm-linux.h in order to
 *  build ACE+TAO for ARM processor using the cross platform toolchain that
 *  is built from uclibC.
 */
//==========================================================================

#ifndef ACE_CONFIG_H
#define ACE_CONFIG_H

#include "ace/config-linux.h"

#define ACE_LACKS_STROPTS_H
#define ACE_LACKS_GETLOADAVG

#if defined (ACE_HAS_STRBUF_T)
#undef ACE_HAS_STRBUF_T
#endif
#if defined (ACE_HAS_PTHREAD_SETSTACK)
#undef ACE_HAS_PTHREAD_SETSTACK
#endif
#define ACE_LACKS_PTHREAD_SETSTACK
#if defined (ACE_HAS_AIO_CALLS)
#undef ACE_HAS_AIO_CALLS
#endif
#if defined (ACE_HAS_GETIFADDRS)
#undef ACE_HAS_GETIFADDRS
#endif
#define ACE_LACKS_STRRECVFD

#define ACE_HAS_CPU_SET_T

#if defined (ACE_SCANDIR_CMP_USES_VOIDPTR)
#undef ACE_SCANDIR_CMP_USES_VOIDPTR
#endif

#if defined (ACE_SCANDIR_CMP_USES_CONST_VOIDPTR)
#undef ACE_SCANDIR_CMP_USES_CONST_VOIDPTR
#endif

#if defined (ACE_HAS_EXECINFO_H)
#undef ACE_HAS_EXECINFO_H
#endif

#if defined(__GLIBC__) 
#undef __GLIBC__
#endif

#if defined(ACE_HAS_SEMUN)
#undef ACE_HAS_SEMUN
#endif

#define ACE_LACKS_NETDB_REENTRANT_FUNCTIONS

#endif /* ACE_CONFIG_H */
