/**
 * This test reproduces wrong behaviour of
 * ACE_Refcounted_Auto_Ptr::release() method.
 *
 * @author Yauheni Akhotnikau  <eao197@intervale.ru>
 */

#include "test_config.h"

#include <ace/Refcounted_Auto_Ptr.h>
#include <ace/Null_Mutex.h>
 
class Test_Object
{
private :
  bool & flag_;

public :
  Test_Object( bool & flag ) : flag_( flag )
    {
      flag_ = true;
    }
  ~Test_Object()
    {
      flag_ = false;
    }
};
 
int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Refcounted_Auto_Ptr_release_Test"));

  bool flag = true;

  Test_Object * p = 0;
  {
    ACE_Refcounted_Auto_Ptr< Test_Object, ACE_Null_Mutex > test(
        new Test_Object( flag ) );
    if( !flag )
      ACE_ERROR_RETURN ((LM_ERROR,
                         "flag must be true at point (1)\n" ),
                        -1 );

    p = test.release();

    if( !flag )
      ACE_ERROR_RETURN ((LM_ERROR,
                         "flag must be true at point (2)\n" ),
                        -1);
  }

  if( !flag )
    ACE_ERROR_RETURN ((LM_ERROR,
                       "flag must still be true at point (3)\n" ),
                      -1);

  ACE_END_TEST;

  return 0;
}

