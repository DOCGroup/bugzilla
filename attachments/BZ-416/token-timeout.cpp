#include "ace/Task.h"
#include "ace/Token.h"

int sleep_time = 6;
int acquire_time = 3;
int signal_after_sleep = 1;

class Waiter : public ACE_Task_Base
{
public:

  Waiter (ACE_Condition_Thread_Mutex &condition,
          ACE_Thread_Mutex &mutex,
          ACE_Event &event)
    : condition_ (condition),
      mutex_ (mutex),
      event_ (event)
    {
    }

  int svc (void)
    {
      int result = 0;

      ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, this->mutex_, -1);

      ACE_DEBUG ((LM_DEBUG, "(%t) Waiter: got mutex; signaling event and waiting on condition @ %T\n"));

      result = this->event_.signal ();
      ACE_ASSERT (result != -1);

      ACE_Time_Value acquire_timeout (acquire_time);
      acquire_timeout += ACE_OS::gettimeofday ();

      result = this->condition_.wait (&acquire_timeout);
      if (result == -1)
        {
          if (errno == ETIME)
            {
              ACE_DEBUG ((LM_DEBUG, "(%t) Waiter: condition timed out @ %T\n"));
            }
          else
            {
              ACE_ASSERT (0);
            }
        }
      else
        {
          ACE_DEBUG ((LM_DEBUG, "(%t) Waiter: got condition; no timeouts @ %T\n"));
        }
      return -1;
    }

  ACE_Condition_Thread_Mutex &condition_;
  ACE_Thread_Mutex &mutex_;
  ACE_Event &event_;
};

class Acquirer : public ACE_Task_Base
{
public:

  Acquirer (ACE_Condition_Thread_Mutex &condition,
            ACE_Thread_Mutex &mutex,
            ACE_Event &event)
    : condition_ (condition),
      mutex_ (mutex),
      event_ (event)
    {
    }

  int svc (void)
    {
      int result = 0;

      ACE_DEBUG ((LM_DEBUG, "(%t) Waiter: waiting for event @ %T\n"));

      result = this->event_.wait ();
      ACE_ASSERT (result != -1);

      ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, this->mutex_, -1);

      ACE_DEBUG ((LM_DEBUG, "(%t) Acquirer: got mutex @ %T\n"));

      if (!signal_after_sleep)
        {
          ACE_DEBUG ((LM_DEBUG, "(%t) Acquirer: signaling condition @ %T\n"));

          result = this->condition_.signal ();
          ACE_ASSERT (result != -1);
        }

      ACE_OS::sleep (sleep_time);

      if (signal_after_sleep)
        {
          ACE_DEBUG ((LM_DEBUG, "(%t) Acquirer: signaling condition @ %T\n"));

          result = this->condition_.signal ();
          ACE_ASSERT (result != -1);
        }

      ACE_DEBUG ((LM_DEBUG, "(%t) Acquirer: releasing mutex @ %T\n"));

      return 0;
    }

  ACE_Condition_Thread_Mutex &condition_;
  ACE_Thread_Mutex &mutex_;
  ACE_Event &event_;
};

int
main (int argc, char **argv)
{
  int result = 0;

  ACE_Thread_Mutex mutex;
  ACE_Condition_Thread_Mutex condition (mutex);
  ACE_Event event;

  Waiter waiter (condition,
                 mutex,
                 event);
  Acquirer acquirer (condition,
                     mutex,
                     event);

  result = waiter.activate (THR_NEW_LWP | THR_JOINABLE);
  ACE_ASSERT (result == 0);

  result = acquirer.activate (THR_NEW_LWP | THR_JOINABLE);
  ACE_ASSERT (result == 0);

  result = ACE_Thread_Manager::instance ()->wait ();
  ACE_ASSERT (result == 0);

  return 0;
}
