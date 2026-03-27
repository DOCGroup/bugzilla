// client.cpp,v 1.16 2001/09/18 00:06:00 irfan Exp

#include "testC.h"
#include "ace/Get_Opt.h"

const char *ior = "file://test.ior";
const char *message = "Hello from the client";

int
parse_args (int argc, char *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, "i:m:");
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'i':
        ior = get_opts.optarg;
        break;
      case 'm':
        message = get_opts.optarg;
        break;
      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s "
                           "-i <ior> "
                           "-m <message> "
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
      // Initialize and obtain reference to the Test object.
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv, "", ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (parse_args (argc, argv) != 0)
        return 1;

      CORBA::Object_var object =
        orb->string_to_object (ior, ACE_TRY_ENV);
      ACE_TRY_CHECK;

      Test_var server =
        Test::_narrow (object.in (), ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (CORBA::is_nil (server.in ()))
          ACE_ERROR_RETURN ((LM_ERROR,
                             "ERROR: Object reference <%s> is nil\n",
                             ior),
                            1);
      
      // Invoke a method.
      server->test_method (message, ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                           "Caught unexpected exception in client:");
      return 1;
    }
  ACE_ENDTRY;

  return 0;
}
