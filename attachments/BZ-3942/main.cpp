#include "TestS.h"


class MyInterface_impl
  : public virtual POA_MyInterface
{
};

int main(int argc, char* argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

  {
    MyInterface_impl* myInterface = new MyInterface_impl();
 
    MyInterfaceSequence sequence;
    sequence.length(1);
    sequence[0U] = myInterface->_this();

    {
      MyInterface_var takingOwnership = sequence[0U];
    }
  } // Access violation occurs on cleanup of sequence 

  orb->destroy();  

  return 0;
}