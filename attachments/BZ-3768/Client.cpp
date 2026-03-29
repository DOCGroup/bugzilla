#include "ACE/OS.h"
#include "ACE/Task.h"
#include "TestInterfaceS.h"
#include <stdio.h>

class Worker : public ACE_Task_Base {
public:
  Worker(CORBA::ORB_ptr orb) : orb_(CORBA::ORB::_duplicate(orb)) {}
  int svc(void) {
    orb_->run();
    return 0;
  }
  CORBA::ORB_var orb_;
};

class TestClient_i : public POA_Test1
{
public:
  TestClient_i(CORBA::ORB_ptr orb) : work_(orb) {}
  void call1() {
    printf("call1() called, starting threads...\n");
    work_.activate(0,10);
    ACE_OS::sleep(-1); /* Comment this out to make the test case work */
  }
  void call2() {
    printf("call2() called\n");
  }
  Worker work_;
};

int main(int argc, char** argv)
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc,argv);
  CORBA::Object_var root_poa_obj = orb->resolve_initial_references("RootPOA");
  PortableServer::POA_var root_poa = PortableServer::POA::_narrow(root_poa_obj.in());
  PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();

  TestClient_i servant(orb.in());
  PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId("TestClient");
  root_poa->activate_object_with_id(oid.in(),&servant);
  CORBA::Object_var clientObj = root_poa->id_to_reference(oid.in());
  Test1_var client = Test1::_narrow(clientObj.in());
  poa_manager->activate();

  char ior[10000];
  FILE* f = fopen("ior.txt","rt");
  fgets(ior,sizeof ior,f);
  fclose(f);
  puts(ior);

  CORBA::Object_var svrObj = orb->string_to_object(ior);
  Test2_var svr = Test2::_narrow(svrObj.in());
  printf("Calling server...\n");
  svr->call(client.in());
  ACE_OS::sleep(-1);
  return 0;
}
