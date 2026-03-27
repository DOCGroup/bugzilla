//
// $Id$
//

#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/Event_Handler.h"

class My_Event_Handler : public ACE_Event_Handler
{
public:
  My_Event_Handler();

  virtual int handle_timeout (const ACE_Time_Value& tv,
                              const void* arg);

  int counter;
};

int
main (int, char*[])
{
  ACE_Reactor_Impl *impl = new ACE_TP_Reactor;
  ACE_Reactor *reactor = new ACE_Reactor (impl);

  ACE_Time_Value period(0, 10000);

  My_Event_Handler eh;

  if (reactor->schedule_timer(&eh, 0, period, period) == -1)
    return 1;

  ACE_Time_Value total_time(10, 0);
  while (total_time != ACE_Time_Value::zero)
    {
      if (reactor->handle_events (total_time) == -1)
        return 1;
    }

  ACE_DEBUG ((LM_DEBUG, "Count = %d (expected ~1000)\n", eh.counter));

  return 0;
}

My_Event_Handler::My_Event_Handler()
  : counter(0)
{
}

int
My_Event_Handler::handle_timeout(const ACE_Time_Value&,
                                 const void*)
{
  this->counter++;
  return 0;
}
