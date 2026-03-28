
// ============================================================================
//
// = LIBRARY
//    tests
//
// = FILENAME
//    Timer_Small_Interval_Test.cpp
//
// = DESCRIPTION
//    A test to ensure the timer cancellation works correctly.
//
// = AUTHOR
//    Irfan Pyarali <irfan@oomworks.com>
//
// ============================================================================

#include "ace/OS_NS_unistd.h"
#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/Task.h"

#include <iostream>

class Event_Handler : public ACE_Event_Handler
{
public:

  Event_Handler ();

  int handle_timeout (const ACE_Time_Value &current_time,
                      const void *act);

  int handle_close (ACE_HANDLE handle,
                    ACE_Reactor_Mask close_mask);

};

Event_Handler::Event_Handler ()
{
}

int
Event_Handler::handle_timeout (const ACE_Time_Value &,
                               const void *)
{
  return 0;
}

int
Event_Handler::handle_close (ACE_HANDLE,
                             ACE_Reactor_Mask)
{
  delete this;
  return 0;
}


class Sleep_Event_Handler : public ACE_Event_Handler
{
public:

  Sleep_Event_Handler ();

  int handle_timeout (const ACE_Time_Value &current_time,
                      const void *act);

  int handle_close (ACE_HANDLE handle,
                    ACE_Reactor_Mask close_mask);

};

Sleep_Event_Handler::Sleep_Event_Handler ()
{
}

int
Sleep_Event_Handler::handle_timeout (const ACE_Time_Value &,
                               const void *)
{
  std::cerr << "hello" << std::endl;
  ACE_Time_Value large(1);
  ACE_OS::sleep (large);
  return 0;
}

int
Sleep_Event_Handler::handle_close (ACE_HANDLE,
                             ACE_Reactor_Mask)
{
  return 0;
}


int
main ()
{

  ACE_Reactor reactor (new ACE_TP_Reactor, 1);

  Sleep_Event_Handler handler;

  // Scheduler a timer to kick things off.
  ACE_Time_Value small(0,1);
  reactor.schedule_timer (&handler,  0, small, small);

  for (size_t i = 0; i < 32; ++i)
  {
    reactor.schedule_timer (new Event_Handler,  0, small, small);
  }
  

  // Run the event loop for a while.
  ACE_Time_Value timeout (400);
  reactor.run_reactor_event_loop (timeout);

  return 0;
}

