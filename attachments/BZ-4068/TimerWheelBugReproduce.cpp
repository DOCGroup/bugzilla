#include "ace/OS_main.h"
#include "ace/Reactor.h"
#include "ace/Log_Msg.h"
#include "ace/Synch.h"
#include "ace/Timer_Wheel.h"
#include "ace/Event_Handler.h"

class Timeout_Handler : public ACE_Event_Handler {
public:
	virtual int handle_timeout (const ACE_Time_Value &,
								const void *)
	{
		ACE_DEBUG ((LM_DEBUG,
			"(%t) Timeout_Handler::handle_timeout\n"));
		return 0;
	}
};



int ACE_TMAIN (int argc, ACE_TCHAR *argv[]) {
	ACE_DEBUG ((LM_DEBUG,
			  "(%t) ACE_TMAIN\n"));

	// Reproduce Timer_Wheel_T problem.
	const unsigned int nspoke_shift = 12;
	const unsigned int nspoke = 1 << nspoke_shift; // means 2^nsopk_shift
	const unsigned int overflow_shift = sizeof(long) * 8/*bits*/ - nspoke_shift;
	unsigned int overflow = (1 << overflow_shift) - 1; // means 2^overflow_num_shift - 1

	ACE_DEBUG ((LM_DEBUG,
		"(%t) nspoke=%d, overflow=%x\n", nspoke, overflow));

	Timeout_Handler handler;
	ACE_Timer_Wheel timer_wheel(nspoke, 1, 0);
	// Schedule same time_values in timer_wheel so that we use only one spoke in timer_wheel.
	ACE_Time_Value timeout_time = ACE_OS::gettimeofday() + ACE_Time_Value(10, 0);
	long timer_id = -1;
	long prev_timer_id = timer_id;

	// first scheduling
	timer_id = timer_wheel.schedule (&handler, 0, timeout_time);
	if (timer_id == -1L) {
		ACE_ERROR ((LM_ERROR,
			"(%t) timer_id == -1\n"));
		return 1;
	}
	prev_timer_id = timer_id;

	ACE_DEBUG ((LM_DEBUG,
		"(%t) Previous counter in spoke[%d] was %x\n",
		(timer_id & (nspoke - 1)), timer_id >> nspoke_shift));

	if ((timer_id & (nspoke - 1)) == nspoke - 1) --overflow; // This is workaround for max_cnt
	for (long i = 1; i < (long)overflow; ++i){
		timer_id = timer_wheel.schedule (&handler, 0, timeout_time);
		if (timer_id == -1L) {
			ACE_ERROR ((LM_ERROR,
				"(%t) timer_id == -1\n"));
			return 1;
		}
		timer_wheel.cancel (prev_timer_id);
		prev_timer_id = timer_id;
		// There is only one timer left in the queue
	}

	ACE_DEBUG ((LM_DEBUG,
		"(%t) Previous counter in spoke[%d] %x, in tne next scheduling, the counter will overflow\n",
		(timer_id & (nspoke - 1)), timer_id >> nspoke_shift));

	for (int i = 0; i < (long)overflow * 4; ++i){
		timer_id = timer_wheel.schedule (&handler, 0, timeout_time);
		if (timer_id == -1L) {
			ACE_ERROR ((LM_ERROR,
				"(%t) timer_id == -1\n"));
			return 1;
		}
		if (i == 0) {
			// if there still remain bug in Timer_Wheel_T.cpp, never reached here.
			ACE_DEBUG ((LM_DEBUG,
				"(%t) Can you see this log? Timer_Wheel_T must have been fixed.\n"
				"(%t) Counter overflowed and counter in spoke[%d] must be 0, and is %x\n"
				"(%t) Continue scheduling...\n",
				(timer_id & (nspoke - 1)), timer_id >> nspoke_shift));
		}
		timer_wheel.cancel (prev_timer_id);
		prev_timer_id = timer_id;
		// There is only one timer left in the queue
	}


	// End of Reproduce Timer_Wheel_T problem.
	ACE_DEBUG ((LM_DEBUG,
			"(%t) ACE_TMAIN ending\n"));
	return 0;
}
