/***
 *      $Header: /usr/local/cvs/users/dws/egtao231/doctest/Hello.cpp,v 1.2 2005
/07/15 13:34:50 dws Exp $
 ***/

#include "HelloS.h"

#include "ace/Task.h"
#include "ace/Mutex.h"
#include "ace/streams.h"
#include "tao/corba.h"
#include "orbsvcs/PortableGroup/MIOP.h"
#include "orbsvcs/PortableGroup/GOA.h"

#define CLIENT_SLEEP_TIME       100     /* in milliseconds */
#define NB_HELLO_CLIENT_THREAD  8
#define NB_HELLO_CALLS          100

namespace Test
{
    class Hello_impl
    : public virtual POA_Test::Hello
    {
      public:
        void say_hello(CORBA::Short count) throw(CORBA::SystemException)
        {
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("   HELLO %d\n"),
                        count));
        };
    };
};

void
sleep(int millisec)
{
    struct timespec tv;
    tv.tv_sec = millisec / 1000;
    tv.tv_nsec = (millisec % 1000) * 1000000;
    struct timespec tv_not_elapsed;
    int status_nanosleep;
    while ((status_nanosleep = nanosleep(&tv, &tv_not_elapsed)) < 0
           && (errno == EINTR))
    {
        tv = tv_not_elapsed;
    }
};

class OrbRunThread
: public ACE_Task_Base
{
  public:
    OrbRunThread(CORBA::ORB_ptr orb)
    : m_orb(CORBA::ORB::_duplicate(orb))
    {
    };
    virtual int svc()
    {
        m_orb->run();
        return 1;
    };
  private:
    CORBA::ORB_var m_orb;
};

class HelloClientThread
: public ACE_Task_Base
{
  public:
    HelloClientThread(Test::Hello_ptr hello)
    : m_hello(Test::Hello::_duplicate(hello)),
      m_count(0)
    {
    };
    virtual int svc()
    {
        while (m_count < NB_HELLO_CALLS)
        {
            ACE_TRY_NEW_ENV
            {
                int count;
                {
                    // get your message number here
                    ACE_Guard < ACE_Mutex > guard(m_mutex);
                    count = m_count++;
                }
                ACE_DEBUG ((LM_DEBUG,
                            ACE_TEXT ("   ====> say_hello() %d\n"),
                            count));
                {
                    // send your message number here
#ifdef SEQUENCED_SAY_HELLO_REQUIRED
                    ACE_Guard < ACE_Mutex > guard(m_mutex);
#endif /* SEQUENCED_SAY_HELLO_REQUIRED */
                    m_hello->say_hello(count);
                }
                sleep(CLIENT_SLEEP_TIME);
            }
            ACE_CATCHANY
            {
                ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                     ACE_TEXT ("Exception thrown during say_hello()\n"));
            }
            ACE_ENDTRY;
        }
        return 1;
    };
  private:
    Test::Hello_var m_hello;
    int m_count;
    ACE_Mutex m_mutex;
};

int
main(int argc, char *argv[])
{
    CORBA::ORB_var orb = CORBA::ORB::_nil();
    int exit_code = 0;
    for (int i = 0; i < argc; ++i)
    {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s "),
                    argv[i]));
    }
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("\n")));

    OrbRunThread *orbThread = 0;
    Test::Hello_impl * hello_i = 0;

    ACE_TRY_NEW_ENV
    {
        /*** in svc.conf
dynamic UIPMC_Factory Service_Object * TAO_PortableGroup:_make_TAO_UIPMC_Protocol_Factory() ""
static Resource_Factory "-ORBProtocolFactory IIOP_Factory -ORBProtocolFactory UIPMC_Factory"
dynamic PortableGroup_Loader Service_Object * TAO_PortableGroup:_make_TAO_PortableGroup_Loader() ""
static Server_Strategy_Factory "-ORBConcurrency reactive -ORBPOALock thread -ORBAllowReactivationOfSystemids 1"
static Client_Strategy_Factory "-ORBProfileLock thread -ORBClientConnectionHandler MT"
         ***/

        // Initialize the ORB
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("   - ORB_init\n")));
        orb = CORBA::ORB_init(argc, argv);

        // Get the root POA 
        CORBA::Object_var obj_root =
            orb->resolve_initial_references("RootPOA");
        PortableGroup::GOA_var rootPOA =
            PortableGroup::GOA::_narrow(obj_root.in());

        // Activate the POA manager
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("   - POA manager activation\n")));
        PortableServer::POAManager_var poaManager = rootPOA->the_POAManager();

        poaManager->activate();

        // create Hello servant
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("   - create Hello servant\n")));
        hello_i = new Test::Hello_impl();

        // activate Hello Object
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("   - activate Hello object\n")));
        PortableServer::ObjectId_var obj_id =
            rootPOA->activate_object(hello_i);

        // create MIOP reference
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("   - create MIOP reference\n")));
        std::string multicast_addr =
            "corbaloc:miop:1.0@1.0-cdmwftdomain-1/225.1.1.8:5555";
        CORBA::Object_var miop_ref =
            orb->string_to_object(multicast_addr.c_str());

        // associate MIOP reference with Hello object 
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("   - associate MIOP reference with Hello object\n")));
        rootPOA->associate_reference_with_id(miop_ref.in(), obj_id);

        // create Hello reference
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("   - create Hello reference\n")));
        Test::Hello_var hello = Test::Hello::_unchecked_narrow(miop_ref.in());

        // start ORB run() in a thread
        orbThread = new OrbRunThread(orb.in());
        orbThread->activate(THR_NEW_LWP, 1);

        // create client threads
        HelloClientThread *helloThread = new HelloClientThread(hello.in());

        // start client threads
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("--------------------------------------------------\n")));
        helloThread->activate(THR_NEW_LWP, NB_HELLO_CLIENT_THREAD);
        helloThread->wait();
        delete helloThread;
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                             ACE_TEXT ("Exception thrown during main()\n"));
        exit_code = 1;
    }
    ACE_ENDTRY;
    if (orbThread != 0)
    {
        orb->shutdown(true);
        orbThread->wait();
        delete orbThread;
        delete hello_i;
    }
    if (!CORBA::is_nil(orb.in()))
    {
        ACE_TRY_NEW_ENV
        {
            orb->destroy();
        }
        ACE_CATCHANY
        {
            ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                         ACE_TEXT ("Exception thrown during orb check\n"));
            exit_code = 1;
        }
        ACE_ENDTRY;
    }
    exit(exit_code);
    return 0;
}
