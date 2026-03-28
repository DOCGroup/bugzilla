#include <iostream>
#include <corba.h>
#include <orbsvcs/CosNamingC.h>
#include <tao/Messaging/Messaging.h>

CORBA::ORB_var orb;

void* threadfunc( void* arg )
{
    while( true )
    {
       try
        {
            CORBA::Object_var ncRef =
                orb->string_to_object( 
                    "corbaloc:iiop:10.175.12.99:15025/NameService" );

	    CORBA::PolicyList policies;

	    TimeBase::TimeT timeout = 5000 * 10000;

	    CORBA::Any timeoutAny;
	    timeoutAny <<= timeout;

	    policies.length(1);
	    policies[0] = orb->create_policy(
	        Messaging::RELATIVE_RT_TIMEOUT_POLICY_TYPE,
	        timeoutAny );

	    CORBA::Object_var object = ncRef->_set_policy_overrides(
	        policies, CORBA::SET_OVERRIDE );

	    policies[0]->destroy();
  
            CosNaming::NamingContext_var namingContext =
    	        CosNaming::NamingContext::_narrow( object.in() );
            namingContext->_non_existent();
        }
        catch( const CORBA::Exception& e )
        {
            std::cout << "Caught exception " << e << std::endl;
        }
    }
}
            

int main( int argc, char* argv[] )
{
    orb = CORBA::ORB_init( argc, argv );
    
    for( int i=0; i<10; i++ )
    {
        pthread_t thread;
        pthread_create( &thread, NULL, &threadfunc, NULL );
    }
    orb->run();
    return 0;
}
