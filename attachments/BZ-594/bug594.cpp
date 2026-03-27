// bug594.cpp
//

#include <ace/OS.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>

void	send_packet( ACE_SOCK_Stream& stream, const void *p, size_t n )
{
 ACE_Time_Value tmo(30);	// 30-sec timeout
 ssize_t st = stream.send_n( p, n, 0, &tmo );

 if( st<0 ) {
	ACE_OS::fprintf( stderr, "error sending %d bytes, errno=%d\n", n, errno );
 }
}

int	main( int argc, char *argv[] )
{
 if( argc<2 ) {
	ACE_OS::fprintf( stderr, "please specify host name\n" );
	return 1;
 }

 ACE_INET_Addr sink_addr( 9, argv[1] );

 ACE_SOCK_Stream stream;
 ACE_SOCK_Connector connector;

 if( connector.connect( stream, sink_addr )<0 ) {
	ACE_OS::fprintf( stderr, "unable to connect to host\n" );
	return 1;
 }

 ACE_OS::fprintf( stderr, "Connected to %s\n", argv[1] );

 char *buf = new char[8192];

 for( int i=0; i<100; ++i ) {
	// send 'magic' block sequence
	ACE_OS::fprintf( stderr, "try %d\n", i );
	send_packet( stream, buf, 142 );
	send_packet( stream, buf, 46 );
	send_packet( stream, buf, 2 );
	send_packet( stream, buf, 8192 );
	send_packet( stream, buf, 8192 );
	send_packet( stream, buf, 8192 );
 }

 delete[] buf;

 stream.close();

 return 0;
}


