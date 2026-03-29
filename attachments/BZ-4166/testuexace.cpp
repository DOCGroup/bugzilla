// 
//

#include "ace/Task.h"
#include "ace/Recursive_Thread_Mutex.h"
#include "ace/Guard_T.h"
#include "ace/Thread_Semaphore.h"
#include "ace/OS.h"

#include <iostream>
#include <string>


using namespace std;

typedef ACE_Guard<ACE_Recursive_Thread_Mutex> RTM_Guard;

//Mutex that will be orphaned due to a crashing thread.
ACE_Recursive_Thread_Mutex TheMutex;

//Ensures the crashing thread owns 'TheMutex' before the main thread.
ACE_Thread_Semaphore sema;

void crashing_function( )
{
    RTM_Guard g( TheMutex);

    //signal that this thread owns 'TheMutex' to main thread
    sema.release();

    int* pint = 0;
    ACE_OS::sleep( 3 ); //3s

    //Crash: raises the SEH exception 0xC0000005
    *pint = 5;
}

class AceTaskWithUnhandledSEHException: public ACE_Task<ACE_MT_SYNCH>
{
  virtual int svc (void)
  {
	  crashing_function();
	  return 0;
  }
};


void crashing_thread_endthreadex_in_finally( void*)
{
    __try
    {
        crashing_function();
    }
    __finally
    {
        cout << "_endthreadex \n";
        ::_endthreadex (0);
    }
}

void crashing_thread_endthreadex_after_finally( void*)
{
    __try
    {
        crashing_function();
    }
    __finally
    {
        cout << "__finally\n";
    }
    ::_endthreadex (0);
}

ACE_THR_FUNC_RETURN crashing_aceos_thread( void*)
{
    crashing_function( );
    return 0;
}


int main(int argc, char* argv[])
{
    string test = ( argc > 1) ? argv[1] : "";
    
    AceTaskWithUnhandledSEHException task;

    sema.acquire();

    if (test == string("infin"))
    {
        cout << "Starting crashing CRT thread. (with _endthreadex() in __finally)\n"
                "  Expecting main thread to hang.\n";

        _beginthread(crashing_thread_endthreadex_in_finally, 0, NULL );
    }
    else if (test == string("afterfin"))
    {
        cout << "Starting crashing CRT thread (with _endthreadex() after __finally).\n"
                "  Expecting process to be terminated by OS.\n";

        _beginthread(crashing_thread_endthreadex_after_finally, 0, NULL );
    }
    else if (test == string("ace"))
    {
        cout << "Starting crashing ACE thread.\n"
            "  Expecting main thread to hang.\n";
        task.activate();

    }
    else if (test == string("aceos"))
    {
        cout << "Starting crashing ACE OS thread.\n"
            "  Expecting main thread to hang.\n";
        ACE_thread_t thread = 0;
        ACE_OS::thr_create( crashing_aceos_thread, 0, THR_JOINABLE, &thread);
    }
    else if (test == string("aceafx"))
    {
        cout << "Starting crashing ACE OS AFX thread.\n"
            "  Expecting main thread to hang.\n";
        ACE_thread_t thread = 0;
        ACE_OS::thr_create( crashing_aceos_thread, 0, THR_JOINABLE | THR_USE_AFX, &thread);
    }
    else
    {
        cout 
        << "Test behavior with a crashing thread."
           "Usage: testuexace ace|infin|afterfin\n"
           "  ace : Crash in an ACE Task\n"
           "  aceos : Crash in an ACE OS thread\n"
           "  infin : Crash in an MS CRT thread with a call of _endthreadex() in the __finally block like ACE.\n"
           "  afterfin :  Crash in an MS CRT thread with a call of _endthreadex() after the __finally block.\n"
           ;
        return 1;
    }
    

    cout << "Wait until crashing thread owns 'TheMutex'.\n";
    sema.acquire();

    cout << "Try to acquire 'TheMutex'.\n"
            " Program is expected to hang here (wrong) or to be terminated (good).\n";
    RTM_Guard g( TheMutex);

    //Real application would access data protected by 'TheMutex'.

    cout << "Normal termination (shall not be reached).\n";
	return 0;
}

