/**
 * @file Reactor_Notification_Queue_Test2.cpp
 *
 * $Id$
 *
 * Try to break the notification queue implementation using 2 threads
 * and specific timing for inserting/removing the notification queue.
 *
 * @author peter.leipold@eqonline.fi
 *
 */

#include "test_config.h"
#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/Select_Reactor.h"
#include "ace/Dev_Poll_Reactor.h"
#include "ace/Task.h"
#include "ace/Condition_T.h"

ACE_RCSID(tests,
          Reactor_Notification_Queue_Test2, "$Id$")

class Task : public ACE_Task_Base
{
public:
  Task(ACE_Reactor * reactor,
       int max_notifications,
       char const *test_name);

  /// Run the test
  void run(void);

  /// Thread which will send us notifications.
  virtual int svc (void);

  /// Receive the notifications.
  virtual int handle_exception(ACE_HANDLE);

private:
  enum Phase {
    ph_send,        //!< We are sending notifications
    ph_recv,        //!< We are receiving the notifications
    ph_stop         //!< We want to stop
  };

  /// Change our phase, and then wait until phase is again the same what it was.
  void chg_phase(Phase new_phase);

private:
  /// The maximum number of notifications
  int max_notifications_;

  /// The number of notifications was actually sent
  int num_notifications_;

  /// The name of the test
  char const * test_name_;

  /// Our phase, see the enum
  Phase phase_;

  /// Condition variable, used when changing phase
  ACE_Thread_Mutex mtx_;
  ACE_Thread_Condition<ACE_Thread_Mutex> cond_;
};

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Reactor_Notification_Queue_Test2"));

#if !defined(ACE_HAS_REACTOR_NOTIFICATION_QUEUE)
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("Notification queue disabled, ")
              ACE_TEXT ("small test version, ")
              ACE_TEXT ("which is of no practical use\n")));

  int max_notifications = 16;
#else
  int max_notifications = 10 * 1024;
#endif /* ACE_HAS_THREADS */

  {
    ACE_Reactor select_reactor (
        new ACE_Select_Reactor,
        1);

    Task task(&select_reactor,
              max_notifications,
              "Select_Reactor");

    task.run ();
  }

  {
    ACE_Reactor tp_reactor (new ACE_TP_Reactor,
                            1);

    Task task(&tp_reactor,
              max_notifications,
              "TP_Reactor");

    task.run();
  }

#if 0
  /** For some reason Dev_Poll_Reactor don't return from notify regardless of the
      timeout and that makes the test freezing, so this is now commented out.
    */

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)

  {
    ACE_Reactor dev_poll_reactor (new ACE_Dev_Poll_Reactor,
                            1);

    Task task(&dev_poll_reactor,
              max_notifications,
              "Dev_Poll_Reactor");

    task.run();
  }
#endif  /* (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL) */
#endif  /* 0 */

  ACE_END_TEST;
  return 0;
}

Task::Task(
    ACE_Reactor * r,
    int max_notifications,
    char const *test_name)
    :
    max_notifications_(max_notifications),
    num_notifications_(0),
    test_name_(test_name),
    phase_(ph_send),
    cond_(mtx_)
{
    reactor(r);
}

void Task::run (void)
{
    // launch the thread which will send us notifications
    activate();

    // wait for the notifications
    reactor()->run_reactor_event_loop();

    // check if test was succeed
    if (num_notifications_ < max_notifications_)
        ACE_ERROR((LM_INFO,
                    ACE_TEXT ("%C test: Test failed, sent only %d notifications.\n"),
                    test_name_, num_notifications_));
    else
        ACE_ERROR((LM_INFO,
                    ACE_TEXT ("%C test: Test succeed, sent all (%d) notifications.\n"),
                    test_name_, num_notifications_));
}

int Task::svc (void)
{
    ACE_ERROR((LM_INFO,
                ACE_TEXT ("%C test: thread started.\n"),
                test_name_));

    // try to send max_notifications_ noitifications
    for (num_notifications_ = 0; num_notifications_ < max_notifications_; ) {

        // give 1 seconds for the notify to finish
        ACE_Time_Value to(1);

        // Note: sending 2 notifications here is a requirement for the bug to raise
        // In reality the sender is a fast-sender, and sends not only 2, but more
        if (reactor()->notify (this, ACE_Event_Handler::EXCEPT_MASK, &to) == -1 ||
            reactor()->notify (this, ACE_Event_Handler::EXCEPT_MASK, &to) == -1) {

            ACE_ERROR((LM_ERROR,
                        ACE_TEXT ("Cannot send more notifications in %C test (%d/%d)\n"),
                        test_name_, num_notifications_, max_notifications_));
            break;
        }

        // we have successfully sent 2 notifications
        num_notifications_ += 2;

        // let's now wait until the reactor thread process the notifications.
        // we continue sending only when the reactor thread will wake up us in
        // the handle_exception func.
        chg_phase(ph_recv);
    }

    // This makes sure that the reactor thread will wake up and
    // eat up the notification pipe, even if it's full
    chg_phase(ph_stop);

    reactor()->end_reactor_event_loop();

    ACE_ERROR((LM_INFO,
                ACE_TEXT ("%C test: thread stopped.\n"),
                test_name_));

    return 0;
}

int Task::handle_exception (ACE_HANDLE)
{
    // We got notifications (actually 2 notifications) from the other thread.
    // Note, that purging the notification queue here is a must to raise the bug
    reactor()->purge_pending_notifications(this);

    // We wait until the other thread sends us again 2 notifications
    // This wait is needed to keep us in the dispatch loop, and bug will arise only then.
    // In reality, a time-consuming task may be performed here.
    chg_phase(ph_send);

    return 0;
}

void Task::chg_phase(Phase new_phase)
{
    ACE_Guard<ACE_Thread_Mutex> guard(mtx_);

    // Change the phase only if we are not stopping
    if (phase_ != ph_stop) {
        phase_ = new_phase;
        cond_.signal();
    }

    // Waits until we are again in the same phase as we were..
    while (phase_ != ph_stop && phase_ == new_phase)
        if (cond_.wait() == -1) {
            ACE_ERROR((LM_INFO,
                        ACE_TEXT ("condition variable failure\n")));
            break;
    }
}
