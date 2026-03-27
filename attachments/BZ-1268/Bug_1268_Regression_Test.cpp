/**
 * @file
 *
 * $Id$
 *
 * Verify that the notification queue can be used with large numbers
 * of events.
 *
 * Normally the ACE_Reactor uses a pipe to implement the notify()
 * methods.  ACE can be compiled with
 * ACE_HAS_REACTOR_NOTIFICATION_QUEUE, with this configuration flag
 * the Reactor uses a user-space queue to contain the notifications.
 * A single message is sent through the pipe to indicate "pipe not
 * empty."
 *
 * @author Carlos O'Ryan <coryan@atdesk.com>
 *
 */

#include "test_config.h"
#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/Select_Reactor.h"

ACE_RCSID(tests, Bug_1268_Regression_Test, "$Id$")

class Event_Handler : public ACE_Event_Handler
{
public:
  Event_Handler(
      ACE_Reactor * reactor,
      int max_notifications,
      char const * test_name);

  /**
   * @brief Run the test
   */
  void run();

  /**
   * @brief Receive the notifications.
   */
  virtual int handle_exception(ACE_HANDLE);

private:
  /**
   * @brief Implement a single iteration.
   *
   * Each iteration of the test consists of sending multiple
   * notifications simultaneously.
   */
  void send_notifications();

  /**
   * @brief Return true if the test is finished.
   */
  bool done() const;

private:
  /**
   * @brief The maximum number of notifications in any single
   * iteration.
   */
  int max_notifications_;

  /**
   * @brief The name of the test
   */
  char const * test_name_;
  /**
   * @brief Number of notifications received
   */
  int notifications_sent_;
  /**
   * @brief Number of notifications sent
   */
  int notifications_recv_;

  /**
   * @brief Number of notifications sent on each iteration
   */
  int notifications_curr_;
};

int
ACE_TMAIN (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Bug_1268_Regression_Test"));

#if !defined(ACE_HAS_REACTOR_NOTIFICATION_QUEUE)
  ACE_ERROR ((LM_INFO,
                 ACE_TEXT ("Notification queue disabled, "
                     "small test version\n")));
  int max_notifications = 16;
#else
  int max_notifications = 1024 * 1024;
#endif /* ACE_HAS_THREADS */

  {
    ACE_Reactor select_reactor(
        new ACE_Select_Reactor);
    Event_Handler handler(
        &select_reactor, max_notifications, "Select_Reactor");
    handler.run();
  }

  {
    ACE_Reactor tp_reactor(
        new ACE_TP_Reactor);
    Event_Handler handler(
        &tp_reactor, max_notifications, "TP_Reactor");
    handler.run();
  }

  ACE_END_TEST;
  return 0;
}

Event_Handler::
Event_Handler(
    ACE_Reactor * reactor,
    int max_notifications,
    char const * test_name)
  : ACE_Event_Handler(reactor)
  , max_notifications_(max_notifications)
  , test_name_(test_name)
  , notifications_sent_(0)
  , notifications_recv_(0)
  , notifications_curr_(1)
{
}

void Event_Handler::
run()
{
  send_notifications();

  // Run for 30 seconds or until the test is done.
  for(int i = 0; i != 30 && !done(); ++i)
  {
    ACE_Time_Value tv(1,0);
    reactor()->run_reactor_event_loop(tv);
  }
  if(!done())
  {
    ACE_ERROR((LM_ERROR,
                  "Test %s failed due to timeout sent=%d, recv=%d\n",
                  test_name_, notifications_sent_, notifications_recv_));
  }
  else
  {
    ACE_ERROR((LM_INFO,
                  "Test %s passed sent=%d, recv=%d\n",
                  test_name_, notifications_sent_, notifications_recv_));
  }
}

int Event_Handler::
handle_exception(ACE_HANDLE)
{
  ++notifications_recv_;
  if(notifications_recv_ == notifications_sent_)
  {
    if(notifications_curr_ >= max_notifications_)
    {
      return 0;
    }
    send_notifications();
  }
  return 0;
}

void Event_Handler::
send_notifications()
{
  for(int i = 0; i != notifications_curr_; ++i)
  {
    if(reactor()->notify(this) == -1)
    {
      ACE_ERROR((LM_ERROR, "Cannot send notifications in %s test (%d/%d)\n",
                    test_name_, i, notifications_curr_));
      return;
    }
    ++notifications_sent_;
  }
  //  ACE_ERROR((LM_ERROR,
  //                "Started iteration with %d notify() calls in test %s\n",
  //                notifications_curr_, test_name_));
  notifications_curr_ *= 2;
}

bool Event_Handler::
done() const
{
  return (notifications_curr_ >= max_notifications_)
    && (notifications_sent_ == notifications_recv_);
}
