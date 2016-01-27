#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dni_shutils.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <net/if.h>

void main( int argc, char **argv)
{
int s;
int iRet = 0;
struct ifreq ifr;
int iSpeed;

	//prepare socket and ifreq for getWanLinkState()
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("%s:: Create socket error [%d]\n", __FUNCTION__, errno );
		return -1;
	}

	memset( &ifr, 0, sizeof(ifr) );	
	if( argc == 1 )
	{
		printf( "\nUsage: phytest [eth?]\n" );
		printf( "\nWhere:\n" );
		printf( "\teth?: optional ehternet if, eth0 or eth1. eth1, i.e. WAN if, is default.\n\n" );
	}
	
	if( (argc >= 2) && argv[1] && !strncmp( argv[1], "eth", 3 ) )
		strcpy( ifr.ifr_name, argv[1] );
	else
		strcpy(ifr.ifr_name, "eth1");
		
	iRet = getWanLinkState( s, &ifr, &iSpeed );
	printf( "Link status [%d], speed [%d]\n", iRet, iSpeed );
	
	close(s);
}