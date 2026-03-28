// -*- C++ -*-

//=============================================================================
/**
 * @file    Numeric_Limits.h
 *
 * $Id$
 *
 * Traits containing basic integer limits.  Useful for template-based
 * code on platforms that lack @c std::numeric_limits<>.
 *
 * @note These traits are not meant to be a replacement for
 *       @c std::numeric_limits<>.  Rather they are a crutch until all
 *       ACE-supported platforms also support
 *       @c std::numeric_limits<>.
 *
 * @author  Ossama Othman <ossama_othman at symantec dot com>
 */
//=============================================================================

#ifndef ACE_NUMERIC_LIMITS_H
#define ACE_NUMERIC_LIMITS_H

#include /**/ "ace/pre.h"

#include "ace/ACE_Export.h"

# if !defined (ACE_LACKS_PRAGMA_ONCE)
#   pragma once
# endif /* ACE_LACKS_PRAGMA_ONCE */

#ifdef ACE_LACKS_NUMERIC_LIMITS
# include "ace/Basic_Types.h"
#else
# include <limits>
#endif /* ACE_LACKS_NUMERIC_LIMITS */

// Address global namespace pollution potentially incurred by some
// platforms.
#undef min
#undef max

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

#ifdef ACE_LACKS_NUMERIC_LIMITS

template <typename T> struct ACE_Numeric_Limits;


// ------------------------------------------
// Signed integers.

template<>
struct ACE_Export ACE_Numeric_Limits<ACE_INT8>
{
  static ACE_INT8 min (void) { return ACE_CHAR_MIN; }
  static ACE_INT8 max (void) { return ACE_CHAR_MAX; }
};

template<>
struct ACE_Export Limits<ACE_INT16>
{
  static ACE_INT16 min (void) { return ACE_INT16_MIN; }
  static ACE_INT16 max (void) { return ACE_INT16_MAX; }
};

template<>
struct ACE_Export Limits<ACE_INT32>
{
  static ACE_INT32 min (void) { return ACE_INT32_MIN; }
  static ACE_INT32 max (void) { return ACE_INT32_MAX; }
};

template<>
struct ACE_Export Limits<ACE_INT64>
{
  static ACE_INT64 min (void) { return ACE_INT64_MIN; }
  static ACE_INT64 max (void) { return ACE_INT64_MAX; }
};

// ------------------------------------------
// Unsigned integers

template<>
struct ACE_Export ACE_Numeric_Limits<ACE_UINT8>
{
  static ACE_UINT8 min (void) { return 0; }
  static ACE_UINT8 max (void) { return ACE_OCTET_MAX; }
};

template<>
struct ACE_Export Limits<ACE_UINT16>
{
  static ACE_UINT16 min (void) { return 0; }
  static ACE_UINT16 max (void) { return ACE_UINT16_MAX; }
};

template<>
struct ACE_Export Limits<ACE_UINT32>
{
  static ACE_UINT32 min (void) { return 0; }
  static ACE_UINT32 max (void) { return ACE_UINT32_MAX; }
};

template<>
struct ACE_Export Limits<ACE_UINT64>
{
  static ACE_UINT64 min (void) { return 0; }
  static ACE_UINT64 max (void) { return ACE_UINT64_MAX; }
};

// ------------------------------------------
// Floating point types

template<>
struct ACE_Export Limits<float>
{
  static float min (void) { return FLT_MIN; }
  static float max (void) { return FLT_MAX; }
};

template<>
struct ACE_Export Limits<double>
{
  static double min (void) { return DBL_MIN; }
  static double max (void) { return DBL_MAX; }
};

template<>
struct ACE_Export Limits<long double>
{
  static long double min (void) { return LDBL_MIN; }
  static long double max (void) { return LDBL_MAX; }
};

#else

// std::numeric_limits<> has all of the necessary specializations.
// Just wrap it.

template <typename T>
struct ACE_Export ACE_Numeric_Limits
{
  static T min (void) { return std::numeric_limits<T>::min (); }
  static T max (void) { return std::numeric_limits<T>::max (); }
};

#endif /* ACE_LACKS_NUMERIC_LIMITS */

ACE_END_VERSIONED_NAMESPACE_DECL

#include /**/ "ace/post.h"

#endif  /* ACE_NUMERIC_LIMITS_H */
