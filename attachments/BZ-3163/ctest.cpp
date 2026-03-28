#include <ace/OS.h>
#include <tao/corba.h>
#include <tao/PortableServer/PortableServer.h>
#include <iostream>
#include "corbarpc/ctestS.h"

using namespace std;

class ctest_impl : public virtual POA_ctest
{
public:
	CORBA::Long ctestfn(CORBA::Long size, ctest::UCSeq_out data)
	{
		cout << "ctestfn called for " << size << " objects" << endl;

		data = new ctest::UCSeq;

		data->length(size);

		for (size_t i = 0; i < data->length(); ++i)
        	{
			data[i] = (rand() % 26) + 'A';
		}

		return 0;
	}
};

int main(int argc, char * argv[])
{
  try
  {
  	if ((argc == 1) || (argc > 3))
  	{
		cout << "Server Mode" << endl;

  		CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

  		CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");

  		PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

  		PortableServer::POAManager_var mgr = poa->the_POAManager();

  		mgr->activate();

  		ctest_impl servant;

  		ctest_var object = servant._this();

  		CORBA::String_var str = orb->object_to_string(object);

  		cout << str << endl;

  		orb->run();
  	}
  	else
  	{
		cout << "Client Mode" << endl;

		CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

		CORBA::Object_var obj = orb->string_to_object(argv[1]);

		if (CORBA::is_nil(obj))
		{
			cout << "Nil reference" << endl;
			return EXIT_FAILURE;
		} 

		ctest_var ctestVar;

		ctestVar = ctest::_narrow(obj);

		cout << "Calling ctestfn" << endl;

		ctest::UCSeq_var data;

		ctestVar->ctestfn(ACE_OS::atoi(argv[2]), data);

		cout << "Received " << data->length() << " bytes" << endl;
  	}

  	ACE_OS::sleep(10);
  }
  catch(...)
  {
	cout << "An exception" << endl;
  }

  return EXIT_SUCCESS;
}

