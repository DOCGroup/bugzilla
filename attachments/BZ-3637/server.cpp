#include "CorbaSimpleS.h"
#include <tao/IORTable/IORTable.h>
#include <iostream>
#include <ace/OS.h>

class  CorbaSimple_i :
	public virtual PortableServer::RefCountServantBase,
	public virtual POA_CorbaSimple

{
public:
	CorbaSimple_i(CORBA::ORB_ptr orb) : _orb(CORBA::ORB::_duplicate(orb)) {}

	CORBA::Long hello(CORBA::Long i)
		ACE_THROW_SPEC ((CORBA::SystemException))
	{
		std::cout << "SERVER: "
			  << ACE_OS::getpid()
			  << " Executing CorbaSimple::hello()" << std::endl;
		_orb->shutdown(false);
		return i;
	}
private:
	CORBA::ORB_var _orb;
};

int main (int argc, char* argv[])
{
	try
	{
		CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, 0);

		CORBA::Object_var root_poa_o =
			orb->resolve_initial_references ("RootPOA");
		PortableServer::POA_var rootPoa;
		rootPoa = PortableServer::POA::_narrow (root_poa_o.in ());
		if (CORBA::is_nil (rootPoa.in()))
		{
			std::cerr << "ROOT POA IS NIL!" << std::endl;
			return -1;
		}

		PortableServer::POAManager_var poaMgr;
		poaMgr = rootPoa->the_POAManager ();
		poaMgr->activate ();

                CORBA::PolicyList policies;
                policies.length (2);
                policies[0] = rootPoa->create_id_assignment_policy (
                        PortableServer::USER_ID);
                policies[1] = rootPoa->create_lifespan_policy (
                        PortableServer::PERSISTENT);

		PortableServer::POA_var poa = rootPoa->create_POA (
                        "CorbaSimple_POA", poaMgr.in (), policies );

		for (CORBA::ULong i = 0; i < policies.length (); ++i)
		{
                        policies[i]->destroy ();
                }

		CorbaSimple_i * servant;
		PortableServer::ServantBase_var v = servant =
			new CorbaSimple_i(orb);

		PortableServer::ObjectId_var oid =
			PortableServer::string_to_ObjectId ("CorbaSimple");

		poa->activate_object_with_id (oid.in(), servant);

		CORBA::Object_var obj = poa->id_to_reference (oid.in ());

		CORBA::String_var str = orb->object_to_string (obj.in());

		CORBA::Object_var tobj =
			orb->resolve_initial_references("IORTable");
		IORTable::Table_var table =
			IORTable::Table::_narrow (tobj.in ());

		table->bind("CorbaSimple", str);

		orb->run();
	}
	catch (CORBA::Exception& ex)
	{
		std::cerr << ex << std::endl;
		return -1;
	}
	catch (std::exception& ex)
	{
		std::cerr << "std::exception: " << ex.what () << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cerr << "unknown exception" << std::endl;
		return -1;
	}

	return 0;
}
