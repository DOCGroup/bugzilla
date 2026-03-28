/**
 * @file Bug_2609_Regression_Test.cpp
 *
 * $Id: Bug_2609_Regression_Test.cpp 55322 2004-01-10 21:20:20Z shuston $
 *
 * Reproduces the problems reported in bug 2609:
 *   http://deuce.doc.wustl.edu/bugzilla/show_bug.cgi?id=2609
 *
 * @author Carlos O'Ryan <coryan@atdesk.com>
 */

#include "test_config.h"

ACE_RCSID (tests,
           Bug_2609_Regression_Test,
           "$Id: Bug_1576_Regression_Test.cpp 55322 2004-01-10 21:20:20Z shuston $")

#include "ace/Svc_Handler.h"
#include "ace/Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/INET_Addr.h"
#include "ace/Trace.h"

#define INVOKE_SVC_HANDLER_HANDLE_CLOSE

// Define to simulate failed accept_svc_handler()
#define OPEN_FAILS
// Define to invoke remove_reference() as in
// $ACE_ROOT/tests/Thread_Pool_Reactor_Resume_Test.cpp
#define REMOVE_REF_IN_OPEN

# define TRACE(X) ACE_Trace ____ (ACE_LIB_TEXT (X), __LINE__, ACE_LIB_TEXT (__FILE__))

class My_Svc_Handler : public ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH>
{
public:
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH> super;
  My_Svc_Handler()
  {
    TRACE ("My_Svc_Handler:My_Svc_Handler");
    reference_counting_policy().value(
      Reference_Counting_Policy::ENABLED);
  }

  ~My_Svc_Handler()
  {
    TRACE ("My_Svc_Handler::~My_Svc_Handler");
  }

  int open (void* pv)
  {
    TRACE ("open");
#ifdef OPEN_FAILS
    return -1;
#endif
    int result = super::open (pv);
    if (result < 0)
    {
      return -1;
    }
#ifdef REMOVE_REF_IN_OPEN
    remove_reference();
#endif
    return 0;
  }
  int handle_close (ACE_HANDLE fd, ACE_Reactor_Mask mask)
  {
    TRACE ("handle_close");
#ifdef INVOKE_SVC_HANDLER_HANDLE_CLOSE
    super::handle_close (fd, mask);
#endif
    return 0;
  }
};

typedef ACE_Acceptor<My_Svc_Handler, ACE_SOCK_ACCEPTOR> My_Acceptor;

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Bug_2609_Regression_Test"));

  My_Acceptor acceptor (ACE_INET_Addr(9000));

  ACE_Reactor::instance()->run_reactor_event_loop();

  ACE_END_TEST;

  return 0;
}
