// EchoServer.cpp

#include "DefaultEcho_i.h"
#include <ace/streams.h>


int main(int argc, char *argv[])
{
  try {

    // initialize the ORB
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // Get the Root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj.in());

    // Create the policies for our child POA:
    //   LifespanPolicy: PERSISTENT
    //   IdAssignmentPolicy: USER_ID
    //   IdUniquenessPolicy: MULTIPLE_ID
    //   RequestProcessingPolicy: USE_DEFAULT_SERVANT
    //   ServantRetentionPolicy: NON_RETAIN
    //   ImplicitActivationPolicy: NO_IMPLICIT_ACTIVATION (defaults)
    PortableServer::LifespanPolicy_var persistent =
      root_poa->create_lifespan_policy(PortableServer::PERSISTENT);
    PortableServer::IdAssignmentPolicy_var user_id =
      root_poa->create_id_assignment_policy(PortableServer::USER_ID);
    PortableServer::IdUniquenessPolicy_var multiple_id =
      root_poa->create_id_uniqueness_policy(PortableServer::MULTIPLE_ID);
    PortableServer::RequestProcessingPolicy_var default_servant =
      root_poa->create_request_processing_policy(
        PortableServer::USE_DEFAULT_SERVANT);
    PortableServer::ServantRetentionPolicy_var non_retain =
      root_poa->create_servant_retention_policy(PortableServer::NON_RETAIN);

    // Create a policy list for our POAs
    CORBA::PolicyList policies;
    policies.length(5);
    policies[0] = PortableServer::LifespanPolicy::_duplicate(persistent.in());
    policies[1] = PortableServer::IdAssignmentPolicy::_duplicate(user_id.in());
    policies[2] = PortableServer::IdUniquenessPolicy::_duplicate(multiple_id.in());
    policies[3] = PortableServer::RequestProcessingPolicy::_duplicate(default_servant.in());
    policies[4] = PortableServer::ServantRetentionPolicy::_duplicate(non_retain.in());

    // Get the Root POA's POA Manager so that it can manage our child POA.
    PortableServer::POAManager_var poa_mgr = root_poa->the_POAManager();

    // Create the child POA.
    PortableServer::POA_var child_poa = 
      root_poa->create_POA("Echo POA", poa_mgr.in(), policies);

    // Create and set the DefaultServant on the child POA.
    DefaultEcho_i* default_echo = new DefaultEcho_i(child_poa.in());
    child_poa->set_servant(default_echo);

    // Destroy the POA policies; create_POA makes a copy.
    persistent->destroy();
    user_id->destroy();
    multiple_id->destroy();
    default_servant->destroy();
    non_retain->destroy();

    // Generate two object references, both of which will be incarnated
    // by our DefaultServant.  Stringify each one and write it to a file.
    PortableServer::ObjectId_var oid = 
      PortableServer::string_to_ObjectId("EchoObject_1");
    CORBA::Object_var obj1 = child_poa->create_reference_with_id(
      oid.in(), default_echo->_interface_repository_id());
    ofstream ior_file("EchoObject_1.ior");
    CORBA::String_var str = orb->object_to_string(obj1.in());
    ior_file << str.in() << endl;
    ior_file.close();
    cout << "EchoObject_1 written to EchoObject_1.ior" << endl;

    oid = PortableServer::string_to_ObjectId("EchoObject_2");
    CORBA::Object_var obj2 = child_poa->create_reference_with_id(
      oid.in(), default_echo->_interface_repository_id());
    ior_file.open("EchoObject_2.ior");
    str = orb->object_to_string(obj2.in());
    ior_file << str.in() << endl;
    ior_file.close();
    cout << "EchoObject_2 written to EchoObject_2.ior" << endl;

    // Activate POAs.
    poa_mgr->activate();
    
    // Run the ORB's event loop.
    orb->run();
  }
  catch(CORBA::SystemException &se) {
    cerr << argv[0] << ": Unexpected exception " << endl << se << endl;
  }
  catch (CORBA::UserException &ue) {
	cerr << "Unknown user exception caught " << endl << ue << endl;
  }
  catch(...) {
    cerr << argv[0] << ": Unknown exception " << endl;
  }

 return 0;
}
