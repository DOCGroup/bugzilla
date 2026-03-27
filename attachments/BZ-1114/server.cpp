// server.cpp,v 1.16 2001/09/18 00:06:00 irfan Exp

#include "testS.h"
#include "ace/Get_Opt.h"

class Test_i : public POA_Test
{
  // = TITLE
  //   An implementation for the Test interface in test.idl
  //
public:

  Test_i (const char * name);

  // = The Test methods.
  void test_method (const char *message,
                    CORBA::Environment&)
    ACE_THROW_SPEC ((CORBA::SystemException));

private:

  const char * name_;
};

Test_i::Test_i (const char *name)
  : name_ (name)
{
}

void
Test_i::test_method (const char *message,
                     CORBA::Environment &)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  ACE_DEBUG ((LM_DEBUG,
              "Server %s received message: %s\n",
              this->name_, message));

}

//*************************************************************************

const char *ior_output_file = "test.ior";
const char *object_id_file = "test.id";
const char *object_id_in_file = 0;

// Parse command-line arguments.
int
parse_args (int argc, char *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, "o:i:");
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'i':
        object_id_in_file = get_opts.optarg;
        break;

      case 'o':
        ior_output_file = get_opts.optarg;
        break;

      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s "
                           "-o <iorfile>"
                           "-i <objectid>"
                           "\n",
                           argv [0]),
                          -1);
      }

  return 0;
}

int
main (int argc, char *argv[])
{
  ACE_TRY_NEW_ENV
    {
      // Standard initialization.
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv, "", ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (parse_args (argc, argv) != 0)
        return 1;

      CORBA::Object_var object =
        orb->resolve_initial_references("RootPOA", ACE_TRY_ENV);
      ACE_TRY_CHECK;

      PortableServer::POA_var root_poa =
        PortableServer::POA::_narrow (object.in (), ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (CORBA::is_nil (root_poa.in ()))
        ACE_ERROR_RETURN ((LM_ERROR,
                           "ERROR: Panic <RootPOA> is nil\n"),
                          -1);

      PortableServer::POAManager_var poa_manager =
        root_poa->the_POAManager (ACE_TRY_ENV);
      ACE_TRY_CHECK;

      // Create POA with PERSISTENT and SYSTEMID policies.
      CORBA::PolicyList poa_policy_list;
      poa_policy_list.length (2);
      poa_policy_list[0] =
        root_poa->create_lifespan_policy (PortableServer::PERSISTENT,
                                          ACE_TRY_ENV);
      ACE_TRY_CHECK;

      poa_policy_list[1] =
        root_poa->create_id_assignment_policy (PortableServer::SYSTEM_ID,
                                               ACE_TRY_ENV);
      ACE_TRY_CHECK;


      PortableServer::POA_var child_poa =
        root_poa->create_POA ("Child_POA",
                              poa_manager.in (),
                              poa_policy_list,
                              ACE_TRY_ENV);
      ACE_TRY_CHECK;



      //////////////////////////////////////////////////////////////////////
      Test_i server_impl1 ("OLD");
      Test_i server_impl2 ("NEW");
      
      // Recreate old object if id is provided.
      PortableServer::ObjectId id;
      if (object_id_in_file != 0)
        {
          ACE_HANDLE id_input_file = ACE_OS::open (object_id_in_file, O_RDONLY);
          if (id_input_file == ACE_INVALID_HANDLE)
            ACE_ERROR_RETURN ((LM_ERROR,
                               "Cannot open output file for reading Id: %s",
                               object_id_in_file),
                              1);

          char buf[256];
          CORBA::ULong len = 0;
          if (ACE_OS::read (id_input_file, &len, sizeof (len)) == -1
              || ACE_OS::read (id_input_file, buf, len) == -1)
            ACE_ERROR_RETURN ((LM_ERROR,
                               "Error reading object id\n"),
                              1);
          ACE_OS::close (id_input_file);

          id.replace (len,
                      len,
                      (CORBA::Octet*)buf,
                      0);

          child_poa->activate_object_with_id (id, 
                                              &server_impl1,
                                              ACE_TRY_ENV);
          ACE_TRY_CHECK;
        }

      else
        {
          // Create a new object.
          PortableServer::ObjectId_var new_id =
            child_poa->activate_object (&server_impl2, 
                                        ACE_TRY_ENV);
          ACE_TRY_CHECK;

          ACE_HANDLE id_output_file= ACE_OS::open (object_id_file, 
                                                   O_RDWR|O_CREAT|O_EXCL, 
                                                   S_IRUSR|S_IWUSR);
          if (id_output_file == ACE_INVALID_HANDLE)
            ACE_ERROR_RETURN ((LM_ERROR,
                           "Cannot open output file for writing Id: %s",
                           object_id_file),
                          1);
          CORBA::ULong l = new_id->length ();
          if (ACE_OS::write (id_output_file, 
                             &l, 
                             sizeof (l))
              == -1
              || ACE_OS::write (id_output_file, 
                                new_id->get_buffer (), 
                                l) == -1)
            ACE_ERROR_RETURN ((LM_ERROR,
                               "Error writing object id to a file\n"),
                              1);
          ACE_OS::close (id_output_file);
      

          // Print Object IOR.
          CORBA::Object_var server =
            child_poa->id_to_reference (new_id.in (),
                                        ACE_TRY_ENV);
          ACE_TRY_CHECK;
          
          CORBA::String_var ior =
            orb->object_to_string (server.in (), ACE_TRY_ENV);
          ACE_TRY_CHECK;
          
          FILE *output_file= ACE_OS::fopen (ior_output_file, "w");
          if (output_file == 0)
            ACE_ERROR_RETURN ((LM_ERROR,
                               "Cannot open output file for writing IOR: %s",
                               ior_output_file),
                              1);
          ACE_OS::fprintf (output_file, "%s", ior.in ());
          ACE_OS::fclose (output_file);
        }
      // Run ORB Event loop.
      poa_manager->activate (ACE_TRY_ENV);
      ACE_TRY_CHECK;

      orb->run (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                           "Unexpected Exception caught in server:");
      return 1;
    }
  ACE_ENDTRY;

  return 0;
}
