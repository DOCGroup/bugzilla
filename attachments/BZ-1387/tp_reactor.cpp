#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/Svc_Handler.h"
#include "ace/Thread_Manager.h"
#include "ace/INET_Addr.h"
#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Connector.h"
#include "ace/Synch.h"

const char *HTTP_RESPONSE = "HTTP/1.1 200 OK\nServer: Microsoft-IIS/5.0\nDate: Sun, 10 Nov 2002 16:29:46 GMT\nContent-Type: text/plain\nAccept-Ranges: bytes\nLast-Modified: Thu, 13 Dec 2001 12:51:54 GMT\nETag: \"275518f1d483c11:91e\"\nContent-Length: 41\r\n\r\nThis is a test file on the Web Server !!!";

#if defined (ACE_FIX_VERSION)
static void * svc(void *)
#else
static ACE_THR_FUNC_RETURN svc (void *)
#endif
{
  if (ACE_Reactor::instance()->run_reactor_event_loop() == -1)
    ACE_ERROR ((LM_ERROR, "%p\n", "event loop"));
  return 0;
}

class MyHandler: public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
public:
  int open(void *p = 0)
  {
    ACE_DEBUG ((LM_DEBUG, "%T (%t) open\n"));
    this->m_timerID = 0;
    ACE_Reactor::instance()->register_handler(this, READ_MASK);
    return 0;
  }
  int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_DEBUG, "%T (%t) input\n"));
    if (this->m_timerID)
      ACE_Reactor::instance()->cancel_timer(m_timerID);
    ACE_Time_Value delay(2, 0);
    this->m_timerID = ACE_Reactor::instance()->schedule_timer (this,
                                                               NULL,
                                                               delay);
    if (this->m_timerID < 0)
      {
        ACE_ERROR_RETURN ((LM_ERROR, "%x: %p\n", this, "schedule_timer"), -1);
      }

    char buf[1024];
    int retVal = this->peer_.recv(buf, 1024);
    if(retVal <= 0)
      return -1;
    if (this->peer_.send_n(HTTP_RESPONSE, ACE_OS::strlen (HTTP_RESPONSE)) == -1)
      ACE_ERROR_RETURN ((LM_ERROR, "%x: %p\n", this, "send_n"), -1);

    return 0;
  }

  int handle_close(ACE_HANDLE handle = ACE_INVALID_HANDLE,
                   ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK)
  {
    this->destroy();
    ACE_DEBUG ((LM_DEBUG, "(-%x)", this));
    return 0;
  }

  int handle_timeout(const ACE_Time_Value&, const void*)
  {
    ACE_DEBUG ((LM_DEBUG, "%T (%t) timer expired\n"));
#if 0
    // This workaround avoids the problem.
    this->reactor ()->notify (this);
    return 0;
#else
    return -1;
#endif
  }

private:
  int m_timerID;
};


typedef ACE_Acceptor<MyHandler, ACE_SOCK_ACCEPTOR> MyAcceptor;

int main(int argc, char* argv[])
{
  ACE_TP_Reactor *tp_reactor = new ACE_TP_Reactor();
  ACE_Reactor *reactor = new ACE_Reactor (tp_reactor);
  ACE_Reactor::instance(reactor);

  ACE_INET_Addr addr(1234);
  MyAcceptor acc;
  if (acc.open(addr) == -1)
    ACE_ERROR ((LM_ERROR, "%p\n", "accept"));

  ACE_Thread_Manager::instance()->spawn_n(4, svc);

#if 0
  ACE_INET_Addr con ((u_short)1234, INADDR_LOOPBACK);
  //  acc.acceptor ().get_local_addr (con);
  int i;
  ACE_SOCK_Stream sock[200];
  ACE_SOCK_Connector connector;
  for (i = 0; i < 200; )
    {
      if (connector.connect (sock[i], con) == -1)
        {
          ACE_ERROR ((LM_ERROR, "%p\n", "connect"));
          ACE_OS::sleep (1);
        }
      else
        {
          ++i;
          ACE_DEBUG ((LM_DEBUG, "["));
        }
    }
  const char msg[] = "Hi\n";
  char buff[100];
  for (i = 0; i < 200; ++i)
    sock[i].send (msg, 3);
  int closed = 0;
  while (closed < 200)
    {
      for (i = 0; i < 200; ++i)
        {
          if (sock[i].get_handle () != ACE_INVALID_HANDLE)
            if (sock[i].recv (buff, 100) > 0)
              {
                sock[i].close ();
                ++closed;
                ACE_DEBUG ((LM_DEBUG, "]"));
              }
        }
    }
  ACE_Reactor::instance ()->end_reactor_event_loop ();
#endif
  ACE_Thread_Manager::instance()->wait();
  ACE_DEBUG ((LM_DEBUG, "\n"));
  return 0;
}
