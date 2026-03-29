#include <iostream>

#include <ace/Reactor.h>
#include <ace/Dev_Poll_Reactor.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/Event_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

void* reactor_thread(void*)
{
	try
	{
		std::cerr << "Reactor is up" << std::endl;
		ACE_Reactor::instance()->owner(ACE_OS::thr_self());
		ACE_Reactor::instance()->run_reactor_event_loop(NULL);
		std::cerr << "Reactor is down" << std::endl;
	}
	catch (...)
	{
		std::cerr << "Caught exception in Reactor event loop!" << std::endl;
		ACE_OS::abort();
	}
	return NULL;
}


class My_handler : public ACE_Event_Handler
{
public:
	typedef ACE_SOCK_Stream stream_type;
    typedef ACE_INET_Addr addr_type;

    My_handler() : m_ioBuff(1024), m_suspended(false)
    {
    }

	~My_handler()
	{
//		peer().close();
		reactor()->remove_handler(this, ACE_Event_Handler::ALL_EVENTS_MASK);
	}

	int open(void*)
	{
	    ACE_INET_Addr addr;
	    peer().get_remote_addr(addr);
	    return reactor()->register_handler(this, ACE_Event_Handler::READ_MASK) == 0;
	}

	int close(u_long)
	{
		reactor()->remove_handler(this, ACE_Event_Handler::ALL_EVENTS_MASK);
		return 0;
	}

	int handle_input(ACE_HANDLE)
	{
		ssize_t count = peer().recv(m_ioBuff.wr_ptr(), m_ioBuff.space());
		if (count > 0)
		{

			// Received data from client, echo it back
			m_ioBuff.wr_ptr(count);
			ssize_t sent = peer().send(m_ioBuff.rd_ptr(), count);
			if ( sent == count ) // if all was sent
			{
				m_ioBuff.reset();
				// after 1st send - suspend the handler
				std::cerr << "--> handle_input()" << std::endl;
				// suspend handler on input
				suspend();
				std::cerr << "<-- handle_input()" << std::endl;
				return 0;
			}
		}

		// Error conditionget_handle
		abort();
		return -1;
	}

	void suspend()
	{
		std::cerr << "--> suspend()" << std::endl;
	    if (!m_suspended)
	    {
	        if ( -1 == reactor()->suspend_handler( get_handle() ) )
	        {
	            ACE_OS::abort();
	        }
	        std::cerr << "suspending handle: " << get_handle() << std::endl;
	        m_suspended = true;
	    }
	    else
	    {
	    	std::cerr << "already suspended handle: " << get_handle() << std::endl;
	    }
		std::cerr << "<-- suspend()" << std::endl;
	}

    virtual int resume_handler(void)
    {
      std::cerr << "--> My_handler::resume_handler()" << std::endl;
        int res = (true == m_suspended) ?
                             ACE_Event_Handler::ACE_APPLICATION_RESUMES_HANDLER
                             :
                             ACE_Event_Handler::ACE_REACTOR_RESUMES_HANDLER;
        if ( res == ACE_Event_Handler::ACE_APPLICATION_RESUMES_HANDLER )
        {
            std::cerr << "We will resume!" << std::endl;
        }
        else
        {
            std::cerr << "Reactor will resume!" << std::endl;
        }
        std::cerr << "<-- My_handler::resume_handler()" << std::endl;
        return res;
    }

    ACE_SOCK_Stream& peer()
    {
        return m_peer;
    }

    ACE_HANDLE get_handle() const
    {
    	return m_peer.get_handle();
    }

private:
    ACE_Message_Block 	m_ioBuff;
    ACE_SOCK_Stream 	m_peer;
    bool m_suspended;
};


int main()
{
	// Initialize Reactor, use epoll implementation
	ACE_Reactor reactor(new ACE_Dev_Poll_Reactor());
	if (!reactor.initialized())
	{
		std::cerr << "Failed to initialize the Reactor" << std::endl;
		return 1;
	}

	// Restart the Reactor event loop after being interrupted by a signal (can't debug without this)
	reactor.restart(true);

	// Set default Reactor instance, the rest of the system *must* work with it (using ACE_Reactor::instance())
	ACE_Reactor::instance(&reactor, true);

	// Run Reactor event loop in a separate thread
	ACE_Thread_Manager thread_manager;
    if (thread_manager.spawn(&reactor_thread, NULL) == -1)
    {
    	std::cerr << "Failed to spawn worker thread" << std::endl;
        return 1;
    }

	// Listen for client connections
    ACE_Acceptor<My_handler, ACE_SOCK_Acceptor> acceptor;
    ACE_INET_Addr addr(33333, "localhost");
	if ( acceptor.open(addr,&reactor,ACE_NONBLOCK) == -1)
	{
		std::cerr << "Failed to start Acceptor" << std::endl;
		return 1;
	}

	// Wait forever on Reactor event loop (or until signal)
	thread_manager.wait();

	// Stop accepting connections
	acceptor.close();

	// Stop dispatching network events
	reactor.end_reactor_event_loop();

    return 0;
}

