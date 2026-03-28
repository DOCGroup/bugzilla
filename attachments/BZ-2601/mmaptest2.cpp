/*
/opt2/linux/ix86/x86_64-pc-linux-gnu/bin/g++ -m64 -I/opt2/linux/x86_64/ACE_wrappers -lACE -L/home/lothar/workspace/tradescape/lib/linux/x86_64/external -L/home/lothar/workspace/tradescape/lib/linux/x86_64/coverage mmaptest.cpp -o mmaptest

rm -f /tmp/mmaptest2.img; LD_LIBRARY_PATH=/home/lothar/workspace/tradescape/lib/linux/x86_64/coverage:/home/lothar/workspace/tradescape/lib/linux/x86_64/external ./mmaptest
*/

#include <memory>
#include <iostream>

#include "ace/OS.h"

int main(int, char **)
{
	std::cout << "sizeof(off_t)=" << sizeof(off_t) << std::endl;
	std::cout << "sizeof(size_t)=" << sizeof(size_t) << std::endl;

	for(int i = 0; i < 40; ++i)
	{
		size_t size = 2L << i;
		errno = 0;
		ACE_HANDLE fd = ACE_OS::open("/tmp/mmaptest2.img", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
		if(fd == ACE_INVALID_HANDLE)
		{
			std::cerr << "oops, cannot open file: " << ACE_OS::strerror(errno) << std::endl;
			abort();
		}
		errno = 0;
		int result = ACE_OS::ftruncate(fd, size);
		if(result != 0)
		{
			std::cerr << "oops, cannot truncate file: " << ACE_OS::strerror(errno) << std::endl;
			abort();
		}
		errno = 0;
		void * p_memory = ACE_OS::mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if(p_memory == 0)
		{
			std::cerr << "oops, cannot map memory: " << ACE_OS::strerror(errno) << std::endl;
			abort();
		}
		else
		{
			std::cout << "map memory (" << size << " == 2^" << i << ") OK" << std::endl;
			char * p_x = (char *)p_memory;
			p_x += size - 1;
			*p_x = 1;
			ACE_OS::munmap(p_memory, size);
			ACE_OS::close(fd);
		}
	}
}
