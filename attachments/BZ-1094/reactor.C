
#include "ace/Reactor.h"

class MyTimerHandler : public ACE_Event_Handler
{
	public:
		int     handle_timeout(const ACE_Time_Value &tv, const void *arg);

};

int MyTimerHandler::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	cout << ACE_OS::gettimeofday().msec() << " | MyTimerHandler::handle_timeout" << endl;
	return -1;
}

int main()
{
	ACE_Reactor *reactor = new ACE_Reactor();
	MyTimerHandler *handler = new MyTimerHandler();

	//cout << ACE_OS::gettimeofday().msec() << " | "

	int res;

	for(int i = 0; i < 10; ++i) {
		cout << ACE_OS::gettimeofday().msec() << " | Scheduling timer" << endl;
		res = reactor->schedule_timer(handler, 0, ACE_Time_Value(1, 0));
		cout << ACE_OS::gettimeofday().msec() << " | Scheduled timer - result:" << res << endl;

		do {
			cout << ACE_OS::gettimeofday().msec() << " | Handling events" << endl;
			res = reactor->handle_events();
			cout << ACE_OS::gettimeofday().msec() << " | Number of events processed: " << res << endl;
		} while (res == 0);
	}
/*
	cout << ACE_OS::gettimeofday().msec() << " | Handling events" << endl;
	res = reactor->handle_events();
	cout << ACE_OS::gettimeofday().msec() << " | Number of events processed: " << res << endl;

	cout << ACE_OS::gettimeofday().msec() << " | Handling events" << endl;
	res = reactor->handle_events();
	cout << ACE_OS::gettimeofday().msec() << " | Number of events processed: " << res << endl;
*/
	return 0;

}
