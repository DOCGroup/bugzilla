#include <QtCore>
#include <QtGui>

#include <ace/INET_Addr.h>
#include <ace/QtReactor/QtReactor.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>

#include <assert.h>
#include <pthread.h>

#include <iostream>
#include <queue>


#include "main.h"

const unsigned int PORT = 7892;

pthread_t g_server_thread;

void* echo_server(void*)
{
    // Error handling omitted.

    ACE_INET_Addr my_addr(PORT);
    ACE_SOCK_Acceptor acceptor(my_addr);
    ACE_SOCK_Stream new_stream;
    std::cerr << "Creating server on port " << PORT << std::endl;
    acceptor.accept(new_stream);
    while(1) {
        char buf[BUFSIZ];
        ssize_t n = new_stream.recv(buf, BUFSIZ);
        new_stream.send_n(buf, n);
    }
    return 0;
}

class Handler : public ACE_Event_Handler
{
public:
    Handler();
    virtual ~Handler();
    virtual int handle_input(ACE_HANDLE handle);
    virtual int handle_output(ACE_HANDLE handle);
    ACE_HANDLE get_handle(void) const;
    void add_to_output_queue(const std::string&);
    bool remove_handler_if_empty();

private:
    ACE_INET_Addr       server;
    ACE_SOCK_Connector  connector;
    ACE_SOCK_Stream     stream;
    std::queue<std::string> output_queue;
};

Handler::Handler() : ACE_Event_Handler()
{
    server.set(PORT, "127.0.0.1");
    ACE_ASSERT(connector.connect(stream, server) != -1);
    ACE_ASSERT(
        ACE_Reactor::instance()->register_handler(
            this, ACE_Event_Handler::READ_MASK)
        == 0);
}

Handler::~Handler()
{
    int mask = ACE_Event_Handler::READ_MASK | ACE_Event_Handler::WRITE_MASK
        | ACE_Event_Handler::DONT_CALL;
    ACE_Reactor::instance()->remove_handler(this, mask);

}

ACE_HANDLE Handler::get_handle(void) const
{
    return stream.get_handle();
}


int Handler::handle_input(ACE_HANDLE)
{
    static const int BUFFSIZE = 4096;
    char buf[BUFFSIZE];
    int rv;
    memset(buf, 0, BUFFSIZE);
    rv = stream.recv(buf, BUFFSIZE - 1);
    if(rv > 0) {
        std::cout << "Received: [" << buf << "]\n";
        rv = 0;
    } else {
        std::cerr << "no connection [" << rv << "]\n";
        rv = -1;
    }
    return rv;
}

int Handler::handle_output(ACE_HANDLE)
{
    if(!remove_handler_if_empty()) {
        const std::string &val = output_queue.front();
        std::cerr << "Sending: [" << val << "]\n";
        int n = stream.send(val.c_str(), val.size());
        assert(n > 0);
        output_queue.pop();
        remove_handler_if_empty();
    }
    return 0;
}

bool Handler::remove_handler_if_empty()
{
    bool rv = false;
    if(output_queue.size() == 0) {
        ACE_Reactor::instance()->remove_handler(
            this, ACE_Event_Handler::WRITE_MASK | ACE_Event_Handler::DONT_CALL);
        rv = true;
    }
    return rv;
}

void Handler::add_to_output_queue(const std::string &msg)
{
    output_queue.push(msg);
    ACE_Reactor::instance()->register_handler(
                this, ACE_Event_Handler::WRITE_MASK);
}

TestApp::TestApp(int argc, char **argv) : QApplication(argc, argv)
{
}

TestApp::~TestApp() 
{
    delete handler;
}

int TestApp::exec()
{
    handler = new Handler();
    handler->add_to_output_queue("Hello bugs");
    return QApplication::exec();
}


int main(int argc, char** argv)
{
    TestApp app(argc, argv);

    ACE_QtReactor qt_reactor((QApplication *)&app);
    ACE_Reactor reactor(&qt_reactor);
    ACE_Reactor::instance(&reactor, false);

    pthread_create(&g_server_thread, NULL, echo_server, 0);
    
    return app.exec();
}
