#include "MyInterfaceImpl.h"
#include "TestS.h"
#include "ace/Get_Opt.h"
#include "ace/OS_NS_stdio.h"

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  try
    {
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv);

      CORBA::Object_var poa_object =
        orb->resolve_initial_references("RootPOA");

      PortableServer::POA_var root_poa =
        PortableServer::POA::_narrow (poa_object.in ());

      if (CORBA::is_nil (root_poa.in ()))
        ACE_ERROR_RETURN ((LM_ERROR,
                           " (%P|%t) Panic: nil RootPOA\n"),
                          1);

      PortableServer::POAManager_var poa_manager =
        root_poa->the_POAManager ();

      MyInterfaceImpl *test_impl = new MyInterfaceImpl;

      PortableServer::ServantBase_var owner_transfer(test_impl);

      PortableServer::ObjectId_var id = root_poa->activate_object (test_impl);

      CORBA::Object_var object = root_poa->id_to_reference (id.in ());

      MyInterface_var test_ref = MyInterface::_narrow (object.in ());

      CORBA::String_var ior = orb->object_to_string (test_ref.in());

      // Output the IOR to the <ior_output_file>
      FILE *output_file= ACE_OS::fopen ("server.ior", "w");
      if (output_file != 0)
        {
          ACE_OS::fprintf (output_file, "%s", ior.in ());
          ACE_OS::fclose (output_file);
        }

       ACE_DEBUG ((LM_DEBUG, "(%P|%t) server - event loop finished\n"));

      root_poa->destroy (1, 1);

      orb->destroy ();
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("Exception caught:");
      return 1;
    }

  return 0;
}
