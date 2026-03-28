#include "ace/Get_Opt.h"
#include "ace/Task.h"
#include <tao/corba.h>
#include <tao/Utils/ORB_Manager.h>
#include <tao/AnyTypeCode/Any.h>
#include <tao/TimeBaseC.h>
#include <tao/Messaging/Messaging.h>
#include <tao/ORBInitializer_Registry.h>
#include <tao/PI/ORBInitializerC.h>

#include <sstream>
#include <stdexcept>

int g_nthreads = 1;
bool g_setTimeout = true;
bool g_doNotRegisterORBinitializer = false;
bool g_initInMain = false;
int g_argc;
char ** g_argv;

int initORB(int threadID);

int
parse_args (int argc, char *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, "min:");
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'm':
        g_initInMain = true;
        break;

      case 'i':
        g_doNotRegisterORBinitializer = true;
        break;

      case 'n':
        g_nthreads = ACE_OS::atoi (get_opts.opt_arg ());
        break;

      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s "
                           "-m (Initialize an ORB from the main thread first) "
                           "-i (Do not register ORB initializer) "
                           "-n <numberOfThreads>"
                           "\n",
                           argv [0]),
                          -1);
      }
  // Indicates sucessful parsing of the command line
  return 0;
}

class Worker : public ACE_Task_Base
{
public:
  Worker ();

  virtual int svc (void);

private:
};

class MyORBinitializer
  : public PortableInterceptor::ORBInitializer
{
public:
  MyORBinitializer( std::string orbID )
    : orbID_( orbID ) {};

  virtual
  void
  pre_init( PortableInterceptor::ORBInitInfo_ptr )
  {
    ACE_DEBUG ((LM_DEBUG, 
                "MyORBinitializer::pre_init() called for ORB \"%s\"\n",
                orbID_.c_str()));
  };

  virtual
  void
  post_init( PortableInterceptor::ORBInitInfo_ptr )
  {
    ACE_DEBUG ((LM_DEBUG, 
                "MyORBinitializer::post_init() called for ORB \"%s\"\n",
                orbID_.c_str()));
  };

  private:
    std::string orbID_;
};

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  g_argc = argc;
  g_argv = argv;

  try
  {
    if (parse_args (argc, argv) != 0)
      return 1;
   
    if ( g_initInMain )
    {
      initORB(0);
    }
//    else
    {
      Worker worker;
      if (worker.activate (THR_NEW_LWP | THR_JOINABLE,
                           g_nthreads) != 0)
        ACE_ERROR_RETURN ((LM_ERROR,
                           "Cannot activate ORB thread(s)\n"),
                          1);

      worker.thr_mgr ()->wait ();

    }
    ACE_DEBUG ((LM_DEBUG, "Event loop finished\n"));
  }
  catch (const CORBA::Exception& ex)
  {
    ex._tao_print_exception ("Exception caught:");
    return 1;
  }

  return 0;
}

// ****************************************************************

Worker::Worker ()
{
}

int
Worker::svc (void)
{
  static int threadID = 0;

  initORB(++threadID);

  return 0;
}

int
initORB(int threadID)
{
  try
  {
    std::ostringstream oss;
    oss << "ORB_" << threadID;
    std::string ORBid( oss.str() );

    ACE_DEBUG ((LM_DEBUG, "Initializing ORB \"%s\"\n", ORBid.c_str()));

    if ( ! g_doNotRegisterORBinitializer )
    {
      ACE_DEBUG ((LM_DEBUG, "Creating ORB initializer\n"));
      PortableInterceptor::ORBInitializer_var rCOI(
        new MyORBinitializer( ORBid ) );
      PortableInterceptor::register_orb_initializer( rCOI.in() );
    }

    std::auto_ptr< TAO_ORB_Manager > pORBmgr;

    ACE_DEBUG ((LM_DEBUG, "Creating TAO_ORB_Manager\n"));
    pORBmgr.reset( new TAO_ORB_Manager );

    if ( -1 == pORBmgr->init( g_argc, g_argv, ORBid.c_str() ) )
    {
      ACE_DEBUG ((LM_DEBUG, "Failed to initialize ORB \"%s\"\n", 
                  ORBid.c_str()));
      throw std::runtime_error( "Failed to initialize TAO_ORB_Manager!" );
    }

    ACE_DEBUG ((LM_DEBUG, "ORB \"%s\" initialized\n", ORBid.c_str()));

    if ( g_setTimeout )
    {
      ACE_DEBUG ((LM_DEBUG, 
                  "Setting connection timeout policy for ORB \"%s\"\n", 
                  ORBid.c_str()));

      CORBA::PolicyList policyList;

      CORBA::ORB_var orb = pORBmgr->orb();
    
      TimeBase::TimeT connectionTimeout = 100*10000;
      CORBA::Any any;
      any <<= connectionTimeout;

      CORBA::ULong l( policyList.length() );
      policyList.length( l+1 );

      policyList[l] = orb->create_policy(
        TAO::CONNECTION_TIMEOUT_POLICY_TYPE,
        any );

      ACE_DEBUG ((LM_DEBUG, "Connection timeout policy set for ORB \"%s\"\n", 
                  ORBid.c_str()));
    }

    pORBmgr->fini();
  }
  catch (const CORBA::Exception& ex)
  {
    ACE_ERROR ((LM_ERROR, "Caught exception: %s\n", ex._info().c_str()));
    return 1;
  }

  return 0;
}
