
// ============================================================================
//
// = LIBRARY
//    tests
//
// = FILENAME
//    Reactor_Wakeup_Test.cpp
//
// = DESCRIPTION
//    This is a test of schedule_wakeup/cancel_wakeup/remove_handler
//    for the ACE_Select_Reactor.
//
// = AUTHOR
//    Patrick Rabau
//
// ============================================================================

#include "tests/test_config.h"
#include "ace/Pipe.h"
#include "ace/Reactor.h"
#include "ace/Select_Reactor.h"
#include "ace/ACE.h"
#include "ace/Min_Max.h"

//ACE_RCSID(tests, Reactor_Wakeup_Test, "...")

class Event_Handler : public ACE_Event_Handler
{
public:

  Event_Handler (ACE_Reactor &reactor,
                 ACE_HANDLE handle1,
                 ACE_HANDLE handle2);
  ~Event_Handler (void);

  int handle_output (ACE_HANDLE handle);

  int handle_timeout (const ACE_Time_Value &, const void *);

  const ACE_HANDLE handle1_;
  const ACE_HANDLE handle2_;
  int handle_output_iteration_;
};

Event_Handler::Event_Handler (ACE_Reactor &reactor,
                              ACE_HANDLE handle1,
                              ACE_HANDLE handle2)
  : ACE_Event_Handler (&reactor),
    handle1_ (handle1),
    handle2_ (handle2),
    handle_output_iteration_ (0)
{
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Event_Handler ctor: handle1= %d  handle2= %d\n"),
              handle1_, handle2_));
  int result;

  result = this->reactor ()->register_handler (this->handle1_,
                                               this,
                                               NULL_MASK);
  ACE_ASSERT (result == 0);

  result = this->reactor ()->register_handler (this->handle2_,
                                               this,
                                               NULL_MASK);
  ACE_ASSERT (result == 0);

  result = this->reactor ()->schedule_wakeup (this->handle1_,
                                              WRITE_MASK);
  ACE_ASSERT (result != -1);
}

Event_Handler::~Event_Handler (void)
{
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Event_Handler dtor\n")));

  this->reactor ()->cancel_timer (this);
}

int
Event_Handler::handle_output (ACE_HANDLE handle)
{
  int result;

  handle_output_iteration_++;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Event_Handler: handle_output (handle=%d)"
                        " iteration=%d\n"),
              handle, handle_output_iteration_));

  if (handle_output_iteration_ == 1)
    {
      ACE_ASSERT (handle == this->handle1_);

      result = this->reactor ()->cancel_wakeup (this->handle1_,
                                                WRITE_MASK);
      ACE_ASSERT (result != -1);

      result = this->reactor ()->remove_handler (this->handle2_,
                                                 DONT_CALL);
      ACE_ASSERT (result == 0);

      result = this->reactor ()->schedule_timer (this,
                                                 NULL,
                                                 ACE_Time_Value (2));
      ACE_ASSERT (result != -1);

      return 0;
    }
  else if (handle_output_iteration_ >= 3)
    {
      this->reactor ()->end_reactor_event_loop ();
      return -1;
    }

  return 0;
}

int
Event_Handler::handle_timeout (const ACE_Time_Value &, const void *)
{
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Event_Handler: handle_timeout: iteration=%d\n"),
              handle_output_iteration_));

  int result = this->reactor ()->schedule_wakeup (this->handle1_,
                                                  WRITE_MASK);
  ACE_ASSERT (result != -1);

  return 0;
}

void
do_test (ACE_Reactor & reactor,
         ACE_HANDLE handle1,
         ACE_HANDLE handle2)
{
   Event_Handler event_handler (reactor,
                                handle1,
                                handle2);

   reactor.run_reactor_event_loop ();

   ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("reactor loop done: iteration= %d\n"),
            event_handler.handle_output_iteration_));
   ACE_ASSERT (event_handler.handle_output_iteration_ == 3);
}

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Reactor_Wakeup_Test"));

  ACE_Select_Reactor select_reactor;
  ACE_Reactor reactor (&select_reactor,
                       0);

  // Create the pipe *after* the reactor has been initialized, so
  // that the pipe handles are the highest handles.
  ACE_Pipe pipe;
  ACE_HANDLE handles[2];
  int result = pipe.open (handles);
  ACE_ASSERT (result == 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("pipe: handles[0] = %d, handles[1] = %d\n\n"),
              handles[0], handles[1]));

  ACE_HANDLE smaller_handle = ACE_MIN (handles[0], handles[1]);
  ACE_HANDLE larger_handle  = ACE_MAX (handles[0], handles[1]);

  do_test(reactor, larger_handle, smaller_handle);

  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("\n")));
  reactor.reset_reactor_event_loop ();

  do_test(reactor, smaller_handle, larger_handle);

  ACE_END_TEST;

  return 0;
}
