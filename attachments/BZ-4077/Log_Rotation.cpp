#include "test_config.h"
#include <ace/Task.h>
#include <ace/Logging_Strategy.h>
#include <ace/ARGV.h>
#include <ace/Reactor.h>

namespace {

class Logger_Task : public ACE_Task_Base
{
public:
  Logger_Task (int maxMsgs, ACE_Reactor* r)
    : _maxMsgs (maxMsgs)
  {
    this->reactor(r);
  }

  int svc ()
  {
    for (int i=0; i<_maxMsgs; ++i)
      {
	ACE_DEBUG ((LM_DEBUG,
		    "%T Log_Rotation message %d\n", i));
      }
    if (-1 == reactor()->end_reactor_event_loop())
      {
	ACE_ERROR ((LM_ERROR,
		    "Error ending reactor loop\n"));
      }
    return 0;
  }

private:
  const int _maxMsgs;
};

} // anonymous namespace

int run_main (int , ACE_TCHAR*[])
{
  ACE_START_TEST (ACE_TEXT ("Log_Rotation_Test"));

  ACE_Reactor* r = ACE_Reactor::instance();

  ACE_Logging_Strategy log_stg;
  ACE_ARGV_T<ACE_TCHAR> params (
    "-f OSTREAM"
    " -s Log_Rotation_Test.log"
    " -p TRACE"
    " -N 2"
    " -m 1000"
    " -i 1");

  if (log_stg.init (params.argc(), params.argv()))
    {
      ACE_ERROR_RETURN ((LM_ERROR,
			 ACE_TEXT ("Failed to initialize ACE_Logging_Strategy\n")),
			1);
    }

  // Additional timer at 1 us to poll the file length more often
  if (-1 == r->schedule_timer (
			       &log_stg, 0, ACE_Time_Value(0), ACE_Time_Value(0, 1)))
    {
      ACE_ERROR_RETURN ((LM_ERROR,
			 ACE_TEXT ("Failed to initialize ACE_Logging_Strategy timer\n")),
			1);
    }

  Logger_Task ltask (100000, r);
  if (-1 == ltask.activate ())
    {
      ACE_ERROR_RETURN ((LM_ERROR,
			 ACE_TEXT ("Failed to initialize logger thread\n")),
			1);
    }

  r->run_reactor_event_loop();

  // Wait for the thread to exit before we exit.
  ACE_Thread_Manager::instance ()->wait ();

  // This is necessary for ACE_Logging_Strategy to unregister its timers from reactor
  log_stg.fini ();

  ACE_END_TEST;
  return 0;
}
