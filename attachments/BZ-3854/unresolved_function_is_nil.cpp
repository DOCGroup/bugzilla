#include <iostream>

#include <tao/ORB_Core.h>
#include <tao/Object.h>

int main (int argc, char** argv)
{
  try {
    if (argc != 2) {
      std::cerr << "usage: " << argv[0] << " <uri>" << std::endl;
      return 1;
    }
    CORBA::ORB_ptr orb = CORBA::ORB_init (argc, argv, "TAO");
    CORBA::Object_var obj2resolve = orb->string_to_object (argv [1]);
    if (CORBA::is_nil (obj2resolve.in ())) {
      std::cerr << argv[0] << ": could not resolve object" << std::endl;
      return 2;
    }
    CORBA::String_var stringified_object = orb->object_to_string (obj2resolve.in ());
    std::cout << stringified_object << std::endl;
  } catch (...) {
    std::cerr << argv[0] << ": conversion caused exception" << std::endl;
    return 3;
  }
  return 0;
}
