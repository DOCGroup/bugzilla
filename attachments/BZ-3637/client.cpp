#include "CorbaSimpleC.h"
#include <iostream>
#include <tao/ORB_Core.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Reactor.h>

class Simple_EH : public ACE_Event_Handler
{
public:
	Simple_EH (CORBA::ORB_ptr orb,
		   ACE_Reactor* r, CorbaSimple_ptr simple,
		   int maxCount, int* retval)
		: ACE_Event_Handler(r)
		, _orb(CORBA::ORB::_duplicate(orb))
		, _simple(CorbaSimple::_duplicate(simple))
		, _retval (retval)
		, _maxCount(maxCount)
		, _count (0)
	{
		reactor()->schedule_timer(this, 0, ACE_Time_Value(0));
	}

	int handle_timeout (const ACE_Time_Value&, const void*)
	{
		try
		{
			std::cout << "INFO:   calling hello()" << std::endl;
			_simple->hello(0);
			std::cout << "INFO:   call good." << std::endl;
		}
		catch (CORBA::TRANSIENT& e)
		{
			std::cout << "FAIL:   " << e << std::endl;
			*_retval = 1;
		}
		catch (CORBA::COMM_FAILURE& e)
		{
			std::cout << "FAIL:   " << e << std::endl;
			*_retval = 2;
		}
		catch (...)
		{
			std::cout << "FAIL:   Unknown exception caught"
				  << std::endl;
			*_retval = 3;
		}
		if (++_count >= _maxCount || *_retval)
		{
			_orb->shutdown(0);
		}
		else
		{
			reactor()->schedule_timer(this, 0, ACE_Time_Value(5));
		}
		return 0;
	}

private:
	CORBA::ORB_var _orb;
	CorbaSimple_var _simple;
	int* _retval;

	const int _maxCount;
	int _count;
};

int main (int argc, char* argv[])
{
	std::cout << "INFO:   Waiting for user to enter IOR... "  << std::endl;
	std::string ior;
	std::cin >> ior;
	int rv = 0;
	try
	{
		CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
		CORBA::Object_var o = orb->string_to_object (ior.c_str());
		if (CORBA::is_nil (o.in()))
		{
			std::cerr << "FAIL:  NIL reference" << std::endl;
			return -1;
		}
		CorbaSimple_var simple =
			CorbaSimple::_unchecked_narrow (o.in());
		ACE_Reactor* r = orb->orb_core()->reactor();
		Simple_EH* svc = new Simple_EH (orb, r, simple, 2, &rv);
		ACE_UNUSED_ARG(svc);
		orb->run();
	}
	catch (CORBA::Exception& e)
	{
		std::cerr << e << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cerr << "FAIL:  Unknown exception" << std::endl;
		return -1;
	}
	if (!rv)
	{
		std::cout << "OK:   PASSED" << std::endl;
	}
	return rv;
}
