// -*- C++ -*-

#include "Network_Endpoint_Selector.h"

#include "tao/Invocation.h"
#include "tao/Stub.h"
#include "tao/Profile.h"
#include "tao/Endpoint.h"
#include "tao/Base_Transport_Property.h"
#include "tao/IIOP_Endpoint.h"
#include "tao/Connector_Registry.h"
#include "tao/Thread_Lane_Resources.h"
#include "tao/Transport_Cache_Manager.h"

ACE_RCSID (tao,
           Network_Endpoint_Selectors,
           "Network_Endpoint_Selectors.cpp,v 1.17 2001/07/12 01:29:15 othman Exp")


ACE_UINT32 TAO_Network_Endpoint_Selector::network_ = 0;
ACE_UINT32 TAO_Network_Endpoint_Selector::netmask_ = 0;

void MyTimeoutHook(TAO_ORB_Core * pOrbCore,
                                TAO_Stub * pStub,
                                int& has_timeout,
                                ACE_Time_Value& time_value)
{
  has_timeout = 1;
  time_value.set(60*5,0); 
}


// ****************************************************************
TAO_Network_Endpoint_Selector::TAO_Network_Endpoint_Selector (void)
{
}

TAO_Network_Endpoint_Selector::~TAO_Network_Endpoint_Selector (void)
{
}


int
TAO_Network_Endpoint_Selector::select_endpoint_i 
(
  int performConnect,  // if this is 0, we will check the cache only 
  int& multihomed,     // 
  const ACE_Time_Value* timeout, 
  TAO_GIOP_Invocation *invocation,
  CORBA::Environment &ACE_TRY_ENV)
{
  // First check to see if we already have a cached endpoint.  If so, use it
  // First look for an existing connection to one of the endpoints in this IOR
  ACE_DEBUG((LM_DEBUG, "--------------select_endpoint()---------------------------------------\n"));
  int endpoints=0;
  int Profile = 0;
  do
    {
      ACE_DEBUG((LM_DEBUG, "Profile %d with %d endpoints\n", Profile, invocation->stub ()->profile_in_use ()->endpoint_count()));
 
      invocation->profile (invocation->stub ()->profile_in_use ());
      invocation->endpoint (invocation->profile ()->endpoint ());

      size_t endpoint_count = invocation->profile ()->endpoint_count();
      for (size_t i = 0; i < endpoint_count; ++i)
      {
          int status=-1;
          // If known endpoint, select it.
          if (invocation->endpoint () != 0)
          {
              // Network matches, attempt to make the call
              TAO_Base_Transport_Property desc (invocation->endpoint ());
              TAO_Transport *base_transport = 0;
              
              char buffer[256];
              desc.endpoint()->addr_to_string(buffer,sizeof(buffer));
              ACE_DEBUG((LM_DEBUG, "profile %d endpoint %d info: IP=%s, hash = %d\n", Profile, i, buffer, desc.endpoint()->hash()));

              if(performConnect)
              {
                ACE_Time_Value orig_max_timeout;
                int reset_timeout=0;
                if(invocation->max_wait_time() && timeout)
                {
                  ACE_DEBUG((LM_DEBUG, "Multiple endpoints are present, using %d second timeout for connection\n", timeout->sec()));
                  orig_max_timeout = *invocation->max_wait_time();
                  *invocation->max_wait_time() = *timeout;                    
                  reset_timeout=1;
                }

                 status = invocation->perform_call (desc, ACE_TRY_ENV);
               
                 if(reset_timeout)
                 {
                    *invocation->max_wait_time() = orig_max_timeout;
                 }
                 ACE_CHECK;

               } else if(invocation->orb_core()->lane_resources ().transport_cache ().find_transport(&desc, base_transport) == 0)
               { 
                 // its in the cache so the connection will be reused
                 // and the perform_call will return quickly
                 status = invocation->perform_call (desc, ACE_TRY_ENV);
                 ACE_CHECK;
               } 
            
                // Check if the invocation has completed.
                if (status == 1)
                  return 0;  // found in the cache            
            }
              ACE_DEBUG((LM_DEBUG, "cach miss in profile %d endpoint %d\n", Profile, i));

              // Go to the next endpoint in this profile.
              invocation->endpoint (invocation->endpoint()->next());
              ++endpoints;
         }
      ++Profile;
    }
  while (invocation->stub ()->next_profile_retry () != 0);
  
  // at this point we know whether we are multihomed or not and how many endpoints
  // are in all the profiles
  if(endpoints>1)
    multihomed=1;
  
  return -1; // none found
}


void
TAO_Network_Endpoint_Selector::select_endpoint (
  TAO_GIOP_Invocation *invocation,
  CORBA::Environment &ACE_TRY_ENV)
{
  // First check to see if we already have a cached endpoint.  If so, use it
  // First look for an existing connection to one of the endpoints in this IOR
  ACE_DEBUG((LM_DEBUG, "--------------select_endpoint()---------------------------------------\n"));
  // check the cache to see if one of our endpoints is in the cache.
  int multihomed=0; // must set this to zero to start with
  ACE_DEBUG((LM_DEBUG, "Checking Cache\n"));
  if(select_endpoint_i(0,multihomed,0,invocation,ACE_TRY_ENV)==0)
    return; // found an end point in the cache

  if(multihomed)
  {
    
    // Next, try and connect to all endpoints using a fast (1 second) timeout
    ACE_DEBUG((LM_DEBUG, "Performing Multi-Homed Fast Connection\n")); 
    if(select_endpoint_i(1,multihomed,&ACE_Time_Value(1),invocation,ACE_TRY_ENV)==0)
      return; // found an end point using a fast connection check
  }

  // Finally, since we have exhausted our attempts at connecting fall back
  // to a hard network connection using the default timeout policy.
  ACE_DEBUG((LM_DEBUG, "Peforming Normal Connection\n")); 
  if(select_endpoint_i(1,multihomed,0,invocation,ACE_TRY_ENV)==0)
    return; // found an end point using the default connection timeout

  // If we get here, we completely failed to find an endpoint selector
  // that we know how to use, so throw an exception.
  ACE_DEBUG((LM_DEBUG, "no endpoints are accessable, giving up\n"));
  ACE_THROW (CORBA::TRANSIENT (TAO_OMG_VMCID | 2,
                               CORBA::COMPLETED_NO));
}

// @@ RTCORBA_Subsetting - next should be deprecated...
void
TAO_Network_Endpoint_Selector::next (TAO_GIOP_Invocation *,
                                     CORBA::Environment &)
{
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("This method is DEPRECATED!\n")));
  // if (invocation->stub_->next_profile_retry () == 0)
  //     ACE_THROW (CORBA::TRANSIENT (TAO_OMG_VMCID | 2,
  //                                  CORBA::COMPLETED_NO));
}

void
TAO_Network_Endpoint_Selector::forward (TAO_GIOP_Invocation
                                        *invocation,
                                        const TAO_MProfile &mprofile,
                                        CORBA::Environment &ACE_TRY_ENV)
{
  invocation->stub ()->add_forward_profiles (mprofile);
  // This has to be and is thread safe.
  // TAO_Stub::add_forward_profiles() already makes a copy of the
  // MProfile, so do not make a copy here.


  // We may not need to do this since TAO_GIOP_Invocations
  // get created on a per-call basis. For now we'll play it safe.
  if (invocation->stub ()->next_profile () == 0)
    ACE_THROW (CORBA::TRANSIENT (
                                 CORBA_SystemException::_tao_minor_code (
                                       TAO_INVOCATION_LOCATION_FORWARD_MINOR_CODE,
                                       errno),
                                 CORBA::COMPLETED_NO));
}

void
TAO_Network_Endpoint_Selector::success (TAO_GIOP_Invocation *invocation)
{
  invocation->stub ()->set_valid_profile ();
}

void
TAO_Network_Endpoint_Selector::close_connection (TAO_GIOP_Invocation *invocation)
{
  // Get rid of any forwarding profiles and reset
  // the profile list to point to the first profile!
  // FRED For now we will not deal with recursive forwards!
  invocation->stub ()->reset_profiles ();
}

void 
TAO_Network_Endpoint_Selector::initialize (ACE_UINT32 network, ACE_UINT32 netmask)
{
  network_ = network;
  netmask_ = netmask;
}


