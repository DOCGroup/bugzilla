// -*- C++ -*-

//=============================================================================
/**
 *  @file    Service_Config_Ext_Test.cpp
 *
 *  $Id: Service_Config_Ext_Test.cpp 84175 2009-01-15 18:52:34Z johnnyw $
 *
 *  This is a simple test to make sure the ACE Service Configurator
 *  framework is working correctly.
 *
 *  @author David Levine <levine@cs.wustl.edu>
 *  @author Ossama Othman <ossama@uci.edu>
 */
//=============================================================================

#include "test_config.h"

#include "ace/Service_Config.h"
#include "ace/Reactor.h"
#include "ace/Thread_Manager.h"
#include "ace/ARGV.h"

ACE_RCSID (tests,
           Service_Config_Ext_Test,
           "$Id: Service_Config_Ext_Test.cpp 84175 2009-01-15 18:52:34Z johnnyw $")

static u_int error = 0;

void
testLoadingServiceConfFileExt (int argc, ACE_TCHAR *argv[])
{
#if defined (ACE_USES_WCHAR)
  // When using full Unicode support, use the version of the Service
  // Configurator file appropriate to the platform.
  // For example, Windows Unicode uses UTF-16.
  //
  //          iconv(1) found on Linux and Solaris, for example, can
  //          be used to convert between encodings.
  //
  //          Byte ordering is also an issue, so we should be
  //          generating this file on-the-fly from the UTF-8 encoded
  //          file by using functions like iconv(1) or iconv(3).
#  if defined (ACE_WIN32)
  const ACE_TCHAR svc_conf[] =
    ACE_TEXT ("Service_Config_Test.UTF-16.conf.xml");
#  else
  const ACE_TCHAR svc_conf[] =
    ACE_TEXT ("Service_Config_Test.WCHAR_T.conf.xml");
#  endif /* ACE_WIN32 */
#else
    // ASCII (UTF-8) encoded Service Configurator file.
  const ACE_TCHAR svc_conf[] =
    ACE_TEXT ("Service_Config_Test.conf.xml");
#endif  /* ACE_USES_WCHAR */

  // We need this scope to make sure that the destructor for the
  // <ACE_Service_Config> gets called.
  ACE_Service_Config daemon;

  if (daemon.process_file (svc_conf))
    {
      if (errno == ENOENT)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("ACE_Service_Config::open: %p\n"),
                    svc_conf));
      else
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("ACE_Service_Config::open: %p\n"),
                    ACE_TEXT ("error")));
    }

  ACE_Time_Value tv (argc > 1 ? ACE_OS::atoi (argv[1]) : 2);

  if (ACE_Reactor::instance()->run_reactor_event_loop (tv) == -1)
    {
      ++error;
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("line %l %p\n"),
                  ACE_TEXT ("run_reactor_event_loop")));
    }

  // Wait for all threads to complete.
  ACE_Thread_Manager::instance ()->wait ();
}

int
run_main (int argc, ACE_TCHAR *argv[])
{
  ACE_START_TEST (ACE_TEXT ("Service_Config_Ext_Test"));

#if (ACE_USES_CLASSIC_SVC_CONF == 1) && defined (ACE_FILE_EXT_BASED_SVC_CONF)
  testLoadingServiceConfFileExt (argc, argv);
#endif

  ACE_END_TEST;
  return error;
}
