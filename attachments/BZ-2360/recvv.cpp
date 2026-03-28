#include <ace/SOCK_Connector.h>

#include <cstdlib>
#include <iostream>

int main()
{
  // connect to daytime port on localhost
  ACE_SOCK_Stream stream;
  ACE_SOCK_Connector connector(stream, ACE_INET_Addr(13, "localhost"));

  iovec v;

  // expected output is 26
  std::cout << "recvv: " << stream.recvv(&v) << std::endl;
  return 0;
}
