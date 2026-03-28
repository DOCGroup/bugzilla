#include <iostream>
using namespace std;
#include <assert.h>

#include <pthread.h>

#include <ace/Reactor.h>
#include <ace/Dev_Poll_Reactor.h>
#include <ace/Event_Handler.h>

extern bool foo;

ACE_Dev_Poll_Reactor reactor_impl(ACE::max_handles(), 1);
ACE_Reactor reactor(&reactor_impl);

int notify_count = 0;
int input_count = 0;

class Handler : public ACE_Event_Handler {
private:
    ACE_Reactor &m_reactor;

public: // our interface
    Handler(ACE_Reactor &reactor) : m_reactor(reactor) { };
    ~Handler() { };
public: // ACE interface
    virtual ACE_HANDLE get_handle(void) const { return 1; };
    int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE) {
        cout << "input " << (input_count++) << endl;
        return 0;
    };
    int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE) {return 0;};
    int handle_close(ACE_HANDLE, ACE_Reactor_Mask) {return 0;};
    int notify() {
        cout << "notify " << (notify_count++) << endl;
        return m_reactor.notify(this, ACE_Event_Handler::READ_MASK);
    };
};

void *reactor_wrapper(void *arg) {
    ACE_Reactor *reactor = (ACE_Reactor *)arg;

    reactor->owner(pthread_self());
    reactor->run_reactor_event_loop();
    return NULL;
}

void runtest(Handler *handler) {
    assert(handler->notify() >= 0);
    // carefully selected delay (1.5 * the delay introduced by the foo
flag)
    usleep(150 * 1000);
    assert(handler->notify() >= 0);
    assert(handler->notify() >= 0);
    // long enough to see any pending notifies delivered
    usleep(1000 * 1000);
}

int main() {
    pthread_t reactor_thread;
    Handler *handler = new Handler(reactor);

    pthread_create(&reactor_thread, NULL, reactor_wrapper, &reactor);
    assert(reactor.register_handler(handler,
ACE_Event_Handler::READ_MASK) >= 0);
    cout << "First test, probably won't deadlock" << endl;
    runtest(handler);
    cout << "Second test, probably will deadlock" << endl;
    foo = true; // expand the race condition window
    runtest(handler);
    if (notify_count > input_count)
        cout << "Deadlock detected: Sent " << notify_count
        << " notifications but only had " << input_count << " handle_input
calls made." << endl;

    assert(reactor.end_reactor_event_loop() >= 0);
    pthread_join(reactor_thread, 0);
    return 0;
}
