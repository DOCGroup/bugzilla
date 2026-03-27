// This may look like C, but it's really -*- C++ -*-

//=============================================================================
/**
 *  @file   Network_Endpoint_Selector.h
 *
 *
 * Strategies for selecting a profile/endpoint from an IOR that is in a
 * specified network for making an invocation.
 *
 *  @author Chris Hafey <chafey@stentor.com>
 */
//=============================================================================


#ifndef TAO_NETWORK_ENDPOINT_SELECTOR_H
#define TAO_NETWORK_ENDPOINT_SELECTOR_H
#include "ace/pre.h"

#include "tao/corbafwd.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "TAO/Invocation_Endpoint_Selectors.h"

// ****************************************************************

class TAO_Network_Endpoint_Selector :
  public TAO_Invocation_Endpoint_Selector
{
  // = TITLE
  //   Network based strategy for endpoint selection.
  //
  // = DESCRIPTION
  //   This strategy is used when the client desires to use a profile
  //   with and endpoint on a specific network.  This is necessary
  //   for non bridged multi-home servers as well as NAT configurations.
  //
public:
  /// Constructor.
  TAO_Network_Endpoint_Selector (void);

  /// Destructor.
  virtual ~TAO_Network_Endpoint_Selector (void);

  virtual void select_endpoint (TAO_GIOP_Invocation *invocation,
                                CORBA::Environment &ACE_TRY_ENV =
                                TAO_default_environment ());
  virtual void next (TAO_GIOP_Invocation *invocation,
                     CORBA::Environment &ACE_TRY_ENV =
                     TAO_default_environment ());
  virtual void forward (TAO_GIOP_Invocation *invocation,
                        const TAO_MProfile &mprofile,
                        CORBA::Environment &ACE_TRY_ENV =
                        TAO_default_environment ());
  virtual void success (TAO_GIOP_Invocation *invocation);
  virtual void close_connection (TAO_GIOP_Invocation *invocation);

  static void initialize (ACE_UINT32 network, ACE_UINT32 netmask);
private:
  int select_endpoint_i (int performConnect,
                         int& multihomed,
                         const ACE_Time_Value* timeout, 
                         TAO_GIOP_Invocation *invocation,
                         CORBA::Environment &ACE_TRY_ENV);

private:
  // The network to select a profile from
  static ACE_UINT32    network_;
  
  // The netmask to apply when determining a matching profile
  static ACE_UINT32    netmask_;
};

#include "ace/post.h"
#endif  /* TAO_NETWORK_ENDPOINT_SELECTOR_H */
