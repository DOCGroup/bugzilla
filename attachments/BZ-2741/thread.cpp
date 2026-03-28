
#include <sysexits.h>

#include <ace/Log_Msg.h>
#include <ace/Thread.h>
#include <ace/Thread_Manager.h>

ACE_THR_FUNC_RETURN thr_func(void*)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("in thr_func\n")));
	ACE_OS::sleep(1);
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("out thr_func\n")));
	return 0;
}

int ACE_TMAIN(int /*argc*/, ACE_TCHAR* /*argv*/[])
{
	ACE_TRACE(ACE_TEXT("main\n"));

	const int thr_num = 20;
	ACE_thread_t t_id[thr_num];

	for (int i = 0; i < thr_num; ++i) {
		ACE_DEBUG((LM_DEBUG, ACE_TEXT("spawning thread #%d\n"), i));
		if (ACE_Thread::spawn(thr_func, 0, THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, &t_id[i]) == -1) {
			ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("spawn failure: %m\n")), EX_OSERR);
		}
		ACE_DEBUG((LM_DEBUG, ACE_TEXT("spawned thread #%d (thr_id=%X)\n"), i, reinterpret_cast<unsigned long>(t_id[i])));
	}

	for (int i = 0; i < thr_num; ++i) {
		ACE_DEBUG((LM_DEBUG, ACE_TEXT("joining thread #%d\n"), i));
		ACE_Thread::join(t_id[i], 0, 0);
		ACE_DEBUG((LM_DEBUG, ACE_TEXT("joined thread #%d\n"), i));
	}

	return 0;
}

