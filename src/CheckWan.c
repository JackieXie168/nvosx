#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include	<net/if.h>
#include <dni_shutils.h>


extern int g_hearbeat_is_listening;
static int g_sys_is_restarting=0;
void on_signal(int i)
{
    cprintf("%s::recvd sig:%d\n", __FUNCTION__, i);
    if( i == SIGUSR1 )
		g_hearbeat_is_listening=1;
    else if( i == SIGUSR2)
		g_hearbeat_is_listening=0;
    else if( i== SIGHUP )
		g_sys_is_restarting = 1;
    signal(i, on_signal);
}

//for debuging CheckWan
#if 0
#define DEBUG_CheckWan	cprintf
#else
#define DEBUG_CheckWan	
#endif

#define SLEEP_SHORT		5
#define SLEEP_LONG		60
//int CheckWan( int argc, char **argv)
int main( int argc, char **argv)
{
int sleep_interval=SLEEP_SHORT;
unsigned long sleep_short_nums=0;
int bLastWanState, bCurWanState;
int bLastLinkState, bCurLinkState;
int s;
struct ifreq ifr;

bLastWanState = WAN_INIT;
bLastLinkState = LINK_INIT;

	signal(SIGCHLD, SIG_IGN);  //to prevent zombies process
	signal(SIGUSR1,on_signal);  //heartbeat is listening lan interface
	signal(SIGUSR2,on_signal);  //heartbeat is NOT listening lan interface
	signal(SIGHUP, on_signal); //if system is restarting

	//prepare socket and ifreq for getWanLinkState()
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		cprintf("%s:: Create socket error [%d]\n", __FUNCTION__, errno );
		return -1;
	}
	//strcpy(ifr.ifr_name, "eth1");
	strcpy(ifr.ifr_name, argv[1]);
	cprintf("%s wan_ifname=%s\n", __FUNCTION__, ifr.ifr_name);
	
	while( 1 )
	{
		//cprintf("T[%d]:%s::bLastLinkState[%d], bLastWanState:[%d]\n",
		//		GetSysUpTime(), __FUNCTION__, bLastLinkState, bLastWanState);
		//if( (bCurLinkState = getLinkState()) == LINK_DOWN )
		if( (bCurLinkState = getWanLinkState( s, &ifr, NULL )) == LINK_DOWN )  //we don't need to know speed
		{	// current Link down
			if( bLastLinkState != bCurLinkState ) 
			{ // Last Link state is LINK_INIT or LINK_UP
				if( bLastLinkState == LINK_INIT )
				{
					bLastLinkState = LINK_DOWN;
					KillLedProc( WAN_CONNECT_LEDPIN );
					SetLedTask( WAN_CONNECT_LEDPIN, LED_OFF );
					sleep_interval=SLEEP_SHORT;
					DEBUG_CheckWan("[T%d]:LINK_INIT==>LINK_DOWN\n", GetSysUpTime());
				}
				else if( bLastLinkState == LINK_UP )
				{ 
					//bLastLinkState = LINK_DOWN;
					bLastLinkState = LINK_DOWNING;
					KillLedProc( WAN_CONNECT_LEDPIN );
					SetLedTask( WAN_CONNECT_LEDPIN, LED_OFF );
					sleep_interval=SLEEP_SHORT;
					DEBUG_CheckWan("[T%d]:LINK_UP==>LINK_DOWNING\n", GetSysUpTime());
				}
				else if( bLastLinkState == LINK_DOWNING )
				{
					bLastLinkState = LINK_DOWN;
					KillLedProc( WAN_CONNECT_LEDPIN );
					SetLedTask( WAN_CONNECT_LEDPIN, LED_OFF );
					DEBUG_CheckWan("[T%d]:LINK_DOWNING==>LINK_DOWN, calling StopWan...\n", 
										GetSysUpTime());
					//wan_stop(NULL);  //to release if_ip
					cprintf("%s wan_stop...\n", ifr.ifr_name);
					wan_stop(ifr.ifr_name);
					sleep_interval=SLEEP_SHORT;
				}
			} 
			else
			{
				DEBUG_CheckWan("[T%d]:LINK_DOWN==>LINK_DOWN\n", GetSysUpTime());
			}
		} 
		else if( bCurLinkState == LINK_UP )// current Link UP
		{
			if( bLastLinkState != LINK_UP )
			{	// Last Link State is LINK_INIT or LINK_DOWN
				if( bLastLinkState == LINK_INIT )
				{
					bLastLinkState = LINK_UP;
					KillLedProc( WAN_CONNECT_LEDPIN );
					SetLedTask( WAN_CONNECT_LEDPIN, LED_BLINKING );
					sleep_interval=SLEEP_SHORT;
					DEBUG_CheckWan("[T%d]:LINK_INIT==>LINK_UP\n", GetSysUpTime());
				} 
				else
				{	//Last Link State is LINK_DOWN
					bLastLinkState = LINK_UP;
					DEBUG_CheckWan("[T%d]:LINK_DOWN==>LINK_UP\n", GetSysUpTime());

					switch( bLastWanState )
					{
					case WAN_INIT:
						bLastWanState = WAN_CONNECTING;
						KillLedProc( WAN_CONNECT_LEDPIN );
						SetLedTask( WAN_CONNECT_LEDPIN, LED_BLINKING );
						sleep_interval=SLEEP_SHORT;
						break;
					case WAN_CONNECTING:
						KillLedProc( WAN_CONNECT_LEDPIN );
						SetLedTask( WAN_CONNECT_LEDPIN, LED_BLINKING );
						sleep_interval=SLEEP_SHORT;
						break;
					case WAN_CONNECTED:
					case WAN_DISCONNECTED:
					case WAN_DOWN:
						bLastLinkState = LINK_INIT;
						bLastWanState = WAN_INIT;
						KillLedProc( WAN_CONNECT_LEDPIN );
						SetLedTask( WAN_CONNECT_LEDPIN, LED_BLINKING );
						//sys_restart();
						wan_restart(ifr.ifr_name);
						sleep_interval=SLEEP_SHORT;
						break;
					}
				}
			}
			else // LINK is always LINK_UP
			{
				DEBUG_CheckWan("[T%d]:LINK_UP==>LINK_UP [%d::%d], is_restart[%d]\n", GetSysUpTime(),
						(SLEEP_SHORT*sleep_short_nums), sleep_interval, g_sys_is_restarting);
				
				if( ((SLEEP_SHORT*sleep_short_nums) >= sleep_interval) || g_sys_is_restarting)
				{
					sleep_short_nums = 0;  //reset to 0
					bCurWanState = getWanState(ifr.ifr_name);
					//Once recved signal of SIGHUP(sys_is_restarting) from apply_cgi ==> 
					//change curr_wan_state to WAN_CONNECTING(to make wan_led blinking)
					if(g_sys_is_restarting)
					{
						DEBUG_CheckWan("curr_wan_st[%d], last_wan_st[%d]\n", bCurWanState, bLastWanState);
						g_sys_is_restarting = 0;   //reset to 0
						//once the last_wan_state is CONNECTED ==> set curr_wan_state to CONNECTING
						//it is not perfect but acceptable
						if(bLastWanState == WAN_CONNECTED)
							bCurWanState = WAN_CONNECTING;
					}
					
					//switch( bCurWanState = getWanState())
					switch( bCurWanState )
					{
					case WAN_INIT: 
					case WAN_CONNECTING:	// NO IP or logining
						if( bLastWanState != WAN_CONNECTING )
						{
							bLastWanState = WAN_CONNECTING;
							KillLedProc( WAN_CONNECT_LEDPIN );
							SetLedTask( WAN_CONNECT_LEDPIN, LED_BLINKING );
							sleep_interval=SLEEP_SHORT;
						}
						break;
					case WAN_CONNECTED:
						if( bLastWanState != WAN_CONNECTED )
						{
							bLastWanState = WAN_CONNECTED;
							KillLedProc( WAN_CONNECT_LEDPIN );
							SetLedTask( WAN_CONNECT_LEDPIN, LED_ON );
							sleep_interval=SLEEP_LONG;
						}
						break;
					case WAN_DISCONNECTED:	// Idle (NO IP), logout
						if( bLastWanState != WAN_DISCONNECTED )
						{
							bLastWanState = WAN_DISCONNECTED;
							KillLedProc( WAN_CONNECT_LEDPIN );
							SetLedTask( WAN_CONNECT_LEDPIN, LED_OFF );
							sleep_interval=SLEEP_SHORT;
						}
						break;
					case WAN_DOWN:	// Got IP, but not logout, but can't ping or query dns server
						switch( bLastWanState )
						{
						case WAN_INIT: // impossible
							cprintf("%s_impos_0::bCurWanState:%d, bLastWanState:%d\n", __FUNCTION__, bCurWanState, bLastWanState);
							break;
						case WAN_CONNECTING:	// NO IP or logining
							cprintf("%s_impos_1::bCurWanState:%d, bLastWanState:%d\n", __FUNCTION__, bCurWanState, bLastWanState);
							break;
						case WAN_CONNECTED:		// GotIP and Login
							bLastLinkState = LINK_INIT;
							bLastWanState = WAN_INIT;
							KillLedProc( WAN_CONNECT_LEDPIN );
							SetLedTask( WAN_CONNECT_LEDPIN, LED_BLINKING );
							sleep_interval=SLEEP_SHORT;
							//sys_restart();
							wan_restart(ifr.ifr_name);
							break;
						case WAN_DISCONNECTED:	// Idle, logout
							cprintf("%s_impos_2::bCurWanState:%d, bLastWanState:%d\n", __FUNCTION__, bCurWanState, bLastWanState);
							break;
						case WAN_DOWN:
							cprintf("%s_impos_3::bCurWanState:%d, bLastWanState:%d\n", __FUNCTION__, bCurWanState, bLastWanState);
							break;
						}
						break;
					case WAN_UNKNOW:
						//do nothing, keep the last state.
						break;
					}
				}
			}
		}
		else
		{
			DEBUG_CheckWan("%s::getWanLinkState ret (-1)\n", __FUNCTION__);
		}

		//always sleep short to check link_state every SLEEP_SHORT interval
		//sleep(sleep_interval);
		sleep(SLEEP_SHORT);
		sleep_short_nums ++;
	}
}
