#include "ace/config-all.h"

ACE_Proper_Export_Flag int Function1()
{
    return 1;
}

#ifdef SOME_DEFINE
ACE_Proper_Export_Flag int Function2()
{
    return 2;
}
#endif

