
#include <ace/Module.h>
#include <ace/Task.h>
#include <ace/Stream.h>


class Test_Task : public ACE_Task<ACE_SYNCH>
{
public:
    Test_Task( void ) :
        _destructorCalled(0)
    {
    }

    virtual ~Test_Task( void )
    {
        _destructorCalled++;
        if (_destructorCalled > 1)
            printf("Test_Task::~Test_Task() should be called once!!!\n");
    }

private:
    int _destructorCalled;
};


class Test_Module : public ACE_Module<ACE_SYNCH>
{
public:
    Test_Module( void )
    {
        this->open( ACE_TEXT("Test module"),
                    &_writerTask,
                    &_readerTask,
                    NULL,
                    M_DELETE_NONE );
    }
    
private:
    Test_Task _writerTask, _readerTask;
};


int ACE_TMAIN( int argc, ACE_TCHAR *argv[] )
{
    ACE_Stream<ACE_SYNCH> stream;
    
    if (stream.push(new Test_Module()) == -1)
    {
        printf("stream.push() failed\n");
    }
    
    return 0;
}
