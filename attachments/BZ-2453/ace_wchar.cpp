#include <ace/ACE.h> 
#if defined(ACE_HAS_ICONV)
#include <iconv.h> 

iconv_t ACE_Wide_To_Ascii::ACE_Wide_To_Ascii_iconv_env=NULL;
iconv_t ACE_Ascii_To_Wide::ACE_Ascii_To_Wide_iconv_env=NULL;
#endif
