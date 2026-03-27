
#include <string>
#include <ace/Task.h>
#include <tao/corba.h>


// ************************************************************************** //

class ORBWorker : public ACE_Task_Base {
  public:
    ORBWorker (CORBA::ORB_ptr orb) :  mOrb (CORBA::ORB::_duplicate (orb)) { }
    virtual int svc (void);
  private:
    CORBA::ORB_var mOrb;
};

inline int ORBWorker::svc (void) {
  mOrb->run();
  return 0;
}

// ************************************************************************** //

class MasterServer {
  public:
    MasterServer (char* argv0, string name, string end_point,
                  string debug = "0");
    ~MasterServer();
  private:
    ORBWorker*                     mWorker;
    // ORB and related
    CORBA::ORB_var                 mORB;
    PortableServer::POA_var        mRootPOA;
    PortableServer::POA_var        mPOA;
    PortableServer::POAManager_var mMgr;
};

MasterServer::MasterServer (char* argv0, string end_point, string name,
                            string debug) {
  int orb_threads = 1;
  // *** ORB Initialization
  int   argc = 5;
  char* argv[5] = { CORBA::string_dup (argv0),
                    CORBA::string_dup ("-ORBEndPoint"),
                    CORBA::string_dup (end_point.c_str()),
                    CORBA::string_dup ("-ORBDebugLevel"),
                    CORBA::string_dup (debug.c_str())     };
  mORB = CORBA::ORB_init (argc, argv, name.c_str());
  // *** POA Initialization
  CORBA::Object_var obj;
  obj = mORB->resolve_initial_references ("RootPOA");
  mRootPOA = PortableServer::POA::_narrow (obj);
  PortableServer::LifespanPolicy_var persistent =
    mRootPOA->create_lifespan_policy (PortableServer::PERSISTENT);
  PortableServer::IdAssignmentPolicy_var user_id =
    mRootPOA->create_id_assignment_policy (PortableServer::USER_ID);
  CORBA::PolicyList policies;
  policies.length(2);
  policies[0] = persistent.in();
  policies[1] = user_id.in();
  mMgr = mRootPOA->the_POAManager();
  string nom_poa = name + "POA";
  mPOA = mRootPOA->create_POA (nom_poa.c_str(), mMgr, policies);
  persistent->destroy();
  user_id->destroy();
  mMgr->activate();
  // Creacion de los trabajadores
  mWorker = new ORBWorker (mORB.in());
  mWorker->activate (THR_NEW_LWP | THR_JOINABLE, orb_threads);
}

MasterServer::~MasterServer() {
  mPOA->destroy (true, true);
  mRootPOA->destroy (true, true);
  mORB->shutdown (false);
  mORB->destroy();
  delete mWorker;
}

// ************************************************************************** //

int main (int argc, char** argv) {
  MasterServer* ms1;
  MasterServer* ms2;
  try {
    ms1 = new MasterServer (argv[0], "iiop://soporte.fi.uba.ar:6666", "Master1");
    ms2 = new MasterServer (argv[0], "uiop://Config.pipe", "Master2", "10");
  } catch (CORBA::Exception& ex) {
    ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "There was an exception");
    return 1;
  }
  
  ACE_OS::sleep (3);
  
  delete ms1;
  delete ms2;
  return 0;
}
