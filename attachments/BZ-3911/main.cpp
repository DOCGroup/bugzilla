#include <string>
using namespace std;

#include <ace/Malloc_T.h>
#include <ace/Memory_Pool.h>
#include <ace/Process_Mutex.h>
#include <ace/Null_Mutex.h>
#include <ace/PI_Malloc.h>

class ShmemMan
{
private:
  // Used to implement the singleton pattern:
  static ShmemMan* c_instance;

  // The memory pool managing all the shared memory:
  ACE_Malloc_T<ACE_MMAP_MEMORY_POOL,
        ACE_Null_Mutex,
        ACE_PI_Control_Block>*    c_memory_pool;

protected:
  ShmemMan(bool do_crash)
  {
    if (do_crash)
      c_memory_pool = new ACE_Malloc_T<ACE_MMAP_MEMORY_POOL, 
                      ACE_Null_Mutex,
                      ACE_PI_Control_Block>("shared_memory");
    else
    {
      ACE_MMAP_Memory_Pool_Options options (ACE_DEFAULT_BASE_ADDR);
      c_memory_pool = new ACE_Malloc_T<ACE_MMAP_MEMORY_POOL, 
                      ACE_Null_Mutex,
                      ACE_PI_Control_Block>("shared_memory"
                                            , "", &options
                                            );
    }
  };

public:
  ~ShmemMan()
  {
    c_memory_pool->release();
    delete c_memory_pool;
  };

  static ShmemMan* getInstance(bool do_crash)
  {
    if (c_instance == 0)
    {
      c_instance = new ShmemMan(do_crash);
    }
    return c_instance;
  };

  int clean()
  {
    c_memory_pool->release();
    c_memory_pool->remove();

    return 0;
  };

  void* getMemoryBlock(string block_name, unsigned int block_size)
  {
    void* shared;

    ACE_OS::printf("errno = %d. Looking for a Shared Memory block named %s...\n",
        ACE_OS::last_error(),
        block_name.c_str());

    if (c_memory_pool->find(block_name.c_str(), shared) == 0)
    {
      // An existing block was found, so take that:
      ACE_OS::printf("Shared Memory block %s was found.",
          block_name.c_str());
    }
    else
    {
      // Allocate the memory and bind it to a name:
      ACE_OS::printf("Shared Memory block %s was not found. errno = %d. Trying to allocate new block...\n",
          block_name.c_str(),
          ACE_OS::last_error());
      shared = c_memory_pool->malloc(block_size);
      if (shared < 0)
      {
        ACE_OS::printf("New Shared Memory block could not be allocated. errno = %d.\n",
            ACE_OS::last_error());
        return (void*)(-1);
      }
      ACE_OS::printf("New Shared Memory block was allocated, trying to bind it to the name %s...\n",
          block_name.c_str());
      if (c_memory_pool->bind(block_name.c_str(), shared) < 0)
      {
        ACE_OS::printf("New Shared Memory block could not be bound to the name %s. errno = %d.\n",
            block_name.c_str(),
            ACE_OS::last_error());

        return (void*)(-1);
      }
    }

    return shared;
  };
      
};

ShmemMan* ShmemMan::c_instance = 0;

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  bool do_crash = (argc>1 && argv[1][0]=='1');
  ShmemMan* smm = ShmemMan::getInstance (do_crash);
  
  void* buf = smm->getMemoryBlock ("block_1", 10 * 4096);
  
  ACE_OS::printf("allocated shmem block at %p\n", buf);

  return 0;
}

