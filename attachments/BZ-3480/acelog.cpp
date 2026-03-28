#include "ace/Log_Msg.h"
#include "ace/Task.h"
#include <fstream>

struct MyThread : ACE_Task_Base
{
  explicit MyThread(bool openfile = false)
    : openfile_(openfile) {}

  bool openfile_;
  static MyThread childthread_;

  int svc()
  {
    if (openfile_)
      {
        ACE_LOG_MSG->msg_ostream(new std::ofstream("log.txt"), 1);
        ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
        ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::LOGGER);
        childthread_.activate();
      }
    ACE_DEBUG((LM_DEBUG, "(%P|%t) - in svc() openfile_ is %s\n",
               (openfile_ ? "true" : "false")));
    if (!openfile_)
      {
        for (int i = 0; i < 100; ++i)
          {
            ACE_DEBUG((LM_DEBUG, "(%P|%t) - loop %d\n", i));
          }
      }
    return 0;
  }
};

MyThread MyThread::childthread_;

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) - in main()\n"));
  MyThread mt(true);
  mt.activate();
  mt.wait();
  MyThread::childthread_.wait();
  return 0;
}
