#include <ace/Thread.h>

/**
* used in 'assert' to print error explanation
*/ 
bool check(bool x, char* s) {
    if (!x) {
        perror(s);
    }
    return x;
}

/**
* Just spend some time
* Due to user level threads (as opposite to kernel) in HP,
* some yield mechanism should be used to assist cooperative multitasking
*/
void do_nothing() {
    for (int i=0;i<10;++i) {
        ACE_Thread::yield();
    }
}

/**
* Master thread prints its ID  
* and clean up memory allocated for its argument
*/
void* thr_master(void* arg) {
    int id=*(int*)arg;
    delete (int*)arg;
    printf("<<%d|",id);
    fflush(stdout);
    do_nothing();
    return NULL;
}

/**
* Control thread waits for master thread termination 
* and clean up memory allocated for its argument
*/
void* thr_control(void* arg) {
    ACE_thread_t* t=(ACE_thread_t*)arg;
    assert(check(0==ACE_Thread::join(*t),"join"));
    printf(">>");
    fflush(stdout);
    delete t;
    return NULL;
}

void run2threads(int id) {
    /**
    * Arguments for threads allocated on heap,
    * thread exists when this routine returns and local variables
    * gone away.
    */
    // t deleted in control thread
    ACE_thread_t* t=new ACE_thread_t;
    {
        // arg deleted in master thread
        int* arg=new int(id);
        // create master thread, joinable
        assert(check(0==ACE_Thread::spawn(thr_master,arg,THR_NEW_LWP|THR_JOINABLE,t),"create master"));
    }
    // create control thread, detached.
    assert(check(0==ACE_Thread::spawn(thr_control,t,THR_NEW_LWP|THR_DETACHED),"create control"));
}

int main(int argc, char* argv[]) {
    int i=0;
    for (;;) {
        run2threads(++i);
        printf("(%d)",i);
        fflush(stdout);
        do_nothing();
    }
}

