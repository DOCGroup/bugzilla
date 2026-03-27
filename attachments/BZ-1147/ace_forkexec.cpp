#include <ace/ACE.h>
#include <ace/OS_NS_sys_wait.h>
#include <ace/Log_Msg.h>
#include <string>

using namespace std;

enum
{
    SUCCESS = 0,
    FAILURE
};

class Static
{
public:
    Static() : m_exit_in_destructor(true) {}

    ~Static()
    {
        if (m_exit_in_destructor)
        {
            ACE_DEBUG((LM_DEBUG,
                       ACE_TEXT ("destructor of static object called in child\n")));

            ACE_OS::_exit(0);
        }
    }

    bool m_exit_in_destructor;
};


// the destructor of this static object should not be called
// in the forked child even if execv() fails
static Static staticObject;

int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);


    // trying to fork_exec this will always fail
    ACE_TCHAR command = ACE_TEXT('\0');
    ACE_TCHAR* fork_argv[] = {&command, NULL};

    pid_t pid = ACE_OS::fork_exec(fork_argv);
    // fork_exec returns an error only if fork() fails;
    // execv() failures show up in child's exit status
    ACE_ASSERT(pid > 0);

    ACE_exitcode status;
    pid = ACE_OS::waitpid(pid, &status, 0);
    ACE_ASSERT(pid > 0);

    // don't foul up parent's exit routine
    staticObject.m_exit_in_destructor = false;

    if (!WIFEXITED(status))
    {
        ACE_ERROR((LM_DEBUG,
                   ACE_TEXT ("Child exit with no status\n")));
        return FAILURE;
    }

    int child_exit_code = WEXITSTATUS(status);
    if (child_exit_code == 0)
    {
        // if exec part of fork_exec() fails, it should
        // return errno as child's exit code;
        // 0 exit status will only be returned if staticObject's
        // destructor was called in the child.

        ACE_ERROR((LM_DEBUG,
                   ACE_TEXT ("fork_exec() calls exit() instead of _exit() on exec error\n"),
                   child_exit_code, status, ENOENT));
        return FAILURE;
    }

    return SUCCESS;
}
