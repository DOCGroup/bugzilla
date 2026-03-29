#include "ACE/OS.h"
#include "ACE/Task.h"
#include "TestInterfaceS.h"
#include <stdio.h>

class TestServer_i : public POA_Test2
{
public:
  void call(Test1_ptr test) {
    test1_ = Test1::_duplicate(test);
    test1_->call1();
  }
  Test1_var test1_;
};

class Worker : public ACE_Task_Base {
public:
  Worker(CORBA::ORB_ptr orb) : orb_(CORBA::ORB::_duplicate(orb)) {}
  int svc(void) {
    orb_->run();
    return 0;
  }
  CORBA::ORB_var orb_;
};

int main(int argc, char** argv)
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc,argv);
  CORBA::Object_var root_poa_obj = orb->resolve_initial_references("RootPOA");
  PortableServer::POA_var root_poa = PortableServer::POA::_narrow(root_poa_obj.in());
  PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();

  TestServer_i servant;
  PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId("TestServer");
  root_poa->activate_object_with_id(oid.in(),&servant);
  CORBA::Object_var obj = root_poa->id_to_reference(oid.in());
  poa_manager->activate();

  CORBA::String_var ior = orb->object_to_string(obj);
  FILE* f = fopen("ior.txt","wt");
  fprintf(f,"%s",ior.in());
  fclose(f);
  puts(ior.in());

  Worker work(orb.in());
  work.activate(0,1);
  printf("Waiting for 10 seconds, please start test client.\n");
  ACE_OS::sleep(10);
  if (servant.test1_.in() == 0) {
    printf("Client has not yet called, exiting...\n");
    return 1;
  }
  printf("Should now see messages showing repeated calls to client...\n");
  int counter = 0;
  while (1) {
    printf("Calling client (%d)\n",++counter);
    servant.test1_.in()->call2();
    ACE_Time_Value tv(0,100*1000);
    ACE_OS::sleep(tv);
  }
  return 0;
}
