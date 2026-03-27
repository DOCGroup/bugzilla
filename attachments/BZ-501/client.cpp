#include "EchoC.h"
#include <iostream.h>
#include <stdlib.h>

// A minimal client illustrating the use of TAO
int
main (int argc, char **argv)
{

  try {

    // Initialize the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);

    // get IOR of EchoServer from command line (argv[1]) and convert to
    // object reference
    CORBA::Object_var object = orb->string_to_object(argv[1]);

    // Downcast the object reference to a Echo reference.
    Echo_var echo = Echo::_narrow(object.in());

    CORBA::String_var echo_string;
    if (argc > 2) {
      echo_string = CORBA::string_dup(argv[2]);
    }
    else {
      echo_string = CORBA::string_dup("Hello, world.");
    }

    cout << "Echoing the string: '" << echo_string.in() << "'" << endl;

    // Access the Echo object on the server side
    CORBA::String_var what_it_said = echo->echo_it(echo_string.in());
    cout << "Echo Service responds: '" << what_it_said.in() << "'" << endl;
  
  }
  catch (CORBA::SystemException& exc) {
    cerr << "Client caught a CORBA::SystemException exception" << endl
         << exc << endl;
    return 1;
  }
  catch (CORBA::Exception& exc) {
    cerr << "Client caught unknown CORBA::Exception exception" << endl
         << exc << endl;
    return 1;
  }
  catch (...) {
    cerr << "Client caught unknown exception." << endl;
    return 1;
  }

  return 0;

}
