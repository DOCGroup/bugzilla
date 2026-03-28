/**
 * @file Bug_3156_Regression_Test.cpp
 *
 * $Id: Bug_3156_Regression_Test.cpp 55322 2004-01-10 21:20:20Z shuston $
 *
 * Reproduces the problems reported in bug 3156:
 *   http://deuce.doc.wustl.edu/bugzilla/show_bug.cgi?id=3156
 *
 * @author Bogdan Graur <bgraur@gmail.com>
 */

#include <new>

#include "test_config.h"

#include "ace/OS.h"
#include "ace/Log_Msg.h"
#include "ace/CDR_Stream.h"

#include <iostream>

static int alloc_count = 0;
static int error_count = 0;

void log_func()
{
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("<<Inside log_func() called by ACE_Log_Msg::log formatter %%r>>\n")));
}

/**
 * Oveload the new(::std::nothrow) operator in order to keep score
 * on the allocation count
 */
void* 
operator new ( std::size_t size, const std::nothrow_t& unused) throw()
{	
  ACE_UNUSED_ARG (unused);
  ++alloc_count;
  
	return (void*) (new char[size]);
}

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Bug_3156_Regression_Test"));
  
  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT (" Test bug fix for the part: replace member "
                       "ACE_Log_Msg::msg_ of type ACE_TCHAR* with ACE_Log_Msg::log_record_ "
                       "of type ACE_Log_Record*\n")));
   
  ACE_DEBUG ((LM_DEBUG,
		  	  ACE_TEXT (" Log text1 \n")));

  ACE_DEBUG ((LM_DEBUG,
		  	  ACE_TEXT (" Log text2 \n")));
  
  int temp_alloc_count = alloc_count;
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT (" At this point nb of allocations is %d. \n"), 
              temp_alloc_count));

  ACE_DEBUG ((LM_DEBUG,
		  	      ACE_TEXT (" Log text3 \n")));

  ACE_DEBUG ((LM_DEBUG,
		  	      ACE_TEXT (" Log text4 \n")));
  
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" Log text5 \n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" Log text6 \n")));

  ACE_DEBUG ((LM_DEBUG, 
              ACE_TEXT (" At this point nb of allocations is %d. \n"), alloc_count));
  ACE_DEBUG ((LM_DEBUG, 
              ACE_TEXT (" It should be: %d. \n"), temp_alloc_count));

  if (alloc_count != temp_alloc_count)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\n\n Test FAILED!\n\n")));
      ++error_count;
    }
  else
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\n\n Test SUCCEEDED!\n\n")));
    }
  
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Testing a serialization and a deserialization\n")));
  
  ACE_OutputCDR output;
  ACE_Log_Record record1;
  ACE_Log_Record record2;
  
  record1.msg_data (ACE_TEXT (" ''Test optimisation of 'operator>>(ACE_InputCDR&,ACE_Log_Record&)''\n"));
  record1.priority (LM_DEBUG);
  record1.pid (ACE_OS::getpid());
  record1.time_stamp(ACE_OS::gettimeofday());
  
  output << record1;
  
  ACE_InputCDR input (output);
  
  temp_alloc_count = alloc_count;
  ACE_DEBUG ((LM_DEBUG, 
              ACE_TEXT (" At this point nb of allocations is %d. \n"), temp_alloc_count));
  input >> record2;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" Serialized ACE_Log_Record is:\n")));
  ACE_DEBUG ((record1, 0));
  
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" Deserialized ACE_Log_Record is:\n")));  
  ACE_DEBUG ((record2, 0));
  
  ACE_DEBUG ((LM_DEBUG, 
              ACE_TEXT (" At this point nb of allocations is %d. \n"), alloc_count));
  ACE_DEBUG ((LM_DEBUG, 
              ACE_TEXT (" It should be: %d. \n"), temp_alloc_count));
  
  if (alloc_count != temp_alloc_count)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (" \n\n Test FAILED!\n\n")));
      ++error_count;
    }
  else
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT (" \n\n Test SUCCEEDED!\n\n")));
    }
  
  // check equality for record1 == record2
  if (record1.priority () != record2.priority () || 
      record1.pid () != record2.pid () ||
      record1.time_stamp () != record2.time_stamp () ||
      ACE_OS::strcmp (record1.msg_data(), record2.msg_data()) != 0)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("Serialized record is not equal to deserialized record!\n")));
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\n\n Serialization/Deserialization test FAILED!\n\n")));
      
      record1.dump();
      record2.dump();
      
      ++error_count;
    }
  else
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\n\n Serialization / Deserialization test SUCCEEDED!\n\n")));
    }
  
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("Testing %%r formatter. We have to check by "
                        "observation to see if test succeeded.\n")));
  
  ACE_DEBUG ((LM_DEBUG, 
              ACE_TEXT ("While formatting this a function is called: "
                        "<BEFORE CALL> <%r> <AFTER CALL>\n"), log_func));
    
  ACE_END_TEST;

  return error_count;
}



