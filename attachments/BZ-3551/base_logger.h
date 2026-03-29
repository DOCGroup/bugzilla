#ifndef _BASE_LOGGER_H_
#define _BASE_LOGGER_H_

#include "log.h"
#include <ace/Task.h>
#include <ace/Mutex.h>

namespace test_space
{

    /* @{ */
    /**
     * @class base_logger
     *
     * @brief base_logger is the class that provides mechanism to use for logging messages.
     */
    class base_logger : public ACE_Task< ACE_MT_SYNCH >
    {
    public:
        /// Constructor
        base_logger()
                : ACE_Task< ACE_MT_SYNCH >(ACE_Thread_Manager::instance())
        {
        };
        /// Destructor
        virtual ~base_logger()
        {
        };

        /// Pure virtual to be used with the derived class for logging messages
        virtual void log_message(   severity_t severity_level,
                                    unsigned int line_number,
                                    const char *file,
                                    const char *function_name,
                                    const char *message = NULL ) = 0;
    };
    /* @} */

};

#endif

