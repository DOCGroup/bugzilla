//Search for : REPRODUCE_BUG

//=============================================================================
/**
 *  @file    Dev_Poll_Reactor_Test.cpp
 *
 *  $Id: Dev_Poll_Reactor_Test.cpp 80826 2008-03-04 14:51:23Z wotte $
 *
 *  This test verifies that the Dev_Poll_Reactor is functioning
 *  properly, and demonstrates how "speculative reads" can be
 *  performed.  "Speculative reads" are necessary when using event
 *  demultiplexing mechanisms that use a "state change" interface.
 *  Similarly, "speculative writes" may be necessary, i.e. keep
 *  writing until the connection is flow controlled.  An example of a
 *  demuxing mechanism with such an interface is Linux's `/dev/epoll'
 *  character device.  Mechansims with state change interfaces are
 *  also said to be "edge triggered," versus "level triggered"
 *  mechanisms such as select().
 *
 *  @author Ossama Othman <ossama@dre.vanderbilt.edu>  */ //=============================================================================

#include "test_config.h"

ACE_RCSID (tests,
           Dev_Poll_Reactor_Test,
           "$Id: Dev_Poll_Reactor_Test.cpp 80826 2008-03-04 14:51:23Z wotte $")


#if defined (ACE_HAS_DEV_POLL) || defined (ACE_HAS_EVENT_POLL)

#include "ace/OS_NS_signal.h"
#include "ace/Reactor.h"
#include "ace/Dev_Poll_Reactor.h"

#include "ace/Acceptor.h"
#include "ace/Connector.h"

#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"

#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_netdb.h"


typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> SVC_HANDLER;

// ----------------------------------------------------

class Client : public SVC_HANDLER
{
public:

  Client (void);

  //FUZZ: disable check_for_lack_ACE_OS
  virtual int open (void * = 0);
  //FUZZ: enable check_for_lack_ACE_OS

  virtual int handle_output (ACE_HANDLE handle);

  virtual int handle_timeout (const ACE_Time_Value &current_time,
                              const void *act);

  virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask mask);

private:

  unsigned int call_count_;

};


class Server : public SVC_HANDLER
{
public:

  Server (void);

  virtual int handle_input (ACE_HANDLE handle);

  virtual int handle_timeout (const ACE_Time_Value &current_time,
                              const void *act);

  virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask mask);

private:

  unsigned int call_count_;

};

// ----------------------------------------------------

Client::Client (void)
  : call_count_ (0)
{
}

int
Client::open (void *)
{
  //  ACE_ASSERT (this->reactor () != 0);

  if (this->reactor ()
      && this->reactor ()->register_handler (
           this,
           ACE_Event_Handler::WRITE_MASK) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%t) %p\n"),
                       ACE_TEXT ("unable to register client handler")),
                      -1);

  return 0;
}

int
Client::handle_output (ACE_HANDLE)
{
  for (int i = 1; i <= 5; ++i)
    {
      char buffer[BUFSIZ] = { 0 };

      ACE_OS::sprintf (buffer, "test message %d.\n", i);

      ssize_t bytes_sent =
        this->peer ().send (buffer, ACE_OS::strlen (buffer));

      if (bytes_sent == -1)
        {
          if (errno == EWOULDBLOCK)
            return 0;  // Flow control kicked in.
          else if (errno == EPIPE || errno == ECONNRESET)
            {
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("(%t) Client::handle_output; server ")
                          ACE_TEXT ("closed handle %d\n"),
                          this->peer ().get_handle ()));
              return -1;
            }
          else
            ACE_ERROR_RETURN ((LM_ERROR,
                               ACE_TEXT ("(%t) %p\n"),
                               ACE_TEXT ("Client::handle_output")),
                              -1);
        }
      else if (bytes_sent == 0)
        return -1;
      else
        ACE_DEBUG ((LM_INFO, ACE_TEXT ("(%t) Sent %s"), buffer));
    }

  return 0;
}

int
Client::handle_timeout (const ACE_Time_Value &, const void *) {
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("(%t) Expected client timeout occured at: %T\n")));

  this->call_count_++;

  int status = this->handle_output (this->get_handle ());
  if (status == -1 || this->call_count_ > 10)
    {
      if (this->reactor ()->end_reactor_event_loop () == 0)
        ACE_DEBUG ((LM_INFO,
                    ACE_TEXT ("(%t) Successful client reactor shutdown.\n")));
      else
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("(%t) %p\n"),
                    ACE_TEXT ("Failed client reactor shutdown")));

      // Force this service handler to be closed in either case.
      return -1;
    }

  return 0;
}

int
Client::handle_close (ACE_HANDLE handle,
                      ACE_Reactor_Mask mask) {
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("(%t) Client Svc_Handler closed ")
              ACE_TEXT ("handle <%d> with reactor mask <0x%x>.\n"),
              handle,
              mask));

  return 0;
}

// ----------------------------------------------------

Server::Server (void)
  : call_count_ (0)
{
}

int
Server::handle_input (ACE_HANDLE /* handle */) {
  char buffer[BUFSIZ+1] = { 0 };    // Insure a trailing nul
  ssize_t bytes_read = 0;

  char * const begin = buffer;
  char * const end   = buffer + BUFSIZ;

printf("~~~~> handle_input: %d\n", call_count_);

  for (char * buf = begin; buf < end; buf += bytes_read)
    {
      // Keep reading until it is no longer possible to do so.
      //
      // This is done since the underlying event demultiplexing
      // mechanism may have a "state change" interface (as opposed to
      // "state monitoring"), in which case a "speculative" read is
      // done.
      bytes_read = this->peer ().recv (buf, end - buf);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("****** bytes_read = %d\n"),
                  bytes_read));

      if (bytes_read == -1)
        {
          if (errno == EWOULDBLOCK)
            {

//               ACE_HEX_DUMP ((LM_DEBUG,
//                              buf,
//                              80,
//                              "BUFFER CONTENTS"));
              if (buf == buffer)
                return 0;
              else
                break;
            }
          else
            ACE_ERROR_RETURN ((LM_ERROR,
                               ACE_TEXT ("(%t) %p\n"),
                               ACE_TEXT ("Server::handle_input")),
                              -1);
        }
      else if (bytes_read == 0)
        return -1;
    }

  ACE_DEBUG ((LM_INFO, ACE_TEXT ("(%t) Message received: %s\n"), buffer));

  return 0;
}

int
Server::handle_timeout (const ACE_Time_Value &,
                        const void *)
{
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("(%t) Expected server timeout occured at: %T\n")));

//   if (this->call_count_ == 0
//       && this->handle_input (this->get_handle ()) != 0
//       && errno != EWOULDBLOCK)
//     return -1;

//   ACE_DEBUG ((LM_INFO,
//               "SERVER HANDLE = %d\n",
//               this->get_handle ()));


  this->call_count_++;
printf("~~~~> handle_timeout: %d\n", call_count_);
/*
  if (this->call_count_ > 10)
    {
      if (this->reactor ()->end_reactor_event_loop () == 0)
        ACE_DEBUG ((LM_INFO,
                    ACE_TEXT ("(%t) Successful server reactor shutdown.\n")));
      else
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("(%t) %p\n"),
                    ACE_TEXT ("Failed server reactor shutdown")));

      // Force this service handler to be closed in either case.
      return -1;
    }
*/
  return 0;
}

int
Server::handle_close (ACE_HANDLE handle,
                      ACE_Reactor_Mask mask) { printf("~~~~> handle_close: %d\n", call_count_);
  if (this->call_count_ > 4)
    {
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("(%t) Server Svc_Handler closing ")
                  ACE_TEXT ("handle <%d,%d> with reactor mask <0x%x>.\n"),
                  handle,
                  this->get_handle (),
                  mask));

      return this->peer ().close ();
    }

  return 0;
}

// ----------------------------------------------------

typedef ACE_Acceptor<Server, ACE_SOCK_ACCEPTOR>   ACCEPTOR;
typedef ACE_Connector<Client, ACE_SOCK_CONNECTOR> CONNECTOR;

// ----------------------------------------------------

class TestAcceptor : public ACCEPTOR
{
public:

  virtual int accept_svc_handler (Server * handler)
  {
printf("~~~~> accept_svc_handler\n");
    int result = this->ACCEPTOR::accept_svc_handler (handler);

    if (result != 0)
      {
        if (errno != EWOULDBLOCK)
          ACE_ERROR ((LM_ERROR,
                      ACE_TEXT ("(%t) %p\n"),
                      ACE_TEXT ("Unable to accept connection")));

        return result;
      }

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("(%t) Accepted connection.  ")
                ACE_TEXT ("Stream handle: <%d>\n"),
                handler->get_handle ())); printf("~~~~> Accepted connection: %d\n", handler->get_handle ());

//     if (handler->handle_input (handler->get_handle ()) == -1
//         && errno != EWOULDBLOCK)
//       return -1;

#if 0
    ACE_Time_Value delay (2, 0);
    ACE_Time_Value restart (2, 0);
    if (handler->reactor ()->schedule_timer (handler,
                                             0,
                                             delay,
                                             restart) == -1)
      {
        ACE_ERROR_RETURN ((LM_ERROR,
                           ACE_TEXT ("(%t) %p\n"),
                           ACE_TEXT ("Unable to schedule server side ")
                           ACE_TEXT ("timer in ACE_Dev_Poll_Reactor")),
                          -1);
      }
#endif  /* 0 */

    return result;
  }

};

// ----------------------------------------------------

class TestConnector : public CONNECTOR
{
public:

  virtual int connect_svc_handler (
    CONNECTOR::handler_type *& handler,
    const CONNECTOR::addr_type &remote_addr,
    ACE_Time_Value *timeout,
    const CONNECTOR::addr_type &local_addr,
    int reuse_addr,
    int flags,
    int perms)
  {
    const int result = this->CONNECTOR::connect_svc_handler (handler,
                                                             remote_addr,
                                                             timeout,
                                                             local_addr,
                                                             reuse_addr,
                                                             flags,
                                                             perms);

    if (result != 0)
      return result;

    ACE_TCHAR hostname[MAXHOSTNAMELEN];
    if (remote_addr.get_host_name (hostname,
                                   sizeof (hostname)) != 0)
      {
        ACE_ERROR_RETURN ((LM_ERROR,
                           ACE_TEXT ("(%t) %p\n"),
                           ACE_TEXT ("Unable to retrieve hostname")),
                          -1);
      }

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("(%t) Connected to <%s:%d>.\n"),
                hostname,
                (int) remote_addr.get_port_number ()));

#if 0
    ACE_Time_Value delay (4, 0);
    ACE_Time_Value restart (3, 0);
    if (handler->reactor ()->schedule_timer (handler,
                                             0,
                                             delay,
                                             restart) == -1)
      {
        ACE_ERROR_RETURN ((LM_ERROR,
                           ACE_TEXT ("(%t) %p\n"),
                           ACE_TEXT ("Unable to schedule client side ")
                           ACE_TEXT ("timer in ACE_Dev_Poll_Reactor")),
                          -1);
      }
#endif  /* 0 */

    return result;
  }

  virtual int connect_svc_handler (
    CONNECTOR::handler_type *& handler,
    CONNECTOR::handler_type *& sh_copy,
    const CONNECTOR::addr_type &remote_addr,
    ACE_Time_Value *timeout,
    const CONNECTOR::addr_type &local_addr,
    int reuse_addr,
    int flags,
    int perms) {
    sh_copy = handler;
    return this->connect_svc_handler (handler, remote_addr, timeout,
                                      local_addr, reuse_addr, flags,
                                      perms);
  }
};

// ----------------------------------------------------

ACE_THR_FUNC_RETURN
server_worker (void *p)
{
  const unsigned short port = *(static_cast<unsigned short *> (p));

  ACE_INET_Addr addr;

  if (addr.set (port, INADDR_LOOPBACK) != 0)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("(%t) %p\n"),
                  ACE_TEXT ("server_worker - ACE_INET_Addr::set")));

      return (void *) -1;
    }

  ACE_Dev_Poll_Reactor dp_reactor;
  dp_reactor.restart (1);     // Restart on EINTR
  ACE_Reactor reactor (&dp_reactor);

  TestAcceptor server;

  int flags = 0;
  ACE_SET_BITS (flags, ACE_NONBLOCK);  // Enable non-blocking in the
                                       // Svc_Handlers.
printf("\n~~~~> Opening listening port: %d\n", port);
  if (server.open (addr, &reactor, flags) != 0)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("(%t) %p\n"),
                  ACE_TEXT ("Unable to open server service handler")));

      return (void *) -1;
    }

printf("~~~~> Calling run_reactor_event_loop ()\n");
  if (reactor.run_reactor_event_loop () != 0)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("(%t) %p\n"),
                  ACE_TEXT ("Error when running server ")
                  ACE_TEXT ("reactor event loop"))); printf("~~~~> Reactor event loop finished with w/ error!\n");

      return (void *) -1;
    }
printf("~~~~> Reactor event loop finished\n");

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("(%t) Reactor event loop finished ")
              ACE_TEXT ("successfully.\n")));

  return 0;
}

// ----------------------------------------------------

ACE_THR_FUNC_RETURN
worker_func (void *p)
{
ACE_Reactor * reactor = (ACE_Reactor *)p;

printf("\n Calling run_reactor_event_loop ()...\n");
  if (reactor->run_reactor_event_loop () != 0) {
    ACE_ERROR ((LM_ERROR,
                       ACE_TEXT ("(%t) %p\n"),
                       ACE_TEXT ("Error when running client ")
                       ACE_TEXT ("reactor event loop")));

      return (void *) -1;
    }

return 0;
}

// ----------------------------------------------------

// struct server_arg
// {
//   unsigned short port;

//   ACE_Condition<ACE_SYNCH_MUTEX> * cv;
// };

// ----------------------------------------------------

int
run_main (int, ACE_TCHAR *[])
{
//  ACE_START_TEST (ACE_TEXT ("Dev_Poll_Reactor_Test"));

  // Make sure we ignore SIGPIPE
  sigset_t sigsetNew[1];
  sigset_t sigsetOld[1];
  ACE_OS::sigemptyset (sigsetNew);
  ACE_OS::sigaddset (sigsetNew, SIGPIPE);
  ACE_OS::sigprocmask (SIG_BLOCK, sigsetNew, sigsetOld);

  ACE_Dev_Poll_Reactor dp_reactor;
  dp_reactor.restart (1);          // Restart on EINTR
  ACE_Reactor reactor (&dp_reactor);


  int flags = 0;
  ACE_SET_BITS (flags, ACE_NONBLOCK);  // Enable non-blocking in the
                                       // Svc_Handlers.
  unsigned short port = 8181;

  ACE_INET_Addr addr;

  if (addr.set (port, INADDR_LOOPBACK) != 0)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("(%t) %p\n"),
                  ACE_TEXT ("server_worker - ACE_INET_Addr::set")));

      return -1;
    }

  
  TestAcceptor server;

#define REPRODUCE_BUG
#ifdef REPRODUCE_BUG
  ACE_Reactor server_reactor (&dp_reactor); //  server and client share dp_reactor.
#else
  ACE_Dev_Poll_Reactor server_dp_reactor;  // server uses own reactor
  server_dp_reactor.restart (1);
  ACE_Reactor server_reactor (&server_dp_reactor); #endif

  ACE_SET_BITS (flags, ACE_NONBLOCK);  // Enable non-blocking in the
                                       // Svc_Handlers.
  printf("\n~~~~> Opening listening port: %d\n", port);
  if (server.open (addr, &server_reactor, flags) != 0)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("(%t) %p\n"),
                  ACE_TEXT ("Unable to open server service handler")));

      return -1;
    }



  printf("\n~~~~> creating worker threads...\n" );
  if (ACE_Thread_Manager::instance ()->spawn_n (4, worker_func, &reactor) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%t) %p\n"),
                       ACE_TEXT ("Unable to spawn worker thread")),
                      -1);

  printf("\n Calling server run_reactor_event_loop ()...\n");
  if (server_reactor.run_reactor_event_loop () != 0)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%t) %p\n"),
                       ACE_TEXT ("Error when running client ")
                       ACE_TEXT ("reactor event loop")),
                      -1);


  if (ACE_Thread_Manager::instance ()->wait () != 0)
    ACE_ERROR_RETURN ((LM_ERROR,
                       ACE_TEXT ("(%t) %p\n"),
                       ACE_TEXT ("Error waiting for threads to complete")),
                      -1);

  printf("done :-)\n");
  //ACE_END_TEST;

  return 0;
}

#else

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Dev_Poll_Reactor_Test"));
  ACE_ERROR ((LM_INFO,
              ACE_TEXT ("Dev Poll and Event Poll are not supported ")
              ACE_TEXT ("on this platform\n")));
  ACE_END_TEST;
  return 0;
}

#endif  /* ACE_HAS_DEV_POLL || ACE_HAS_EVENT_POLL */

