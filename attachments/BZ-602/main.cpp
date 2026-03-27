
// The problem was a combination of calling TAO_Internal::open_services(...) 
// and passing command line arguments (containing the letter 'n') to my executable.
//
// Calling TAO_Internal::open_services(...) will call 
// ACE_Service_Config::parse_args(...) which will parse the command line arguments, 
// find an 'n' in there and will do this "ACE_Service_Config::no_static_svcs_ = 1;"
//
// This means there will be no Resource_Factory loaded, so "Resource_Factory 
// -ORBReactorType tp" will be ignored, and TAO will default to a select reactor.

#include <tao/corba.h>
#include <tao/TAO_Internal.h>
#include <string>

const std::string g_taoResourceFacArgs	= "static Resource_Factory \"-ORBReactorType tp\"";
const std::string g_taoSrvStrategyFac	= "static Server_Strategy_Factory \"-ORBConcurrency reactive\"";


//################################################################
// To see the bug run this progran with this command line argument
//   test.exe -n
//################################################################
int main(int argc, char* argv[])
{
	// this has to be called before ORB_init()
	TAO_Internal::default_svc_conf_entries(g_taoResourceFacArgs.c_str(),
										   g_taoSrvStrategyFac.c_str(),
										   0);

	// not sure why I had this line here, I think I saw it in a TAO example once?
	// this is where static_services are ignored if your command line arguments
	// contained the character 'n'  Look in the function 
	// ACE_Service_Config::parse_args() where there is this piece of code
	//       case 'n':
    //		    ACE_Service_Config::no_static_svcs_ = 1;
	//
	TAO_Internal::open_services(argc, argv, 1);

	// Solution: Don't call TAO_Internal::open_services(argc, argv, 1);  Why was I 
	// calling it in the first place??


	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

	return orb->run();
}