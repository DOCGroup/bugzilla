#include <ace/Event_Handler.h>
#include <ace/FIFO_Recv.h>
#include <ace/FIFO_Send.h>
#include <ace/Reactor.h>
#include <ace/Select_Reactor.h>

class AnEventHandler
 : public ACE_Event_Handler
{
public:

  AnEventHandler(ACE_Reactor *reactor);
  virtual ~AnEventHandler();

  virtual int handle_output(ACE_HANDLE handle);

private:

  enum State
  {
    STATE_ONE,
    STATE_TWO,
  };

  int _HandleOutputSTATE_ONE(ACE_HANDLE handle);
  int _HandleOutputSTATE_TWO(ACE_HANDLE handle);

  ACE_Reactor *mReactor;

  enum State mState;

  ACE_FIFO_Recv mRecvFifo1;
  ACE_FIFO_Send mSendFifo1;
  ACE_FIFO_Recv mRecvFifo2;
  ACE_FIFO_Send mSendFifo2;
};

AnEventHandler::AnEventHandler(ACE_Reactor *reactor)
 : mReactor(reactor)
 , mState(STATE_ONE)
{
  if(mRecvFifo1.open("fifo.1") == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Open FIFO1 failed\n")));
    ACE_OS::exit(1);
  }

  if(mRecvFifo2.open("fifo.2") == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Open FIFO1 failed\n")));
    ACE_OS::exit(1);
  }

  if(mSendFifo1.open("fifo.1") == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Open FIFO1 failed\n")));
    ACE_OS::exit(1);
  }

  if(mReactor->register_handler(mSendFifo1.get_handle(), this, ACE_Event_Handler::WRITE_MASK) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Register FIFO1 handler failed\n")));
    ACE_OS::exit(1);
  }
}

AnEventHandler::~AnEventHandler()
{
  if(mSendFifo2.close() == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Close FIFO2 failed\n")));
  }

  if(mRecvFifo2.remove() == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Remove FIFO2 failed\n")));
  }

  if(mRecvFifo1.remove() == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Remove FIFO1 failed\n")));
  }
}

int
AnEventHandler::handle_output(ACE_HANDLE handle)
{
  switch(mState)
  {
  case STATE_ONE: return _HandleOutputSTATE_ONE(handle);
  case STATE_TWO: return _HandleOutputSTATE_TWO(handle);
  }

  ACE_ERROR((LM_ERROR, ACE_TEXT("Bad state: %d\n"), mState));
  ACE_OS::exit(1);
}

int
AnEventHandler::_HandleOutputSTATE_ONE(ACE_HANDLE handle)
{
  if(mReactor->cancel_wakeup(mSendFifo1.get_handle(), ACE_Event_Handler::WRITE_MASK) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Cancel wakeup on write of FIFO1 failed\n")));
    ACE_OS::exit(1);
  }

  if(mSendFifo2.open("fifo.2") == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Open FIFO2 failed\n")));
    ACE_OS::exit(1);
  }

  if(mReactor->register_handler(mSendFifo2.get_handle(), this, ACE_Event_Handler::WRITE_MASK) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Register FIFO2 handler failed\n")));
    ACE_OS::exit(1);
  }

  mState = STATE_TWO;

  return 0;
}

int
AnEventHandler::_HandleOutputSTATE_TWO(ACE_HANDLE handle)
{
  if(mReactor->remove_handler(mSendFifo2.get_handle(), ACE_Event_Handler::ALL_EVENTS_MASK|ACE_Event_Handler::DONT_CALL) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Remove FIFO2 handler failed\n")));
    ACE_OS::exit(1);
  }

  if(mSendFifo2.close() == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Close FIFO2 failed\n")));
    ACE_OS::exit(1);
  }

  if(mReactor->remove_handler(mSendFifo1.get_handle(), ACE_Event_Handler::ALL_EVENTS_MASK|ACE_Event_Handler::DONT_CALL) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Remove FIFO1 handler failed\n")));
    ACE_OS::exit(1);
  }

  if(mSendFifo1.close() == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("Close FIFO1 failed\n")));
    ACE_OS::exit(1);
  }

  mReactor->end_reactor_event_loop();

  return 0;
}

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  ACE_Reactor *reactor = new ACE_Reactor(new ACE_Select_Reactor(), 1);

  AnEventHandler *eh = new AnEventHandler(reactor);

  reactor->run_reactor_event_loop();

  delete eh;

  delete reactor;

  return 0;
}
