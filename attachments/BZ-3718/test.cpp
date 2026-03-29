#if defined (_POSIX_PTHREAD_SEMANTICS)
# error INVALID
#endif

#include <iostream>

// SunOS 5.9 hosts automatically set _POSIX_PTHREAD_SEMANTICS
// in <sys/feature_tests.h> which is included by config-sunos5.5.h:
#include <sys/feature_tests.h>

int
main ()
{
#if defined (_POSIX_PTHREAD_SEMANTICS)
  std::cout << "FAILED" << std::endl;
  return 1;
#endif
  return 0;
}
