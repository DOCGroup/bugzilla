#include "tao/corba.h"
#include "orbsvcs/Naming/Naming_Utils.h"

int main(int argc, char *argv[])
{
	char *args[] = { "Name Service",
			"-ORBEndpoint",
			"iiop://localhost:6000",
			"-ORBInitRef",
			"NameService=iiop://localhost:6000/NameService",
			"-ORBDebug",
			"-ORBDebugLevel",
			"10",
			};

	int i = sizeof(args) / sizeof(char *);

	ACE::debug(1);

	CORBA::ORB_var orb_var = CORBA::ORB_init(i, args, "");
	TAO_Naming_Server theNameService;

	ACE_DECLARE_NEW_CORBA_ENV;
	ACE_TRY
	{
		CORBA::Object_var obj = orb_var->resolve_initial_references("RootPOA", ACE_TRY_ENV);
		ACE_TRY_CHECK;

		PortableServer::POA_var root_poa_var = PortableServer::POA::_narrow(obj.in(), ACE_TRY_ENV);
		ACE_TRY_CHECK;

		PortableServer::POAManager_var mgr = root_poa_var->the_POAManager(ACE_TRY_ENV);
		ACE_TRY_CHECK;

		mgr->activate(ACE_TRY_ENV);
		ACE_TRY_CHECK;

		theNameService.init(orb_var.in(),
			root_poa_var.in(),
			ACE_DEFAULT_MAP_SIZE,
			0,
			0,
			0);


		CORBA::Object_var nsobj = orb_var->resolve_initial_references("NameService", ACE_TRY_ENV);
		ACE_TRY_CHECK;

		CosNaming::NamingContext_var ns_var = CosNaming::NamingContext::_narrow(nsobj.in(), ACE_TRY_ENV);
		ACE_TRY_CHECK;

		CosNaming::Name name;
		name.length (1);
		name[0].id = CORBA::string_dup("Foo");
		name[0].kind = CORBA::string_dup("");

		ns_var->bind (name, ns_var.in(), ACE_TRY_ENV);
		ACE_TRY_CHECK;

		CORBA::Object_var nsobj2 = ns_var->resolve(name, ACE_TRY_ENV);
		ACE_TRY_CHECK;

		CosNaming::NamingContext_var ns_var2 = CosNaming::NamingContext::_narrow(nsobj2.in(), ACE_TRY_ENV);
		ACE_TRY_CHECK;

	}
	ACE_CATCH (CORBA::Exception, e)
	{
		ACE_PRINT_EXCEPTION(e, "POA activate");
	}
	ACE_ENDTRY;

	orb_var->run();

	orb_var->destroy();

	return 0;
}
