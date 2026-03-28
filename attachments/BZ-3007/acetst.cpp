#include "ace/Log_Msg.h"
#include "ace/Service_Config.h"

int main(int argc, char *argv[])
{
    if( ACE_Service_Config::open( argc, argv, ACE_DEFAULT_LOGGER_KEY,
                                  0, 0, 0 ) < 0 )
    {
        ACE_DEBUG(( LM_ERROR,
                    ACE_TEXT( "ERROR: %p\n" ),
                    ACE_TEXT( "Service_Config open" ) ));
        return -1;
    }

    ACE_DEBUG(( LM_TRACE,  ACE_TEXT("LogTest: Trace\n") ));
    ACE_DEBUG(( LM_DEBUG,  ACE_TEXT("LogTest: Debug\n") ));
    ACE_DEBUG(( LM_INFO,   ACE_TEXT("LogTest: Info\n") ));
    ACE_DEBUG(( LM_WARNING,ACE_TEXT("LogTest: Warning\n") ));
    ACE_DEBUG(( LM_NOTICE, ACE_TEXT("LogTest: Notice\n") ));
    ACE_DEBUG(( LM_ERROR,  ACE_TEXT("LogTest: Error\n") ));

    ACE_Service_Config::reconfigure();

    ACE_DEBUG(( LM_TRACE,  ACE_TEXT("LogTest: Trace\n") ));
    ACE_DEBUG(( LM_DEBUG,  ACE_TEXT("LogTest: Debug\n") ));
    ACE_DEBUG(( LM_INFO,   ACE_TEXT("LogTest: Info\n") ));
    ACE_DEBUG(( LM_WARNING,ACE_TEXT("LogTest: Warning\n") ));
    ACE_DEBUG(( LM_NOTICE, ACE_TEXT("LogTest: Notice\n") ));
    ACE_DEBUG(( LM_ERROR,  ACE_TEXT("LogTest: Error\n") ));

    return 0;
}
