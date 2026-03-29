#include <ace/Get_Opt.h>
#include <ace/Reactor.h>
#include <ace/SString.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>

class MyHandler : public ACE_Event_Handler
{
  public:
     virtual int handle_input(ACE_HANDLE fd)
     {
        // Extra delay, so that TCP flow control kicks in more quickly
        ACE_OS::sleep(ACE_Time_Value(0, 10000));

        ssize_t n = stream_.recv(buffer_ , sizeof(buffer_));

        ACE_DEBUG((LM_DEBUG, "handle_input(HANDLE = %d): n = %d\n",
                   (int)(long)fd, (int)n));

        if (n == 0)
        {
           ACE_DEBUG((LM_DEBUG, "Connection closed\n"));
           return -1;
        }
        else if (n < 0)
        {
           ACE_DEBUG((LM_DEBUG, "Connection error: %m\n"));
           return -1;
        }

        return 0;
     }

     virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
     {
        ACE_DEBUG((LM_DEBUG, "Closing socket\n"));
        stream_.close();
        return 0;
     }

     virtual ACE_HANDLE get_handle (void) const
     {
        return stream_.get_handle();
     }

     ACE_SOCK_Stream & getStream() { return stream_; }

  private:
     ACE_SOCK_Stream stream_;
     char buffer_[20000];
};

static void usage(const char *progname)
{
  ACE_DEBUG((LM_DEBUG, "usage: %s -a <address:port>\n", progname));
}

int main(int argc, char **argv)
{
  ACE_Get_Opt getopt(argc, argv, "a:");
  int c;
  ACE_CString address;

  while ((c = getopt()) != EOF)
  {
     switch(c)
     {
        case 'a':
           address = getopt.opt_arg();
           break;

        default:
           usage(argv[0]);
           return 1;
     }
  }

  if (address.is_empty())
  {
     usage(argv[0]);
     return 1;
  }

  ACE_INET_Addr server_addr(address.c_str());
  ACE_SOCK_Connector con;

  MyHandler handler;

  ACE_DEBUG((LM_DEBUG, "Connecting to %s\n", address.c_str()));

  ACE_Time_Value tv(1);

  if (con.connect(handler.getStream(), server_addr, &tv) < 0)
  {
     ACE_DEBUG((LM_DEBUG, "Failed to connect: %m\n"));
     return 1;
  }

  ACE_Reactor *reactor = ACE_Reactor::instance();


  if (reactor->register_handler(&handler, ACE_Event_Handler::READ_MASK) < 0)
  {
     ACE_DEBUG((LM_DEBUG, "Failure to register handler: %m\n"));
     return 0;
  }

  //reactor->run_reactor_event_loop();

  while (1)
  {
     ACE_Time_Value tv(2);
     int rc = reactor->handle_events(tv);
     ACE_DEBUG((LM_DEBUG, "-- reactor handle_events(): rc = %d\n", rc));
  }
}
