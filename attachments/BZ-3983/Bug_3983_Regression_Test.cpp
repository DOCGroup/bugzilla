#include "test_config.h"
#include "ace/Reactor.h"
#include "ace/Dev_Poll_Reactor.h"
#include "ace/Acceptor.h"
#include "ace/Connector.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/Event_Handler.h"
#include "ace/SOCK_Stream.h"
#include "ace/INET_Addr.h"

static const unsigned short SERVER_PORT = 33333;
static const unsigned int TIMEOUT = 60; // Seconds

void* server_thread(void*);


class Echo_Server : public ACE_Event_Handler
{
public:
    typedef ACE_SOCK_Stream stream_type;
    typedef ACE_INET_Addr addr_type;

    Echo_Server() : m_suspended(false) {}
    ~Echo_Server() {this->close(0);}
    ACE_SOCK_Stream& peer() {return m_peer;}
    ACE_HANDLE get_handle() const {return m_peer.get_handle();}

    int open(void*) {return reactor()->register_handler(this,
            ACE_Event_Handler::READ_MASK) == 0;}

    int close(u_long) {return reactor()->remove_handler(this,
            ACE_Event_Handler::ALL_EVENTS_MASK) == 0;}

    int handle_input(ACE_HANDLE)
    {
    	if (m_suspended)
    	{
    		// Handler should not receive events if suspended
    		ACE_ERROR((LM_ERROR, ACE_TEXT("Bug 3983: Echo_Server received "
    				"event while suspended!\n")));
    		ACE_OS::abort();
    	}

    	char buf[BUFSIZ] = {0};
        ssize_t received = peer().recv(buf, sizeof(buf)-1);
        if (received > 0)
        {
            // Received data from client, echo it back
        	ACE_DEBUG((LM_INFO, ACE_TEXT("Echo_Server responds \"%s\"\n"),
        			buf));
            ssize_t sent = peer().send(buf, received);
            if (sent == received)
            {
            	// Suspend the handler after echoing back first input
                suspend();
                ACE_DEBUG((LM_INFO, ACE_TEXT("Echo_Server is now suspended, "
                		"no further events should arrive\n")));
                return 0;
            }
        }

        ACE_ERROR((LM_ERROR, ACE_TEXT("Echo_Server failed "
        		"to communicate with client\n")));
        ACE_OS::abort();
        return -1;
    }

    void suspend()
    {
		if (reactor()->suspend_handler(get_handle()) == -1)
		{
			ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to suspend Echo_Server\n")));
			ACE_OS::abort();
		}
		m_suspended = true;
    }

    int resume_handler()
    {
        return m_suspended ?
        		ACE_Event_Handler::ACE_APPLICATION_RESUMES_HANDLER :
        		ACE_Event_Handler::ACE_REACTOR_RESUMES_HANDLER;
    }

private:
    ACE_SOCK_Stream     m_peer;
    bool                m_suspended;
};



void* server_thread(void*)
{
	// Initialize server Reactor using epoll implementation
	ACE_Dev_Poll_Reactor dp_reactor;
	dp_reactor.restart(true);
	ACE_Reactor reactor(&dp_reactor);

	ACE_INET_Addr addr(SERVER_PORT, INADDR_LOOPBACK);
	ACE_Acceptor<Echo_Server, ACE_SOCK_Acceptor> acceptor;
	if (acceptor.open(addr, &reactor) == -1)
	{
		ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to start acceptor\n")));
		return (void*)-1;
	}

	if (reactor.run_reactor_event_loop () != 0)
	{
		ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to run server event loop\n")));
		return (void*)-1;
	}

	return 0;
}



class Echo_Client: public ACE_Event_Handler
{
public:
    typedef ACE_SOCK_Stream stream_type;
    typedef ACE_INET_Addr addr_type;

    Echo_Client() : m_message_num(0) {}
    ~Echo_Client() {this->close(0);}
    ACE_SOCK_Stream& peer() {return m_peer;}
    ACE_HANDLE get_handle() const {return m_peer.get_handle();}

    int open(void*)
    {
    	return reactor()->register_handler(this,
            ACE_Event_Handler::READ_MASK |
            ACE_Event_Handler::WRITE_MASK |
            ACE_Event_Handler::TIMER_MASK) == 0;
    }

    int close(u_long)
    {
    	return reactor()->remove_handler(this,
            ACE_Event_Handler::ALL_EVENTS_MASK) == 0;
    }

    int handle_input(ACE_HANDLE)
    {
    	char buf[32] = {0};
        ssize_t received = peer().recv(buf, sizeof(buf)-1);
        if (received > 0)
        {
            // Received data back from the server, print it and enable output
        	// events to send next message
            ACE_DEBUG((LM_INFO, ACE_TEXT("Echo_Client got  \"%s\"\n"), buf));
            if (reactor()->register_handler(this,
            		ACE_Event_Handler::WRITE_MASK))
            {
    			ACE_ERROR((LM_ERROR, ACE_TEXT("Echo_Client failed to register "
    					"handler for output events\n")));
    			ACE_OS::abort();
            }
            return 0;
        }

        ACE_ERROR((LM_ERROR, ACE_TEXT("Echo_Client recv() failed\n")));
        ACE_OS::abort();
        return -1;
    }

    int handle_output(ACE_HANDLE)
    {
		char buf[BUFSIZ] = {0};
		ACE_OS::sprintf(buf, "message #%d", ++m_message_num);

		ACE_DEBUG((LM_INFO, ACE_TEXT("Echo_Client sends \"%s\"\n"), buf));

		ssize_t sent = peer().send(buf, ACE_OS::strlen(buf));
		if (sent <= 0)
		{
			ACE_ERROR((LM_ERROR, ACE_TEXT("Echo_Client send() failed\n")));
			ACE_OS::abort();
		}

		if (reactor()->remove_handler(this, ACE_Event_Handler::WRITE_MASK))
		{
			ACE_ERROR((LM_ERROR, ACE_TEXT("Echo_Client failed to remove "
					"handler for output events\n")));
		}

		if (m_message_num == 2)
		{
			// The second message shouldn't be echoed by the server since it
			// will be suspended. If after a minute we don't get a response
			// consider the server suspended and end the test with success.
			if (reactor()->schedule_timer(this, 0, ACE_Time_Value(TIMEOUT))
					== -1)
			{
				ACE_ERROR((LM_ERROR, ACE_TEXT("Echo_Client failed to schedule "
						"timer\n")));
				ACE_OS::abort();
			}
		}

    	return 0;
    }

    int handle_timeout(const ACE_Time_Value&, const void*)
    {
    	ACE_DEBUG((LM_INFO, ACE_TEXT("Echo_Client timed out waiting for server"
    			" response to the second message. If this is the result of the"
    			" server being suspended then the bug has been fixed.\n")));
    	ACE_OS::abort();
    	return 0;
    }

private:
    ACE_SOCK_Stream m_peer;
    int m_message_num;
};



int run_main(int, ACE_TCHAR*[])
{
	ACE_START_TEST(ACE_TEXT("Bug_3983_Regression_Test"));

    // Run server in a separate thread
    if (ACE_Thread_Manager::instance()->spawn(&server_thread, 0) == -1)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to spawn server thread\n")));
        return -1;
    }

    // Wait for the listening endpoint to be set up
    ACE_OS::sleep(5);

    // Initialize client Reactor using epoll implementation
	ACE_Dev_Poll_Reactor dp_reactor;
	dp_reactor.restart(true);
	ACE_Reactor reactor(&dp_reactor);

	// Connect to server
    Echo_Client* client = 0;
    ACE_INET_Addr addr(SERVER_PORT, INADDR_LOOPBACK);
    ACE_Connector<Echo_Client, ACE_SOCK_Connector> connector(&reactor);
    if (connector.connect(client, addr) == -1)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to connect to server\n")));
        return -1;
    }

	if (reactor.run_reactor_event_loop () != 0)
	{
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to run client event loop\n")));
        return -1;
	}

	ACE_Thread_Manager::instance()->wait();
	ACE_END_TEST;
    return 0;
}




