
/*
 *********************************************************
 *   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
 the contents of this file may not be disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of CyberTAN Inc.

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
*/

/* listen.c listen for any packet through an interface
 */

//#define MY_DEBUG
//#define MY_DEBUG1

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <dni_shutils.h>
#include <dni_bcmnvram.h>

// for PF_PACKET
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif


enum {L_FAIL, L_ERROR, L_UPGRADE, L_ESTABLISHED, L_SUCCESS};


#ifdef RELEASE_CODE
#define DEBUG(args...) do {;} while(0)
#else
#define DEBUG cprintf 
#endif

#define DEBUG1(args...) do {;} while(0)


struct iphdr       
  {      
    u_int8_t version;
    u_int8_t tos; 
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int8_t saddr[4];
    u_int8_t daddr[4];
  };    


struct EthPacket {
        u_int8_t dst_mac[6];
        u_int8_t src_mac[6];
        u_int8_t type[2];
	//struct iphdr ip;	//size=20
    u_int8_t version;
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int8_t saddr[4];
    u_int8_t daddr[4];
    //added by dvd.chen on 09/01/2004
    u_int8_t sport[2];
    u_int8_t dport[2];
    //end of dvd.chen

        u_int8_t data[1500-20]; 
};      


int read_interface(char *interface, int *ifindex, u_int32_t *addr, unsigned char *arp)
{
        int fd;
        struct ifreq ifr;
        struct sockaddr_in *sin;

        memset(&ifr, 0, sizeof(struct ifreq));
        if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
                ifr.ifr_addr.sa_family = AF_INET;
                strcpy(ifr.ifr_name, interface);

                if (addr) {
                        if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
                                sin = (struct sockaddr_in *) &ifr.ifr_addr;
                                *addr = sin->sin_addr.s_addr;
                                DEBUG("%s (our ip) = %s \n", ifr.ifr_name, inet_ntoa(sin->sin_addr));
                        } else {
                                DEBUG("SIOCGIFADDR failed!: \n");
                                return -1;
                        }
                }

                if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
                        DEBUG("adapter index %d \n", ifr.ifr_ifindex);
                        *ifindex = ifr.ifr_ifindex;
                } else {
                        DEBUG("SIOCGIFINDEX failed!: \n");
                        return -1;
                }
                if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
                        memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
                        DEBUG("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x \n",
                                arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
                } else {
                        DEBUG("SIOCGIFHWADDR failed!: \n");
                        return -1;
                }
        } else {
                DEBUG("socket failed!: \n");
                return -1;
        }
        close(fd);
        return 0;
}


int raw_socket(int ifindex)
{
        int fd;
        struct sockaddr_ll sock;

        DEBUG("Opening raw socket on ifindex %d\n", ifindex);
        if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
                DEBUG("socket call failed: \n");
                return -1;
        }

        sock.sll_family = AF_PACKET;
        sock.sll_protocol = htons(ETH_P_IP);
        sock.sll_ifindex = ifindex;
        if (bind(fd, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
                DEBUG("bind call failed: \n");
                close(fd);
                return -1;
        }

        return fd;

}

u_int16_t checksum(void *addr, int count)
{
        /* Compute Internet Checksum for "count" bytes
         *         beginning at location "addr".
         */
        register int32_t sum = 0;
        u_int16_t *source = (u_int16_t *) addr;

        while( count > 1 )  {
                /*  This is the inner loop */
                sum += *source++;
                count -= 2;
        }

        /*  Add left-over byte, if any */
        if( count > 0 ){
                /* Make sure that the left-over byte is added correctly both
                   with little and big endian hosts */
                u_int16_t tmp = 0;
                *(unsigned char *)(&tmp) = *(unsigned char *)source;
                sum += tmp;
        }
        /*  Fold 32-bit sum to 16 bits */
        while (sum>>16)
                sum = (sum & 0xffff) + (sum >> 16);

        return ~sum;
}

extern int g_need_listen;
int listen_interface(char *interface)
{
        int ifindex=0;
        fd_set rfds;
        struct EthPacket packet;
        struct timeval tv;
        int retval;
	unsigned char mac[6];
	static int fd;

        int bytes;
        u_int16_t check;

	//struct in_addr ipaddr, netmask;
	struct in_addr lan_ipaddr, lan_netmask;


        if (read_interface(interface, &ifindex,NULL, mac) < 0 )
                return L_ERROR;

        for (;;) {
/*
		if(check_action() != ACT_IDLE){	// Don't execute during upgrading
			return L_UPGRADE;
		}
		if(check_wan_link(0)){
			return L_ESTABLISHED;	
		}
*/
                tv.tv_sec = 100000;
                //tv.tv_sec = 30;  //dvd.chen
                tv.tv_usec=0;
                FD_ZERO(&rfds);
                fd=raw_socket(ifindex);
                if (fd<0) {
                        DEBUG("FATAL: couldn't listen on socket\n");
                        return L_ERROR;
                }
                if (fd>=0) FD_SET(fd, &rfds);
                if (tv.tv_sec >0) {
                        DEBUG("Waitting for select... \n");
                        retval=select(fd+1, &rfds, NULL, NULL, &tv);
                } else retval=0;

                if (retval==0) 
		  {
                        DEBUG("no packet recieved! \n\n");
			   close(fd);
                } 
		  else  
		  {
		       memset(&packet, 0, sizeof(struct EthPacket));
        		bytes = read(fd, &packet, sizeof(struct EthPacket));
        		if (bytes < 0) {
                		DEBUG("couldn't read on raw listening socket -- ignoring\n");
                		usleep(500000); /* possible down interface, looping condition */
				close(fd);
                		return L_FAIL;
        		}

        		if (bytes < (int) (sizeof(struct iphdr) )) {
                		DEBUG( "message too short, ignoring\n");
				close(fd);
                		return L_FAIL;
        		}

        		if (strncmp(mac,packet.dst_mac,6)) {
                		DEBUG( "dest mac not the router\n");
				close(fd);
                		return L_FAIL;
        		}

			//marked off by dvd.chen, check it below
			/*
        		if ( (inet_addr(nvram_safe_get("lan_ipaddr")) == *(u_int32_t *)packet.daddr) ) {
                		DEBUG( "dest ip equal to lan ipaddr\n");
                		return L_FAIL;
        		}
        		
                     DEBUG("lan_ipaddr=%x, packet.daddr=%x\n",inet_addr(nvram_safe_get("lan_ipaddr")),*(u_int32_t *)packet.daddr);
                    	*/

			//for (i=0; i<34;i++) {
			//	if (i%16==0) printf("\n");
			//	printf("%02x ",*( ( (u_int8_t *)packet)+i) );
			//}
			//printf ("\n");

                     DEBUG1("%02X%02X%02X%02X%02X%02X,%02X%02X%02X%02X%02X%02X,%02X%02X\n",
                                packet.dst_mac[0], packet.dst_mac[1], packet.dst_mac[2],
                                packet.dst_mac[3], packet.dst_mac[4], packet.dst_mac[5],
                                packet.src_mac[0], packet.src_mac[1], packet.src_mac[2],
                                packet.src_mac[3], packet.src_mac[4], packet.src_mac[5],
                                packet.type[0],packet.type[1]);

        		DEBUG("ip.version = %x", packet.version);
        		DEBUG("ip.tos = %x", packet.tos);
			DEBUG("ip.tot_len = %x", packet.tot_len);
		       DEBUG("ip.id = %x", packet.id);
        		DEBUG("ip.ttl= %x", packet.ttl);
        		DEBUG("ip.protocol= %x", packet.protocol);
        		DEBUG("ip.check=%04x", packet.check);
			DEBUG1("ip.saddr=%08x", *(u_int32_t *)&(packet.saddr));
			DEBUG1("ip.daddr=%08x", *(u_int32_t *)&(packet.daddr));

        		if (*(u_int16_t *)packet.type == 0x0800) {
                		DEBUG( "not ip protocol");
				close(fd);
                		return L_FAIL;
        		} 

        		/* ignore any extra garbage bytes */
        		bytes = ntohs(packet.tot_len);

        		/* check IP checksum */
        		check = packet.check;
        		packet.check = 0;

        		if (check != checksum(&(packet.version), sizeof(struct iphdr))) {
                		DEBUG("bad IP header checksum, ignoring\n");
				DEBUG("check received = %X, should be %X",check, checksum(&(packet.version), sizeof(struct iphdr)));
				close(fd);
                		return L_FAIL;
        		}

        		DEBUG( "%s:: Packet Received\n", __FUNCTION__);

			//no need to check wan_ip and wan_netmask
			/*			
			if(nvram_match("wan_proto", "pptp"))
				inet_aton(nvram_safe_get("pptp_server_ip"), &ipaddr);
			else
				inet_aton(nvram_safe_get("wan_ipaddr"), &ipaddr);
			
		       inet_aton(nvram_safe_get("wan_netmask"), &netmask);
			DEBUG("gateway=%08x", ipaddr.s_addr);
			DEBUG("netmask=%08x", netmask.s_addr);
			*/
			//added by dvd.chen on 09/01/2004
			inet_aton(nvram_safe_get("lan_ipaddr"), &lan_ipaddr);
			inet_aton(nvram_safe_get("lan_netmask"), &lan_netmask);

			//if it is dns packet to lan_ip(dns_proxy), means this packet wanna go out
			if( (inet_addr(nvram_safe_get("lan_ipaddr")) == *(u_int32_t *)packet.daddr)
				&& (packet.protocol == 17) && (ntohs(*(u_int16_t *)packet.dport)) == 53 )
			{
				DEBUG( "dns proxy packet, let go out\n");
				break;  //return L_SUCCESS
			}
			else
			{
				DEBUG( "packet.protocol:%d, packet.daddr:%x, packet.dport:%x\n", 
						packet.protocol, ntohl(*(u_int32_t *)packet.daddr), ntohs(*(u_int16_t *)packet.dport) );
			}
			//end of dvd.chen
			
			//modified by dvd.chen on 09/01/2004
			//if the packet's dst_addr is going to lan network, then return L_FAIL, otherwise, it is trying to go out
			//seems that the pkt to other host in aln won't come here, but check it anyway.
			//if((ipaddr.s_addr & netmask.s_addr) != (*(u_int32_t *)&(packet.daddr) & netmask.s_addr))
			if( (*(u_int32_t *)&(packet.daddr) & lan_netmask.s_addr) == (lan_ipaddr.s_addr & lan_netmask.s_addr) ) 
			{
				DEBUG( "%s:: packet is going to lan_network\n", __FUNCTION__);
				close(fd);
				return L_FAIL;
			}
			
        		break;
		}
	}
	close(fd);
	return L_SUCCESS;
}

int listen_lan_interface()
{
	return listen_interface(nvram_get("lan_ifname"));
}

