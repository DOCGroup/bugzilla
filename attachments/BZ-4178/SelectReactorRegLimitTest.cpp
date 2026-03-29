 /***********************************************************************
 *  @file    SelectReactorRegLimitTest.cpp
 *
 *  $Id:$
 *
 *  Verify that the Select Reactor on Windowsdoes not allow more event 
 *  registrations than its repository limit (size is defaulted to FD_SET)
 *
 *
 *  @author Matthew L. Newville <mlnewville@micron.com>
 ************************************************************************/

#include "test_config.h"
#include <ace/Select_Reactor.h>
#include <ace/Mem_Map.h>

//Create a dummy event handler class
class DummyEventHandler : public ACE_Event_Handler
{
public:
	DummyEventHandler(){};
};


class Tester
{
public:
  int test (void);
};

int
Tester::test (void)
{
  int retval = 0;
  int regStatus = 0;

  //Instantiate a Select Reactor instance
  ACE_Select_Reactor * pReactor = new ACE_Select_Reactor();  

  //Create a dummy event handler
  DummyEventHandler * pHandler = new DummyEventHandler();

  //Get the reactor's maximum registration size
  size_t maxRegCount = pReactor->size();

  ACE_DEBUG ( (LM_DEBUG,
		ACE_TEXT ("Max registration count:%d.\n"), maxRegCount));

  int regCount = 1;


  for (; regCount <= maxRegCount + 1; ++regCount)
  {	
	ACE_Mem_Map * pMap = new ACE_Mem_Map("delete_me", 10);
    ACE_HANDLE handle = pMap->handle();

	//Register the dummy event handler with the reactor:
	regStatus = pReactor->register_handler(handle, pHandler, ACE_Event_Handler::ACCEPT_MASK);
	
	//Bail-out when the status is failure
	if (regStatus != 0)	
		break;
  }

  ACE_DEBUG ( (LM_DEBUG,
			ACE_TEXT ("Total handles registred::%d.\n"), regCount));

  delete pReactor;
  delete pHandler;

  //Validate that the registration failed after exceeding the maximum registration count:
  if ( regCount >= maxRegCount )
  {	  
	  if (regStatus == 0)
			ACE_ERROR_RETURN ((LM_ERROR,
				ACE_TEXT ("Error: register_handler did not return error status after exceeding registration limit\n")),
				-1);
	  else
		  ACE_DEBUG ( (LM_DEBUG,
				ACE_TEXT ("Success: register_handler failed upon exceeding registration limit.\n")));

  }
  else
  {
	  if (regStatus == 0)
			ACE_ERROR_RETURN ((LM_ERROR,
				ACE_TEXT ("Error: register_handler returned failure before exceeding maximum registration limit.\n")),
				-1);
  }

  return retval;
}

int run_main(int, ACE_TCHAR* [])
{
  ACE_START_TEST (ACE_TEXT ("SelectReactorRegLimitTest"));

  Tester tester;

  int result = tester.test();

  if (result == 0)
    ACE_DEBUG((LM_DEBUG,
               ACE_TEXT ("SelectReactorRegLimitTest succeeded\n")));
  else
    ACE_ERROR((LM_ERROR,
               ACE_TEXT ("SelectReactorRegLimitTest failed\n")));

  ACE_END_TEST;

  return result;
}
