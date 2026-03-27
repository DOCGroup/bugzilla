// $Id: client.cpp,v 1.2 2001/04/11 02:21:16 coryan Exp $


#include <strstream>
#include <string>
#include <tao/corba.h>
#include <orbsvcs/CosNamingC.h>
#include <libmalloc.h>

ACE_RCSID(NamingContextNarrowLeak, client, "$Id: client.cpp,v 1.2 2001/04/11 02:21:16 coryan Exp $")

int main (int argc, char * argv[])
{
  SetMallocFlags(5);

  int iterations = 2000;

  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "");

  std::string server;
  if (argc > 1) server = argv[1];
  else exit(1);

  std::ostrstream iiop_str;
#if 1
  iiop_str << "iiop://" << server << ":10066/NameService" << '\000';
#else
  iiop_str << "file:///tmp/ns.ior" << ends;
#endif
  
  while (--iterations) {
    char* is = iiop_str.str();
    
    CORBA::Object_var init_cx_obj =
      orb->string_to_object(is);
    
    CosNaming::NamingContext_var init_cx =
      CosNaming::NamingContext::_narrow (init_cx_obj.in());
    
    iiop_str.freeze(0);
  }

  cout << "Stopping with " << iterations << " left." << endl;
  _exit(0);


  return 0;
}
