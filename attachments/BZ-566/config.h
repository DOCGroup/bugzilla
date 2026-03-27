extern "C" {
void* mymalloc (unsigned int);
void* mycalloc (unsigned int, unsigned int);
void* myrealloc (void*, unsigned int);
void myfree (void*);
};

#define ACE_MALLOC_FUNC dlmalloc
#define ACE_CALLOC_FUNC dlcalloc
#define ACE_REALLOC_FUNC dlrealloc
#define ACE_FREE_FUNC   dlfree
#define ACE_HAS_STRDUP_EMULATION
#include "config-linux-lxpthreads.h"
