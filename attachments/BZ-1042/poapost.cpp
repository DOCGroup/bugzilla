#include "smallS.h"

// Demonstrate a POA problem.
//
// The spec says:
//
// The postinvoke operation is considered to be part of a request on an object.That is,
// the request is not complete until postinvoke finishes. If the method finishes normally
// but postinvoke raises a system exception, the method's normal return is overridden; the
// request completes with the exception.
//
// But in TAO the reply is actually sent to the client before postinvoke() is called
// and the exception is ignored.
//
// Usage:
// C:\ start poapost server
// C:\ poapost client
//
// The client will say:
// FAIL -- exception not thrown
//
// The server will say:
// server ready
// test(0)
// test(1)
//
// The source for "small.idl" is:
//
// interface small
// {
// 	void test( in boolean raise_exception );
// } ;

CORBA::ORB_ptr orb;
PortableServer::POA_ptr poa;
PortableServer::POA_ptr test_poa;

CORBA::Boolean raise_on_post;

class SmallImpl : public POA_small
{
public:
	SmallImpl()
	{
	}

	virtual void test ( CORBA::Boolean raise_exception )
	{
		fprintf( stderr, "test(%d)\n", raise_exception );
		raise_on_post = raise_exception;
	}
} ;

void writeior( const char* fname, const char* ior )
{
	FILE* f = fopen( fname, "w" );
	if ( f == NULL )
	{
		fprintf( stderr, "can't open %s for writing\n", fname );
		return;
	}
	fwrite( (char*) ior, 1, strlen( (char*) ior ), f );
	fclose( f );
}

void readior( const char* fname, char*& res )
{
	FILE* f = fopen( fname, "r" );
	if ( f == NULL )
	{
		fprintf( stderr, "can't open %s for reading\n", fname );
		return;
	}

	res = new char[ 4096 ];

	fgets( res, 4000, f );

	char *p;
	for ( p = res; *p; p++ )
	{
		if ( *p == '\n' )
		{
			*p = 0;
		}
	}

	fclose( f );
}

// Servant Locator
class TestLocator : public PortableServer::ServantLocator
{
public:
    TestLocator (){}
	
    PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
									  PortableServer::POA_ptr adapter,
									  const char* operation,
									  PortableServer::ServantLocator::Cookie& the_cookie)
	{
		return &theServant;
	}
	
    void postinvoke(const PortableServer::ObjectId& oid,
                    PortableServer::POA_ptr adapter,
                    const char* operation,
                    PortableServer::ServantLocator::Cookie the_cookie,
	                    PortableServer::ServantBase* the_servant)
	{
		if ( raise_on_post )
		{
			throw CORBA::OBJECT_NOT_EXIST();
		}
	}

private:
	SmallImpl theServant;
};

int main( int argc, char* argv[] )
{
	raise_on_post = false;

	orb = CORBA::ORB_init( argc, argv );

	char *role = argv[ 1 ];

	if ( strcmp( role, "server" ) == 0 )
	{
		// server
	    CORBA::Object_var poaWide = orb->resolve_initial_references( "RootPOA" );
		poa = PortableServer::POA::_narrow( poaWide );

		CORBA::PolicyList policies;
		policies.length(3);   
		policies[(CORBA::ULong)0] = poa->create_servant_retention_policy(PortableServer::NON_RETAIN);
		policies[(CORBA::ULong)1] = poa->create_request_processing_policy(PortableServer::USE_SERVANT_MANAGER);
		policies[(CORBA::ULong)2] = poa->create_id_assignment_policy( PortableServer::USER_ID );

		PortableServer::POAManager_var nilMgr = PortableServer::POAManager::_nil();
		test_poa = poa->create_POA( "test_poa", nilMgr, policies); 

		TestLocator* testLocator = new TestLocator;

		test_poa->set_servant_manager( testLocator );

		PortableServer::POAManager_var poaManager;

		poaManager = poa->the_POAManager();
		poaManager->activate();

		poaManager = test_poa->the_POAManager();
		poaManager->activate();

		SmallImpl* reboundServant = new SmallImpl();

		PortableServer::ObjectId_var objID;

		objID = PortableServer::string_to_ObjectId( "reference" );

		CORBA::Object_var objRef;
		objRef = test_poa->create_reference_with_id( objID, "IDL:small:1.0" );

		CORBA::String_var ior( orb->object_to_string( objRef ) );

		writeior( "server.ior", ior.in() );
		fprintf( stderr, "server ready\n" );

		orb->run();
	}
	else
	{
		CORBA::String_var ior;

		// The client
		readior( "server.ior", ior.out() );

		CORBA::Object_var wide;

		wide = orb->string_to_object( ior.in() );

		small_var obj = small::_narrow( wide );

		obj->test( 0 );

		try
		{
			obj->test(1);
			fprintf( stderr, "FAIL -- exception not thrown\n" );
		}
		catch ( const CORBA::OBJECT_NOT_EXIST& )
		{
			fprintf( stderr, "success\n" );
		}
	}

	return 0;
}
