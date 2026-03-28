/**
 * @file Bug_2873_Regression_Test.cpp
 *
 * Reproduces the problems reported in bug 2873
 *   http://deuce.doc.wustl.edu/bugzilla/show_bug.cgi?id=2873
 *
 * @author Like Ma <likemartinma@gmail.com>
 */

#include "test_config.h"
#include "ace/Process_Manager.h"
#include "ace/Reactor.h"
#include "ace/Log_Msg.h"

ACE_RCSID(tests,
          Bug_2873_Regression_Test, "$Id: Bug_2873_Regression_Test.cpp 2012 2008-10-01 15:28:28Z like $")

/*!
 * @class Exit_Handler
 *
 * @brief A simple process exit handler for the test.
 */
class Exit_Handler : public ACE_Event_Handler
{
public:
  ~Exit_Handler ();

  virtual int handle_exit (ACE_Process*);

  virtual int handle_close (ACE_HANDLE, ACE_Reactor_Mask);
};

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Bug_2873_Regression_Test"));

  ACE_Process_Manager::instance ()->open (ACE_Process_Manager::DEFAULT_SIZE,
                                          ACE_Reactor::instance ());
  Exit_Handler* exit_handler;
  ACE_NEW_RETURN (exit_handler, Exit_Handler, 1);

  ACE_Process_Options opts;
  opts.command_line (ACE_TEXT ("notepad"));
  pid_t pid = ACE_Process_Manager::instance ()->spawn (opts, exit_handler);
  if (pid < 0) {
    delete exit_handler;
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("%p\n"),
                       ACE_TEXT ("Unable to spawn 'notepad'")),
                       1);
  }

  ACE_Reactor::run_event_loop (ACE_Time_Value (5));

  ACE_END_TEST;

  return 0;
}

Exit_Handler::~Exit_Handler ()
{
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Exit_Handler::~Exit_Handler\n")));
}

int Exit_Handler::handle_exit (ACE_Process* proc)
{
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Exit_Handler::handle_exit\n")));
  return 0;
}

int Exit_Handler::handle_close (ACE_HANDLE, ACE_Reactor_Mask)
{
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Exit_Handler::handle_close\n")));
  delete this;
  return 0;
}

