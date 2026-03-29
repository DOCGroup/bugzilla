
//=============================================================================
/**
 *  @file    Bug_4073_Regression_Test.cpp
 *
 *  $Id$
 *
 *    1) schedule single shot timers. Store the timer id, so we can cancel it
 *       we do this in Time_Handle constructor
 *    2) when single shot timer is expired, its Timer_Queue_T frees the slot
 *       *before* making the upcall
 *    3) before handle_timeout completes, it is possible to schedule another
 *       timer which would reuse the slot released in (2), as described in
 *       Bug 4073
 *       We do this in Time_Handle::svc().
 *    4) If another thread tries to cancel the timer using the timerid saved in
 *       (1), it would really cancel the timer scheduled in (3)
 *
 *  In order to expose bug:
 *    - Not all reactors are vulnerable:
 *      On Linux, ACE_Select_Reactor works fine,
 *      but ACE_TP_Reactor or ACE_Dev_Poll_Reactor fail.
 *    - we need to trigger the timer_id reuse algorithm. This test uses
 *      ACE_Timer_Heap_T which is default for Select based reactors and
 *      Dev_Poll_Reactor.
 *  @author milan.cvetkovic@mpathix.com
 */
//=============================================================================


#include "test_config.h"

#include "ace/Task.h"
#include "ace/TP_Reactor.h"
#include "ace/Dev_Poll_Reactor.h"

#include <list>

// trigger timer_id reuse
const int EXTRA_TIMERS = 2;
const int INITIAL_TIMERS = ACE_DEFAULT_TIMERS - EXTRA_TIMERS;

class Time_Handler : public ACE_Task_Base
{
public:
  explicit Time_Handler (ACE_Reactor* r);

  /// Handle the timeout.
  virtual int handle_timeout (const ACE_Time_Value &tv,
                              const void *arg);

  virtual int svc ();

private:
  /// Stores the timer ids of all scheduled timers
  std::list<long> tids_;
  ACE_Thread_Mutex mutex_;
};


Time_Handler::Time_Handler (ACE_Reactor* r)
{
	reactor (r);
	for (int i = 0; i < INITIAL_TIMERS; ++i)
	{
		tids_.push_back (0);
		long* rtid = &tids_.back();
		*rtid = reactor()->schedule_timer(
			this, rtid, ACE_Time_Value (i ? 3000 : 1));
		ACE_DEBUG ((LM_DEBUG,
			    ACE_TEXT("Scheduled timer: %q\n"),
			    *rtid));
	}
	activate();
}

int Time_Handler::svc ()
{
  ACE_Trace t (ACE_TEXT ("Time_Handler::svc"),
               __LINE__,
               ACE_TEXT_CHAR_TO_TCHAR (__FILE__));

  // sleep until 1st handle_timeout expired, but before its handle_timeout
  // completed
  ACE_Time_Value tv_sleep (1, 100000);
  ACE_OS::sleep(tv_sleep);

  long tid = reactor()->schedule_timer (this, 0, ACE_Time_Value(300));
  ACE_DEBUG ((LM_DEBUG,
	      "Time_Handler::svc scheduled timer1 timer_id=%q\n",
	      tid));
  for (int i=0; i < EXTRA_TIMERS; ++i)
    {
      tid = reactor()->schedule_timer (this, 0, ACE_Time_Value(300));
      ACE_DEBUG ((LM_DEBUG,
		  "Time_Handler::svc scheduled extra timer %d timer_id=%q\n",
		  i, tid));
    }

  ACE_Guard<ACE_Thread_Mutex> guard (mutex_);
  // search for timer_id. There should be no match
  std::list<long>::iterator it = std::find (tids_.begin(), tids_.end(), tid);
  if (it != tids_.end())
    {
      ACE_DEBUG ((LM_ERROR,
		  "Newly scheduled timer %q reuses timerid\n",
		  tid));
    }
  else
  {
      ACE_DEBUG ((LM_DEBUG,
		  "OK, newly scheduled timer %q does not reuse timerid\n",
		  tid));
  }
  return 0;
}

int
Time_Handler::handle_timeout (const ACE_Time_Value& currentTime,
			      const void* pv)
{
  ACE_Trace t (ACE_TEXT ("Time_Handler::handle_timeout"),
               __LINE__,
               ACE_TEXT_CHAR_TO_TCHAR (__FILE__));
  const long* rtid = reinterpret_cast<const long*>(pv);
  ACE_DEBUG ((LM_DEBUG,
	      "handle_timeout tid:%q currentTime:%q.%06d\n",
	      *rtid, currentTime.sec(), currentTime.usec()));

  // pretend lengthy operation 800 msec
  ACE_Time_Value sleep_tv (0, 800000);
  ACE_OS::sleep (sleep_tv);

  {
    // scope for guard
    ACE_Guard<ACE_Thread_Mutex> guard (mutex_);

    ACE_DEBUG ((LM_DEBUG,
		"handle_timeout removing timer %q\n",
		*rtid));

    tids_.erase (std::remove (tids_.begin(), tids_.end(), *rtid),
		 tids_.end());
  }
  return 0;
}

int
run_main (int /*argc*/, ACE_TCHAR * /*argv*/[])
{
  ACE_START_TEST (ACE_TEXT ("Bug_4073_Regression_Test"));

  {
    ACE_Trace t (ACE_TEXT ("test_default_ACE_Reactor"),
		 __LINE__,
		 ACE_TEXT_CHAR_TO_TCHAR (__FILE__));
    ACE_Reactor* r = ACE_Reactor::instance();

    Time_Handler* handler = new Time_Handler (r);
    ACE_Event_Handler_var safe_handler(handler);

    ACE_Time_Value tv (10, 0);
    r->handle_events (tv);
  }
  {
    ACE_Trace t (ACE_TEXT ("test_ACE_TP_Reactor"),
		 __LINE__,
		 ACE_TEXT_CHAR_TO_TCHAR (__FILE__));
    ACE_Reactor::instance(new ACE_Reactor (new ACE_TP_Reactor), true);
    ACE_Reactor* r = ACE_Reactor::instance();

    Time_Handler* handler = new Time_Handler (r);
    ACE_Event_Handler_var safe_handler(handler);

    ACE_Time_Value tv (10, 0);
    r->handle_events (tv);
  }
  {
    ACE_Trace t (ACE_TEXT ("test_ACE_Dev_Poll_Reactor"),
		 __LINE__,
		 ACE_TEXT_CHAR_TO_TCHAR (__FILE__));
    ACE_Reactor::instance(new ACE_Reactor (new ACE_Dev_Poll_Reactor), true);
    ACE_Reactor* r = ACE_Reactor::instance();

    Time_Handler* handler = new Time_Handler (r);
    ACE_Event_Handler_var safe_handler(handler);

    ACE_Time_Value tv (10, 0);
    r->handle_events (tv);
  }

  ACE_END_TEST;
  return 0;
}
