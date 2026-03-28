
#include <ace/Reactor.h>
#include <ace/Pipe.h>
#include <ace/Log_Msg.h>

class MyHandler : public ACE_Event_Handler
{
   public:
      MyHandler(ACE_Pipe & pipe) : pipe_(pipe), i_(0) {}

      virtual ACE_HANDLE get_handle() const
      {
         return pipe_.read_handle();
      }

      virtual int handle_input(ACE_HANDLE)
      {
         char c = '?';
         pipe_.recv(&c, 1);

         ACE_DEBUG((LM_DEBUG, "\n-------- handle_input: %c\n", c));

         return 0;
      }

      virtual int handle_timeout(const ACE_Time_Value &, const void *act)
      {
         if (act == (void *)1)
         {
            ACE_DEBUG((LM_DEBUG, "... sending to pipe and doing 3 notifies\n"));
            pipe_.send("abc", 3);

            reactor()->notify(this);
            reactor()->notify(this);
            reactor()->notify(this);

            ACE_Reactor::instance()->schedule_timer(this, (void *)2,
                                                    ACE_Time_Value(1));
         }
         else if (act == (void *)2)
         {
            ACE_DEBUG((LM_DEBUG, "... stopping the reactor\n"));
            ACE_Reactor::instance()->end_reactor_event_loop();
         }

         return 0;
      }

      virtual int handle_exception(ACE_HANDLE)
      {
         ACE_DEBUG((LM_DEBUG, "\n--- handle_exception\n", i_));

         return 0;
      }

   private:
      ACE_Pipe & pipe_;
      int i_;
};

int reactorEventHook(ACE_Reactor *)
{
   ACE_DEBUG((LM_DEBUG, "AFTER handle_events()\n"));
   return 0;
}

int
main (int, ACE_TCHAR *[])
{
   ACE_Reactor *reactor = ACE_Reactor::instance();

   reactor->max_notify_iterations(1);

   ACE_Pipe pipe;
   pipe.open();

   ACE_Pipe pipe2;
   pipe2.open();

   MyHandler handler(pipe);
   MyHandler handler2(pipe2);

   reactor->register_handler(&handler, ACE_Event_Handler::READ_MASK);
   reactor->register_handler(&handler2, ACE_Event_Handler::READ_MASK);

   reactor->schedule_timer(&handler, (void *)1, ACE_Time_Value(1));
   reactor->schedule_timer(&handler2, (void *)1, ACE_Time_Value(1));

   reactor->run_reactor_event_loop(reactorEventHook);

   reactor->remove_handler(&handler, ACE_Event_Handler::READ_MASK);
}

