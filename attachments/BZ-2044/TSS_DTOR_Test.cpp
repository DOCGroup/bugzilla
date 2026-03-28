// $Id$
// ============================================================================
//
// = LIBRARY
//    tests
//
// = FILENAME
//    TSS_DTOR_Test.cpp
//
// = DESCRIPTION
//    This test ensures that dynamically created TSS objects are fully
//    reclaimed when the containing object is destroyed.  The TSS
//    thread_key must deallocated and the TSS object's destructor must
//    be called.
//
// = AUTHOR
//    Ken Sedgwick <ken@bonsai.com>
//
// ============================================================================

#include "test_config.h"
#include "ace/TSS_T.h"

ACE_RCSID(tests, TSS_DTOR_Test, "$Id$")

static int counter;

class TSS_Test_Object
{
public:
  TSS_Test_Object (void);
  ~TSS_Test_Object (void);

  int value_;
};

TSS_Test_Object::TSS_Test_Object (void)
{
  ++counter;
}

TSS_Test_Object::~TSS_Test_Object (void)
{
  --counter;
}

class Created_Object
{
public:
  Created_Object (void);

private:
  ACE_TSS<TSS_Test_Object> tss_;
};

Created_Object::Created_Object (void)
{
  tss_->value_ = 42;            // fault in the TSS
}

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("TSS_DTOR_Test"));

  counter = 0;

  // By setting the loop count to a larger number then the maximum
  // concurrent number of allocated thread keys we require successful
  // deallocation (and reallocation) to complete this loop.

  for (unsigned i = 0; i < 1024 * 4; ++i)
    {
      Created_Object * obj;
      ACE_NEW_RETURN (obj, Created_Object, 1);
      delete obj;
    }

  // If the constructors and destructors have been called correctly
  // the counter will be exactly 0.

  ACE_ASSERT(counter == 0);

  return 0;
}

// Local Variables:
// mode: C++
// c-basic-offset: 2
// c-file-offsets: ((substatement-open . 2))
// End:
