#include <PortableServer/Object_Adapter.h>
#include <PortableServer/PortableServer.h>

using namespace std;

CORBA::ORB_var global_orb = CORBA::ORB::_nil();

int main(int argc, char* argv[])
{
	try 
	{
		// Initialize the ORB.
		//
		global_orb = CORBA::ORB_init(argc, argv);
		CORBA::Object_var poa_obj = global_orb->resolve_initial_references("RootPOA");
		PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_obj);

		// Create a PERSISTENT POA
		//
		// Create the policy list
		CORBA::PolicyList policies(2);
		policies.length(2);
		policies[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);
		policies[1] = root_poa->create_id_assignment_policy(PortableServer::USER_ID);

		// create the poa.
		// for ORBUseIMR=1 -ORBDefaultInitRef corbaloc:iiop:<HOST>:<PORT> 
		// this will cause an access violation if no IMR server is found.
		// The access violation WILL NOT HAPPEN if a POAManager is created and
		// given as a parameter to create_POA (instead of a nil POAManager)
		PortableServer::POA_var persistent_poa = 
			root_poa->create_POA("myPoa", PortableServer::POAManager::_nil(), policies);

		return 0;
	}
	catch (CORBA::Exception& e)
	{
		// ... normally print an error here
	}
	return 1;
}
