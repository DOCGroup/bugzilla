
//=============================================================================
/**
 *  @file    client.cpp
 *
 *  $Id: client.cpp 84563 2009-02-23 08:13:54Z johnnyw $
 *
 *  A client which uses the AMI callback model.
 *
 *
 *  @author Alexander Babu Arulanthu <alex@cs.wustl.edu>
 *  @author Michael Kircher <Michael.Kircher@mchp.siemens.de>
 */
//=============================================================================


#include "ace/Get_Opt.h"
#include "ace/Task.h"
#include "ami_test_i.h"
#include <string.h>  // for strlen and strcpy, duplicating cmdline params
#include <pthread.h>  // for pthread_self() and pthread_t

ACE_RCSID (AMI,
           client,
           "$Id: client.cpp 84563 2009-02-23 08:13:54Z johnnyw $")

const ACE_TCHAR *ior = ACE_TEXT("file://test.ior");
int nthreads = 5;
int niterations = 5;
int debug = 0;
volatile int number_of_replies = 0;

CORBA::Long in_number = 931232;
const char * in_str = "Let's talk AMI.";
int parameter_corruption = 0;
int incorrect_thread = 0;
pthread_t main_thread;

int
parse_args (int argc, ACE_TCHAR *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, ACE_TEXT("dk:n:i:"));
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'd':
        debug = 1;
        break;
      case 'k':
        ior = get_opts.opt_arg ();
        break;
      case 'n':
        nthreads = ACE_OS::atoi (get_opts.opt_arg ());
        break;
      case 'i':
        niterations = ACE_OS::atoi (get_opts.opt_arg ());
        break;
      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s "
                           "-d "
                           "-k <ior> "
                           "-n <nthreads> "
                           "-i <niterations> "
                           "\n",
                           argv [0]),
                          -1);
      }
  // Indicates sucessful parsing of the command line
  return 0;
}


/**
 * @class Client
 *
 * @brief Run the client thread
 *
 * Use the ACE_Task_Base class to run the client threads.
 */
class Client : public ACE_Task_Base
{
public:
  /// ctor
  Client (A::AMI_Test_ptr server,
          int niterations,
          A::AMI_AMI_TestHandler_ptr the_handler);

  /// The thread entry point.
  virtual int svc (void);

  // private:
  /// Var for the AMI_Test object.
  A::AMI_Test_var ami_test_var_;

  /// The number of iterations on each client thread.
  int niterations_;

  /// Var for AMI_AMI_Test_ReplyHandler object.
  A::AMI_AMI_TestHandler_var the_handler_var_;
};

/**
 * @class ClientWorker
 *
 * @brief Run perform_work to handle replies.
 *
 * Use the ACE_Task_Base class to run perform_work on the client ORB, so that
 * it can handle replies to the requests clients make.
 */
class ClientWorker : public ACE_Task_Base
{
public:
  /// ctor
  ClientWorker (CORBA::ORB_ptr client_orb);

  /// The thread entry point.
  virtual int svc (void);

  /// Client ORB var
  CORBA::ORB_var client_orb_var_;

  /// Flag to indicate the ClientWorker should quit. 
  volatile bool quitting_;
};


class Handler : public POA_A::AMI_AMI_TestHandler
{
private:
  void check_thread (void) const
    {
      if (main_thread != pthread_self())
        {
          ACE_ERROR((LM_ERROR, "ERROR: Callback method was not invoked from the main server thread!\n")); 
          incorrect_thread = 1;
        }
    }
public:
  Handler (void)
  {
  };

  void foo (CORBA::Long result,
            CORBA::Long out_l)
    {
      check_thread();
      if (result == 0)
        {
          ACE_ERROR((LM_ERROR, "ERROR: Callback method detected parameter corruption.\n"));
          parameter_corruption = 1;
        }

      if (debug)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "(%P | %t) : Callback method called: result <%d>, out_arg <%d>\n",
                      result,
                      out_l));
        }

      --number_of_replies;
    };

   void foo_excep (::Messaging::ExceptionHolder * excep_holder)
    {
      check_thread();

      ACE_DEBUG ((LM_DEBUG,
                  "Callback method <foo_excep> called:\n"));
      try
        {
          excep_holder->raise_exception ();
        }
      catch (const CORBA::Exception& ex)
        {
          ex._tao_print_exception ("Caught exception:");
        }
    };

  void get_yadda (CORBA::Long result)
    {
      check_thread();
      ACE_DEBUG ((LM_DEBUG,
                  "Callback method <get_yadda> called: result <%d>\n",
                  result));
    };

  void get_yadda_excep (::Messaging::ExceptionHolder *)
    {
      check_thread();
      ACE_DEBUG ((LM_DEBUG,
                  "Callback method <get_yadda_excep> called:\n"));
    };

  void set_yadda (void)
    {
      check_thread();
      ACE_DEBUG ((LM_DEBUG,
                  "Callback method <set_yadda> called:\n"));
    };

  void set_yadda_excep (::Messaging::ExceptionHolder *)
    {
      check_thread();
      ACE_DEBUG ((LM_DEBUG,
                  "Callback method <set_yadda_excep> called:\n"));
    };
  ~Handler (void)
  {
  };

  void inout_arg_test (const char *)
  {
    check_thread();
    ACE_DEBUG ((LM_DEBUG,
                "Callback method <set_yadda_excep> called:\n"));
  }

  void inout_arg_test_excep (::Messaging::ExceptionHolder *)
  {
    check_thread();
  }
};

// ReplyHandler.
Handler handler;

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  // Make main_thread point to the original thread, for later reference.
  main_thread = pthread_self();
  try
    {
      // copy the parameters, since the first ORB will
      // consume them.
      int argc_copy = argc;
      ACE_TCHAR **argv_copy = new ACE_TCHAR*[argc];
      for (int i = 0; i < argc; ++i) {
        argv_copy[i] = new ACE_TCHAR[strlen(argv[i])+1];
        strcpy(argv_copy[i], argv[i]);
      }

      CORBA::ORB_var client_orb =
        CORBA::ORB_init (argc, argv, "client");
      CORBA::ORB_var server_orb =
        CORBA::ORB_init (argc_copy, argv_copy, "server");

      if (parse_args (argc, argv) != 0)
        return 1;

      A::AMI_Test_var server;

      CORBA::Object_var object =
        client_orb->string_to_object (ior);
      server =  A::AMI_Test::_narrow (object.in ());

      if (CORBA::is_nil (server.in ()))
        {
          ACE_ERROR_RETURN ((LM_ERROR,
                             "Object reference <%s> is nil.\n",
                             ior),
                            1);
        }

      // Activate POA to handle the call back.

      CORBA::Object_var poa_object =
        server_orb->resolve_initial_references("RootPOA");

      if (CORBA::is_nil (poa_object.in ()))
        ACE_ERROR_RETURN ((LM_ERROR,
                           " (%P|%t) Unable to initialize the POA.\n"),
                          1);

      PortableServer::POA_var root_poa =
        PortableServer::POA::_narrow (poa_object.in ());

      PortableServer::POAManager_var poa_manager =
        root_poa->the_POAManager ();

      poa_manager->activate ();

      // Make sure that the handler is activated with the server
      // ORB via its POA.
      PortableServer::ObjectId_var oid = root_poa->activate_object(&handler);
      CORBA::Object_var tempobj = root_poa->id_to_reference(oid.in());
      CORBA::String_var handler_ior = server_orb->object_to_string(tempobj.in());
      tempobj = client_orb->string_to_object(handler_ior.in());
      A::AMI_AMI_TestHandler_var handler_var = A::AMI_AMI_TestHandler::_narrow(tempobj.in()); 

      // Let the client perform the test in a separate thread 
      Client client (server.in (), niterations, handler_var.in());
      if (client.activate (THR_NEW_LWP | THR_JOINABLE,
                           nthreads) != 0)
        ACE_ERROR_RETURN ((LM_ERROR,
                           "Cannot activate client threads\n"),
                          1);

      // Fire up a thread to call perform_work on the client ORB.
      ClientWorker worker (client_orb.in ());
      if (worker.activate (THR_NEW_LWP | THR_JOINABLE) != 0)
        ACE_ERROR_RETURN ((LM_ERROR,
                           "Cannot activate worker thread\n"),
                          1);
      

      // Main thread collects replies. It needs to collect
      // <nthreads*niterations> replies.
      number_of_replies = nthreads *niterations;

      if (debug)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "(%P|%t) : Entering perform_work loop to receive <%d> replies\n",
                      number_of_replies));
        }

      // ORB loop.

      while (number_of_replies > 0)
        {
          CORBA::Boolean pending = server_orb->work_pending();

          if (pending)
            {
              server_orb->perform_work();
            }
        }

      if (debug)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "(%P|%t) : Exited perform_work loop Received <%d> replies\n",
                      (nthreads*niterations) - number_of_replies));
        }

      // Tell the worker thread to quit.
      worker.quitting_ = true;
      worker.thr_mgr ()->wait ();
      client.thr_mgr ()->wait ();

      ACE_DEBUG ((LM_DEBUG, "threads finished\n"));

      root_poa->destroy (1,  // ethernalize objects
                         0  // wait for completion
                        );

      server_orb->destroy ();
      client_orb->destroy ();
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("Caught exception:");
      return 1;
    }

  return parameter_corruption + incorrect_thread;
}

// ****************************************************************

Client::Client (A::AMI_Test_ptr server,
                int niterations,
                A::AMI_AMI_TestHandler_ptr the_handler)
                :  ami_test_var_ (A::AMI_Test::_duplicate (server)),
     niterations_ (niterations),
     the_handler_var_ (A::AMI_AMI_TestHandler::_duplicate(the_handler))
{ 
}

int
Client::svc (void)
{
  try
    {
      for (int i = 0; i < this->niterations_; ++i)
        {
          ami_test_var_->sendc_foo (the_handler_var_.in (), in_number, in_str);
        }
      if (debug)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "(%P | %t):<%d> Asynchronous methods issued\n",
                      niterations));
        }
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("MT_Client: exception raised");
    }
  return 0;
}

// ****************************************************************

ClientWorker::ClientWorker (CORBA::ORB_ptr client_orb)
  : client_orb_var_ (CORBA::ORB::_duplicate(client_orb)),
    quitting_(false) 
{
}

int
ClientWorker::svc (void)
{
  try
    {
      while (!quitting_)
        {
          CORBA::Boolean pending = client_orb_var_->work_pending(); 
          if (pending)
            {
              client_orb_var_->perform_work();
            }
        }
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("MT_Client: exception raised");
    }
  return 0; 
}
