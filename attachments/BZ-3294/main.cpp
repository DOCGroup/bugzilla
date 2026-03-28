// $Id:  $
//#include "flcRegFrmApp_pch.h"
// temporary: put the "precompiled headers" inline
// while submitting test to bugzilla.  This is overkill for the 
// bug.
//
// ACE stuff
#include "ace/config-all.h"
#include "ace/ACE.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_sys_time.h"
#include "ace/High_Res_Timer.h"
#include <ace/Service_Config.h>
#include <ace/Reactor.h>
#include <ace/streams.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/Select_Reactor.h>
#include <ace/WFMO_Reactor.h>
#include <ace/Event.h>

// Boost stuff
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/minmax.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

// std stuff
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <queue>
#include <vector>
#include <list>
#include <stdexcept>
#include <string>
#include <iomanip>
#include <algorithm>
#include <limits>


class BaseHandler : public ACE_Event_Handler
{
public:
  BaseHandler(unsigned long id, ACE_Reactor * reactor)
    : id_(id)
    , reactor_(reactor)
    , companion_(0)
    , count_(0)
  {
  }
  virtual ~BaseHandler()
  {
  }

  void companion(BaseHandler * companion)
  {
    companion_ = companion;
  }

  void signalCompanion()
  {
    if(0 != companion_)
    {
      companion_->signal();
    }
  }

  virtual void signal() = 0;

  unsigned long count() const
  {
    return count_;
  }

protected:
  unsigned long id_;
  ACE_Reactor * reactor_;
  BaseHandler * companion_;
  unsigned long count_;
};

class EventHandler : public BaseHandler
{
public:
  EventHandler(unsigned long id, ACE_Reactor * reactor)
    : BaseHandler(id, reactor)
    , cycleCount_(1)
    , endCount_(0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("register handler event %d\n"), id_));
    // The "standard" register_handler() insists that the handle be a socket
    // This alternative call -- specifying the handle in place of the mask
    // -- allows any arbitrary waitable Windows handle to be registered
    // with a WFMO reactor.
    if(0 != reactor_->register_handler(
      this, 
//      ACE_Event_Handler::READ_MASK | ACE_Event_Handler::SIGNAL_MASK
      event_.handle()
      ))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("Registration failed %d\n"), id_));
    }
  }


  virtual ~EventHandler()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("remove handler event %d\n"), id_));
    reactor_->remove_handler(
      this, 
      ACE_Event_Handler::READ_MASK | 
      ACE_Event_Handler::SIGNAL_MASK | 
      ACE_Event_Handler::DONT_CALL);
  }

  virtual void signal()
  {
//    ACE_DEBUG((LM_DEBUG, ACE_TEXT("signal \n")));
    event_.signal();
  }

  void setCycle(unsigned int cycleCount)
  {
    cycleCount_ = cycleCount;
  }

  void setEnd(unsigned int endCount)
  {
    endCount_ = endCount;
  }

  void wait(ACE_Time_Value * until)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Wait \n")));
    event_.wait(until);
  }

  virtual ACE_HANDLE get_handle (void) const
  {
    ACE_HANDLE h = event_.handle();
//    ACE_DEBUG((LM_DEBUG, ACE_TEXT("get handle: %x\n"), h));
    return h; //event_.handle();
  }

  virtual int handle_signal (
    int signum, 
    siginfo_t * info, ucontext_t * context)
  {
    ++count_;
    unsigned int cycle = count_ % cycleCount_;

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle signal Event %d: %d (%d)\n"), id_, count_, cycle));

    if(0 == cycle)
    {
      signalCompanion();
    }
    else
    {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("returning nonzero %d: %d (%d)\n"), id_, count_, cycle));
    }

    if(endCount_ > 0 && count_ >= endCount_)
    {
      reactor_->end_reactor_event_loop();
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("End Reactor Event Loop\n")));
    }

    return cycle;
  }

  virtual int handle_exception (ACE_HANDLE fd = ACE_INVALID_HANDLE)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Handle Exception\n")));
    return -1;
  }

  virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle close\n")));
    reactor_->remove_handler(
      this, 
      ACE_Event_Handler::READ_MASK | 
      ACE_Event_Handler::SIGNAL_MASK | 
      ACE_Event_Handler::DONT_CALL);
    return 0;
  }

private:
  ACE_Event event_;
  unsigned long cycleCount_;
  unsigned long endCount_;
};


class NotifyableHandler : public BaseHandler
{
public:
  NotifyableHandler(unsigned long id, ACE_Reactor * reactor)
    : BaseHandler(id, reactor)
  {
  }

  ~NotifyableHandler()
  {
  }

  void signal()
  {
//    ACE_DEBUG((LM_DEBUG, ACE_TEXT("signal \n")));
    reactor_->notify(this);
  }

  virtual int handle_exception (ACE_HANDLE fd = ACE_INVALID_HANDLE)
  {
    ++count_;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle exception notify %d\n"), id_, count_));
    signalCompanion();
    return 0;
  }
};


class SockHandler : public BaseHandler
{
public:
  SockHandler(unsigned long id, ACE_Reactor * reactor)
    : BaseHandler(id, reactor)
  {
    listen(5555);
    connect("localhost", 5555);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("register handler socket %d\n"), id_));
    if(0 != reactor_->register_handler(
      this, 
      ACE_Event_Handler::READ_MASK | ACE_Event_Handler::SIGNAL_MASK
      ))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("Can't register sockHandler with reactor %d\n"), id_));
    }
    while(!accept())
    {
    }
  }

  virtual ~SockHandler()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("remove handler socket %d\n"), id_));
    reactor_->remove_handler(
      this, 
      ACE_Event_Handler::READ_MASK | 
      ACE_Event_Handler::SIGNAL_MASK | 
      ACE_Event_Handler::DONT_CALL);
  }

  virtual ACE_HANDLE get_handle (void) const
  {
    return inPeer_.get_handle();
    ACE_HANDLE h = inPeer_.get_handle();
//    ACE_DEBUG((LM_DEBUG, ACE_TEXT("get handle socket: %x\n"), h));
    return h;
  }

  virtual int handle_input (ACE_HANDLE fd)
  {
    ++count_;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle input socket %d: %d\n"), id_, count_));
    char aBuffer[1000];
    if(0 >= inPeer_.recv(aBuffer, sizeof(aBuffer)))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("disconnnect socket %d\n"), id_));
      return -1;
    }
    signalCompanion();
    return 0;
  }

  virtual void signal()
  {
    send("signal");
  }

  bool
  connect(
    const char * theHost,
    unsigned short port,
    ACE_Time_Value * theTimeout = 0
    )
  {
    // Create an INET address based on the hostName and portNumber provided
    ACE_INET_Addr address(port,theHost);

    //Establish a connection with the server
    ACE_SOCK_Connector connector;
    if (connector.connect(inPeer_, address, theTimeout) == -1 )
    {
      ACE_ERROR((LM_ERROR,
        ACE_TEXT("FALCON (%P|%t) flcConnection::connect ")
        ACE_TEXT("connect to %s:%d failed %p\n"),
        theHost,
        port,
        ACE_TEXT("Error code")
        ));
      return false;
    }
    // Disable Nagle's Algorithm
    inPeer_.enable(TCP_NODELAY);
    return true;
  }

  bool listen(short port)
  {
    bool result = true;
    ACE_INET_Addr address;
    address.set(port);
    if(-1 == acceptor_.open(address))
    {
      ACE_ERROR ((LM_ERROR,
        ACE_TEXT("FALCON (%P|%t) flcConnection::listen ")
        ACE_TEXT("listen on port %d failed %p\n"),
        port,
        ACE_TEXT("Error code")
        ));
      result = false;
    }
    return result;
  }

  bool accept()
  {
    ACE_Time_Value poll(0,0);
    if(-1 == acceptor_.accept(outPeer_, 0, &poll))
    {
      ACE_DEBUG((LM_INFO, ACE_TEXT("poll for accept\n")));
      return false;
    }
    acceptor_.close();
    return true;
  }

  bool send(std::string buffer)
  {
    size_t sent = outPeer_.send_n( buffer.c_str(), buffer.size());
    return sent == buffer.size();
  }

  unsigned long count() const
  {
    return count_;
  }

private:
  ACE_SOCK_Stream inPeer_;
  ACE_SOCK_Stream outPeer_;
  ACE_SOCK_Acceptor acceptor_;
};

int main(int argc, char* argv[])
{
#ifdef USING_SELECT_REACTOR
  ACE_Select_Reactor * reactorImpl = new ACE_Select_Reactor();
#else // USING_SELECT_REACTOR
  ACE_WFMO_Reactor * reactorImpl = new ACE_WFMO_Reactor();
#endif // USING_SELECT_REACTOR
  ACE_Reactor * reactor = new ACE_Reactor(reactorImpl,1);
  ACE_Reactor::instance(reactor, 1);

  EventHandler h1(1, reactor);
  EventHandler h2(2, reactor);
  SockHandler h3(3, reactor);
  EventHandler h4(4, reactor);
//#define NOTIFY_HANDLER
#ifdef NOTIFY_HANDLER
  NotifyableHandler h5(5, reactor);
  h5.signal();
#else // NOTIFY_HANDLER
  EventHandler h5(5, reactor);
#endif // NOTIFY_HANDLER

  EventHandler h6(6, reactor);
#define RETURN_POSITIVE
#ifdef RETURN_POSITIVE
  // note h6 is independent
  // It returns non-zero to ask for a redispatch
  // then triggers itself.
  // (Note comment out the setCycle call to have it signal itself
  //  during each handle_signal.)
  h6.setCycle(2);
  h6.companion(&h6); // signals itself
  h6.signal();
#endif RETURN_POSITIVE

  // stop after 10 iterations
  h1.setEnd(10);
#ifdef LINK_BACKWARD
  h1.companion(&h5);
  h2.companion(&h1);
  h3.companion(&h2);
  h4.companion(&h3);
  h5.companion(&h4);
#else
  h1.companion(&h2);
  h2.companion(&h3);
  h3.companion(&h4);
  h4.companion(&h5);
  h5.companion(&h1);
#endif

  h1.signal();

  ACE_Time_Value testTime(5,0);
#ifndef EXPLICIT_EVENT_LOOP
  while(h1.count() < 10)
  {
    reactor->handle_events(testTime);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Event loop\n")));
  }
#else
  reactor->run_reactor_event_loop(testTime);
#endif
  std::cout << "H1: " <<  h1.count() << std::endl;
  std::cout << "H2: " <<  h2.count() << std::endl;
  std::cout << "H3: " <<  h3.count() << std::endl;
  std::cout << "H4: " <<  h4.count() << std::endl;
  std::cout << "H5: " <<  h5.count() << std::endl;
  std::cout << "H6: " <<  h6.count() << std::endl;

  if(h6.count() == 1)
  {
    // if the reactor is not honoring the callback request
    // h6 only gets one dispatch
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("ERROR: Test failed. Reactor is not honoring return value from handle_signal()\n")));
    return -1;
  }
  return 0;

}
