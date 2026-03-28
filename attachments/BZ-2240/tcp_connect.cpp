#include <iostream>
#include <iomanip>
#include <ace/OS.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

using namespace std;

void print_error(const char *msg)
{
	#ifdef WIN32
	if(errno>WSABASEERR)
		std::cerr << msg << ':' << ACE::sock_error(errno) << endl;
	else
	#endif
		perror(msg);
}

void call_connect(const ACE_INET_Addr &addr, bool complete)
{
	ACE_SOCK_Stream sock;
	ACE_SOCK_Connector conn;
	ACE_Time_Value start=ACE_OS::gettimeofday(),
		tv_zero(ACE_Time_Value::zero), delta;
	if(conn.connect(sock, addr, &tv_zero)>=0)
	{
		cerr << "non-blocking connection completed in initial call\n";
		return;
	}
	cerr << "errno " << errno << ' ';
	print_error("Error connecting");
	if(complete)
		cerr << "connecting using complete\n";
	else
		cerr << "connecting using ACE::handle_timed_complete\n";
	int result;
	int last_errno=0;
	for(int i=0; ; ++i)
	{
		if(complete)
		{
			result = conn.complete(sock, NULL, &tv_zero);
			if(result>=0)
			{
				cerr << "complete succeeded\n";
				break;
			}
			if(sock.get_handle()==ACE_INVALID_HANDLE)
			{
				cerr << "errno " << errno << ' ';
				print_error("Socket was closed giving up");
				break;
			}
		}
		else
		{
			ACE_HANDLE h = ACE::handle_timed_complete(
				sock.get_handle(), &tv_zero);
			if(h != ACE_INVALID_HANDLE)
			{
				cerr << "handle_timed_complete succeeded\n";
				break;
			}
		}
		if(last_errno != errno)
		{
			last_errno = errno;
			cerr << "errno " << errno << ' ';
			print_error("Error completing connection");
			delta = ACE_OS::gettimeofday() - start;
			cerr << setw(2) << i << ", "
				<< (delta.sec() + delta.usec()/1.0e6) << endl;
		}
		if(last_errno != ETIME)
			break;
	}
	delta = ACE_OS::gettimeofday() - start;
	cerr << "exiting loop at time " << (delta.sec() + delta.usec()/1.0e6)
		<< "\n\n";
	sock.close();
}

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		cerr << "Usage: " << argv[0] << " host port\n";
		return 1;
	}
	// port and host
	ACE_INET_Addr remote(argv[2], argv[1]);
	call_connect(remote, true);
	ACE_OS::sleep(ACE_Time_Value(1, 0));
	call_connect(remote, false);
	return 0;
}
