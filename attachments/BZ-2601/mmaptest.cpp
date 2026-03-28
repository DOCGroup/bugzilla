/*
/opt2/linux/ix86/x86_64-pc-linux-gnu/bin/g++ -m64 -I/opt2/linux/x86_64/ACE_wrappers -lACE -L/home/lothar/workspace/tradescape/lib/linux/x86_64/external -L/home/lothar/workspace/tradescape/lib/linux/x86_64/coverage mmaptest.cpp -o mmaptest

rm -f /tmp/mmaptest.img; LD_LIBRARY_PATH=/home/lothar/workspace/tradescape/lib/linux/x86_64/coverage:/home/lothar/workspace/tradescape/lib/linux/x86_64/external ./mmaptest
*/

#include <memory>
#include <iostream>

#include "ace/Null_Mutex.h"
#include "ace/Memory_Pool.h"
#include "ace/Malloc_T.h"
#include "ace/PI_Malloc.h"


class MemoryPool
:
	public ACE_MMAP_MEMORY_POOL
{
	public:
		MemoryPool (
			const ACE_TCHAR *backing_store_name = 0,
			const ACE_MMAP_MEMORY_POOL::OPTIONS *options = 0
		);
		
		virtual ~MemoryPool ( );
		
		ACE_HANDLE handle ( );
};


MemoryPool::MemoryPool (
	const ACE_TCHAR *backing_store_name,
	const ACE_MMAP_MEMORY_POOL::OPTIONS *options
):
	ACE_MMAP_MEMORY_POOL(
		backing_store_name,
		options
	)
{
} /* end of MemoryPool::MemoryPool ( ) */


MemoryPool::~MemoryPool ( )
{
} /* end of MemoryPool::~MemoryPool ( ) */


ACE_HANDLE MemoryPool::handle ( )
{
  return mmap_.handle();
} /* end of MemoryPool::handle ( ) */



typedef ACE_Malloc_T <
	MemoryPool,
	ACE_Null_Mutex,
	ACE_PI_Control_Block
> SharedMalloc;

typedef ACE_Allocator_Adapter <
	SharedMalloc
> SharedAllocator;


int main(int, char **)
{
	std::auto_ptr < ACE_MMAP_Memory_Pool_Options > a_options(
		new ACE_MMAP_Memory_Pool_Options(
			ACE_DEFAULT_BASE_ADDR,                           //const void *base_addr = ACE_DEFAULT_BASE_ADDR,
			ACE_MMAP_Memory_Pool_Options::NEVER_FIXED,       //int use_fixed_addr = ALWAYS_FIXED,
			0,                                               //int write_each_page = 1,
			0,                                               //off_t minimum_bytes = 0,
			0,                                               //u_int flags = 0,
			1,                                               //int guess_on_fault = 1,
			0,                                               //LPSECURITY_ATTRIBUTES sa = 0,
			S_IWUSR|S_IRUSR|S_IRGRP                          //mode_t file_mode = ACE_DEFAULT_FILE_PERMS
		)
	);
	
	std::auto_ptr < SharedAllocator > a_sharedAllocator(
		new SharedAllocator(
			"/tmp/mmaptest.img"
			,"MyMemory"
			,a_options.get()
		)
	);
	
	void * p_base = a_sharedAllocator->alloc().base_addr();
	
	for(int i = 0; i < 40; ++i)
	{
		size_t size = 2L << i;
		errno = 0;
		void * p_memory = a_sharedAllocator->malloc(size);
		if(p_memory == 0)
		{
			std::cerr << "oops, cannot allocate memory: " << ACE_OS::strerror(errno) << std::endl;
			abort();
		}
		else
		{
			std::cout << "allocate memory (" << size << " == 2^" << i << ") OK" << std::endl;
			a_sharedAllocator->free(p_memory);
		}
	}
}
