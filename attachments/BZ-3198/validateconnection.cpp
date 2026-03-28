#include <tao/ORB.h>
#include <tao/PolicyC.h>

#include <iostream>

int main (int argc, char * argv[])
{
   CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

   try {
     CORBA::Object_var obj = orb->string_to_object(argv[1]);
     CORBA::PolicyList_var pl;
     obj->_validate_connection(pl);
   }
   catch(CORBA::Exception const& e) {
     std::cerr << "Exception: " << e << std::endl;
   }
   return 0;
}
