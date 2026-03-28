/**
 * @file Bug_3532_Regression_Test.cpp
 *
 * $Id: $
 *
 * Reproduces the problems reported in bug 3532
 *   http://deuce.doc.wustl.edu/bugzilla/show_bug.cgi?id=3532
 *   
 * @author gaus  
 */

#include "test_config.h"
#include "ace/ACE.h"

ACE_RCSID (tests,
           Bug_3532_Regression_Test,
           "$Id: $")

int
run_main (int, ACE_TCHAR *[])
{
    ACE_START_TEST (ACE_TEXT ("Bug_3532_Regression_Test"));

    char Buffer[10];
    FILE* pFile = 0;

    // Write a ASCII file with one byte (no BOM)
    Buffer[0] = 'T';
    pFile = ACE_OS::fopen("OneByteFile", L"wb");
    ACE_OS::fwrite(&Buffer, 1, 1, pFile);
    ACE_OS::fclose(pFile);
    
    // Reopen the file and read the byte
    Buffer[0] = '-';
    pFile = ACE_OS::fopen("OneByteFile", L"rb");
    size_t BytesRead = ACE_OS::fread(&Buffer, 1, 1, pFile);
    if(BytesRead == 1)
    {
        if(Buffer[0] != 'T')
        {
            ACE_ERROR ((LM_ERROR,
                        ACE_TEXT ("Error: 'T' expected!!!\n")));            
        }
    }
    else
    {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("Error: One byte should be read!!!\n")));
    }
    
    ACE_OS::fclose(pFile);

    ACE_END_TEST;

    return 0;
}
