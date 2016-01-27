#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <linux/sockios.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ctype.h>
#include <net/if_arp.h>
#include <signal.h>

#include <dni_shutils.h>
#include <dni_devutils.h>



extern char *nvram_get(const char *name);
extern int nvram_set(const char *name, const char *val);

extern char* itos (int n);


struct hosttbl lan_host_table[MAX_LAN_HOST];
int lan_host_num = 0;



struct l3_fwd_entry l3fwd_entry[MAX_L3_FWD_ENTRY];
int l3_fwd_num = 0;



/* 
 * Construct host PC at lan site, record each host Info 
 */

int build_lan_hosts_info(int vlaue) 
{
	FILE *fp;
	int pid;
	struct in_addr addr;
	struct lease_t lease;
	char *ipaddr, mac[20]="", expires_time[50]="", tmp_mac[20];
	char tmpipaddr[20];
	unsigned long expires;
	int i,j;
	int dhcp_use=0;
	char tmp[100];
	unsigned char tok1[]=" ";
    unsigned char buf1[30];

	pid = get_task_pid("udhcpd");
	if (pid != 0) {	   
		kill(pid, SIGUSR1);    
		sleep(1);		/* waiting writed completed */
	}	

	lan_host_num = 0;
	for (i=0 ; i<MAX_LAN_HOST ; i++)
	{
		lan_host_table[i].expire = -1;
		memset(lan_host_table[i].hostname, 0, sizeof(lan_host_table[i].hostname));
		memset(lan_host_table[i].hwaddr, 0, sizeof(lan_host_table[i].hwaddr));
		memset(lan_host_table[i].ipaddr, 0, sizeof(lan_host_table[i].ipaddr));
		memset(lan_host_table[i].sourcetype, 0, sizeof(lan_host_table[i].sourcetype));
			memset(lan_host_table[i].intftype, 0, sizeof(lan_host_table[i].intftype));
		lan_host_table[i].isactive = 0;
	}


	/* check PC using dhcp mode at lan site */
	if ((fp = fopen("/var/tmp/udhcpd.leases", "r")) != NULL) {  

		while (fread(&lease, sizeof(lease), 1, fp)){ 
			addr.s_addr = lease.yiaddr;
			ipaddr = inet_ntoa(addr);
			expires = ntohl(lease.expires);
			//expires = lease.expires;
			for (i = 0; i < 6; i++) {    
				sprintf(mac+strlen(mac),"%02X", lease.chaddr[i]);
				if (i != 5) sprintf(mac+strlen(mac),":");
			}    
			mac[17] = '\0';

			if (!expires){   
				continue;
				strcpy(expires_time,"expired");
			}
			else {
					if (expires > 60*60*24) {
						sprintf(expires_time+strlen(expires_time),"%ld days, ",expires / (60*60*24));
						expires %= 60*60*24;
					}
					if (expires > 60*60) {
						sprintf(expires_time+strlen(expires_time),"%02ld:",expires / (60*60));	// hours
						expires %= 60*60;
					}
					else{
						sprintf(expires_time+strlen(expires_time),"00:");	// no hours
					}
					if (expires > 60) {
						sprintf(expires_time+strlen(expires_time),"%02ld:",expires / 60);	// minutes
						expires %= 60;
					}
					else{
						sprintf(expires_time+strlen(expires_time),"00:");	// no minutes
					}

					sprintf(expires_time+strlen(expires_time),"%02ld:",expires);		// seconds

					expires_time[strlen(expires_time)-1]='\0';
				}   	

			strcpy(lan_host_table[lan_host_num].ipaddr, ipaddr);
			strcpy(lan_host_table[lan_host_num].hwaddr, mac);
			strcpy(lan_host_table[lan_host_num].hostname, lease.hostname);
			strcpy(lan_host_table[lan_host_num].sourcetype, "dhcp");
			strcpy(lan_host_table[lan_host_num].intftype, "ethernet");
			lan_host_table[lan_host_num].expire = expires;
			lan_host_table[lan_host_num].isactive = 0;

            lan_host_num ++;
			dhcp_use ++;

			if (lan_host_num > MAX_LAN_HOST){
				   fclose(fp);
				   return -1;
			}
		}
	}
	fclose(fp);

	i = 0;
	/* check any PC using static IP at lan site */
	if ((fp = fopen("/proc/net/arp", "r")) != NULL) { 
		while(fgets(tmp, 100, fp) != NULL){
			if (i > 0){

				sprintf(buf1, "%s", strtok(tmp, tok1));   /* IP address */

				if (dhcp_use > 0){
					/* have host pc using dhcp */
					for (j=0 ; j<dhcp_use ; j++){
	
						if(strcmp(buf1, lan_host_table[j].ipaddr) == 0){
	
							lan_host_table[j].isactive = 1;
						}
						else {
							strcpy(tmpipaddr, buf1);
	
							sprintf(buf1, "%s", strtok(NULL, tok1));    /* HW type */
							sprintf(buf1, "%s", strtok(NULL, tok1));    /* flag */
							sprintf(buf1, "%s", strtok(NULL, tok1));    /* HW address */
	
							strcpy(tmp_mac,buf1);
	
							sprintf(buf1, "%s", strtok(NULL, tok1));    /* Mask */
							sprintf(buf1, "%s", strtok(NULL, tok1));    /* Device */
	 
							if (strncmp(buf1, "eth0", 4) != 0){

								strcpy(lan_host_table[lan_host_num].ipaddr, tmpipaddr);
								strcpy(lan_host_table[lan_host_num].hwaddr, tmp_mac);
								strcpy(lan_host_table[lan_host_num].sourcetype, "static");
								lan_host_table[lan_host_num].isactive = 1;
								lan_host_table[lan_host_num].expire = 0; /* follow tr-069, if static ip, then leasetime is 0 */
	
								lan_host_num ++;
								if (lan_host_num > MAX_LAN_HOST){
									fclose(fp);
									return -1;
								}
							}	
						}
					}
				}
				else {
					/* no host pc using dhcp */
					strcpy(tmpipaddr, buf1);
	
					sprintf(buf1, "%s", strtok(NULL, tok1));    /* HW type */
					sprintf(buf1, "%s", strtok(NULL, tok1));    /* flag */
					sprintf(buf1, "%s", strtok(NULL, tok1));    /* HW address */
	
					strcpy(tmp_mac,buf1);
	
					sprintf(buf1, "%s", strtok(NULL, tok1));    /* Mask */
					sprintf(buf1, "%s", strtok(NULL, tok1));    /* Device */

					if (strncmp(buf1, "eth0", 4) != 0){

						strcpy(lan_host_table[lan_host_num].ipaddr, tmpipaddr);
						strcpy(lan_host_table[lan_host_num].hwaddr, tmp_mac);
						strcpy(lan_host_table[lan_host_num].intftype, "ethernet");
						strcpy(lan_host_table[lan_host_num].sourcetype, "static");
						lan_host_table[lan_host_num].isactive = 1;
						lan_host_table[lan_host_num].expire = 0; /* follow tr-069, if static ip, then leasetime is 0 */
	
						lan_host_num ++;
						if (lan_host_num > MAX_LAN_HOST){
							fclose(fp);
							return -1;
						}
					}	

				}
		     }
			 i ++;
		}
	}
	else printf("\n can't open arp");
	fclose(fp);
#if 1
	for (i=0 ; i<lan_host_num ; i++){
		printf("\nnum = %d", i+1);
		printf("\nIP Address = %s", lan_host_table[i].ipaddr);
		printf("\nmac = %s", lan_host_table[i].hwaddr);
		printf("\nlease time = %d", lan_host_table[i].expire);
		printf("\nhostname = %s", lan_host_table[i].hostname);	
		printf("\ntype = %s", lan_host_table[i].sourcetype);	
		printf("\nis_active = %d", lan_host_table[i].isactive);	
		printf("\ninterface type = %s", lan_host_table[i].intftype);
		printf("\n");
	}
#endif
	return 0;
}



/*
   bp format likes followings : 
                                 ( receive )                                                ( transfer )
   interface bytes packets errs drop fifo frame compressed multicast  bytes packets errs drop fifo colls carrier compressed  
*/

void netdev_decode(char *bp, struct new_net_device_stats *stats )
{
	char tok[]=" ";
    char buf[20];  

    //printf("\nbp = %s\n", bp);

    sprintf(buf, "%s", strtok(bp, tok)); 
	sprintf(buf, "%s", strtok(NULL, tok)); 	
    stats->rx_bytes = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 	
    stats->rx_packets = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 	
    stats->rx_errors = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->rx_dropped = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->rx_fifo_errors = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->rx_frame_errors = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->rx_compressed = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->rx_multicast = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_bytes = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_packets = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_errors = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_dropped = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_fifo_errors = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->collisions = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_carrier_errors = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok)); 
    stats->tx_compressed = atoi(buf);
}



/* use /proc/net/dev to get lan tx/rx status */

int get_lan_rx_tx_status(struct new_net_device_stats *stats)
{
   FILE *fh;
   char buf[512], ptr[20], devinfo[512];
   char tok[]=" ";

   memset(stats, 0, sizeof(struct new_net_device_stats));

   fh = fopen(_PATH_PROCNET_DEV, "r");

   if (!fh) {
	  printf("\nWarning: cannot open %s. Limited output.", _PATH_PROCNET_DEV);
	  return 0;
   }

   fgets(buf, sizeof buf, fh);	/* eat line */
   fgets(buf, sizeof buf, fh);


   while (fgets(buf, sizeof buf, fh)) {

	   strcpy(devinfo, buf);
	   sprintf(ptr, "%s", strtok(buf, tok)); 

       if (strncmp(ptr, LNA_DEV, 4) == 0){
		   netdev_decode(devinfo, stats);
		   break;
	   }
   }

#if 0
   printf("\nrx_bytes = %d", stats->rx_bytes);
   printf("\nrx_packets = %d", stats->rx_packets);
   printf("\nrx_errors = %d", stats->rx_errors);
   printf("\nrx_dropped = %d", stats->rx_dropped);
   printf("\nrx_fifo_errors = %d", stats->rx_fifo_errors);
   printf("\nrx_frame_errors = %d", stats->rx_frame_errors);
   printf("\nrx_multicast = %d", stats->rx_multicast);
   printf("\ntx_bytes = %d", stats->tx_bytes);
   printf("\ntx_packets = %d", stats->tx_packets);
   printf("\ntx_errors = %d\n", stats->tx_errors);
#endif

   fclose(fh);

   return 1;
}


/* use /proc/net/dev to get wan tx/rx status */

int get_wan_rx_tx_status(struct new_net_device_stats *stats)
{
   FILE *fh;
   char buf[512], ptr[20], devinfo[512];
   char tok[]=" ";

   memset(stats, 0, sizeof(struct new_net_device_stats));

   fh = fopen(_PATH_PROCNET_DEV, "r");
   if (!fh) {
	  printf("\nWarning: cannot open %s. Limited output.", _PATH_PROCNET_DEV);
	  return 0;
   }
   fgets(buf, sizeof buf, fh);	/* eat line */
   fgets(buf, sizeof buf, fh);

   while (fgets(buf, sizeof buf, fh)) {

	   strcpy(devinfo, buf);
	   sprintf(ptr, "%s", strtok(buf, tok)); 
       if (strncmp(ptr, WAN_DEV, 4) == 0){
		   netdev_decode(devinfo, stats);
		   break;
	   }
   }

#if 0
   printf("\nrx_bytes = %d", stats->rx_bytes);
   printf("\nrx_packets = %d", stats->rx_packets);
   printf("\nrx_errors = %d", stats->rx_errors);
   printf("\nrx_dropped = %d", stats->rx_dropped);
   printf("\nrx_fifo_errors = %d", stats->rx_fifo_errors);
   printf("\nrx_frame_errors = %d", stats->rx_frame_errors);
   printf("\nrx_multicast = %d", stats->rx_multicast);
   printf("\ntx_bytes = %d", stats->tx_bytes);
   printf("\ntx_packets = %d", stats->tx_packets);
   printf("\ntx_errors = %d\n", stats->tx_errors);
#endif

   fclose(fh);

   return 1;
}




int get_wlan_tx_rx_status(struct wlan_state *stats)
{
	FILE *fp;

	char tmp[50];
	unsigned char tok1[]=" ";
    unsigned char buf1[30], ptr[30];	
	char tx_bytes[]="tx_bytes:";
	char tx_pkts[]="tx_packets:";
	char rx_bytes[]="rx_bytes:";
	char rx_pkts[]="rx_packets:";

	if ((fp = fopen("/proc/wlan0/stats", "r")) != NULL) { 

		while(fgets(tmp, 50, fp) != NULL){

			sprintf(buf1, "%s", strtok(tmp, tok1));
			strcpy(ptr, buf1);
			if (strncmp(ptr, tx_bytes, sizeof(tx_bytes)) == 0){
			
				sprintf(buf1, "%s", strtok(NULL, tok1));
				stats->tx_bytes = atoi(buf1);
				 //printf("%s:%d", ptr, atoi(buf1));
			}
			else if (strncmp(ptr, tx_pkts, sizeof(tx_pkts)) == 0){
					 sprintf(buf1, "%s", strtok(NULL, tok1));
					 stats->tx_packets = atoi(buf1);
					// printf("%s:%d", ptr, atoi(buf1));
			}
			else if (strncmp(ptr, rx_bytes, sizeof(rx_bytes)) == 0){
					 sprintf(buf1, "%s", strtok(NULL, tok1));
					 stats->rx_bytes = atoi(buf1);
					//  printf("%s:%d", ptr, atoi(buf1));
			}
			else if (strncmp(ptr, rx_pkts, sizeof(rx_pkts)) == 0){
					 sprintf(buf1, "%s", strtok(NULL, tok1));
					 stats->rx_packets = atoi(buf1);
					 //  printf("%s:%d", ptr, atoi(buf1));
			}
		}
	}
	return 1;
}





int get_l3_fwd_entry(int value)
{
	FILE *fp;
	int i=0, j;
	char tmp[150];
	char tok1[]=" ";
    char buf1[100], ptr[20];


	for (i=0 ; i<MAX_L3_FWD_ENTRY ; i++){
		memset(l3fwd_entry[i].destIP, 0, sizeof(l3fwd_entry[i].destIP));
		memset(l3fwd_entry[i].destmask, 0, sizeof(l3fwd_entry[i].destmask));
		l3fwd_entry[i].enable = 0;
		l3fwd_entry[i].fwdmetric = -1;
		memset(l3fwd_entry[i].gateway, 0, sizeof(l3fwd_entry[i].gateway));
		memset(l3fwd_entry[i].interface, 0, sizeof(l3fwd_entry[i].interface));
		l3fwd_entry[i].mtu = 1500;
		strcpy(l3fwd_entry[i].sourceIP, "0.0.0.0");
		strcpy(l3fwd_entry[i].sourcemask, "0.0.0.0");
		strcpy(l3fwd_entry[i].type, "Network");
		strcpy(l3fwd_entry[i].status, "disable");

	}
	i = 0;
	l3_fwd_num = 0;

    system("route -n > /tmp/route");

	if ((fp = fopen("/tmp/route", "r")) != NULL) { 
		
		while(fgets(tmp, 150, fp) != NULL){	
			if (i > 1){

                sprintf(buf1, "%s", strtok(tmp, tok1));     /* destination IPaddress */
				strcpy(l3fwd_entry[l3_fwd_num].destIP, buf1);
				//printf("\nbuf1 = %s, len = %d", buf1, strlen(buf1)); 

				sprintf(buf1, "%s", strtok(NULL, tok1));	   /* Gateway */
				strcpy(l3fwd_entry[l3_fwd_num].gateway, buf1);
               // printf("\nbuf1 = %s, len = %d", buf1, strlen(buf1));

				sprintf(buf1, "%s", strtok(NULL, tok1));     /* Genmask */
				strcpy(l3fwd_entry[l3_fwd_num].destmask, buf1);

				sprintf(buf1, "%s", strtok(NULL, tok1));      /* flag */
                strcpy(ptr, buf1);
				//printf("\nptr = %s", ptr);
				for (j=0 ; j<strlen(ptr) ; j++){
					//printf("\nptr[i] = %c", ptr[j]);
					if (ptr[j] == 'U'){
						strcpy(l3fwd_entry[l3_fwd_num].status, "enable");
						l3fwd_entry[l3_fwd_num].enable = 1;
					}
					if (ptr[j] == 'G'){
						strcpy(l3fwd_entry[l3_fwd_num].type, "Default");
					}
					if (ptr[j] == 'H'){
						strcpy(l3fwd_entry[l3_fwd_num].type, "Host");
					}
				}
				l3_fwd_num ++;
			}
			i ++;
		}
	}
	fclose(fp);
	system("rm -rf /tmp/route");

#if 0
	for (i=0 ; i<l3_fwd_num ; i++){
		printf("\n%d enable = %d", i+1,l3fwd_entry[i].enable);
		printf("\n%d status = %s", i+1,l3fwd_entry[i].status);
		printf("\n%d type = %s", i+1,l3fwd_entry[i].type);
		printf("\n%d destIP = %s", i+1,l3fwd_entry[i].destIP);
		printf("\n%d destmask = %s", i+1,l3fwd_entry[i].destmask);
		printf("\n%d sourceIP = %s", i+1,l3fwd_entry[i].sourceIP);
		printf("\n%d sourcemask = %s", i+1,l3fwd_entry[i].sourcemask);
		printf("\n%d gateway = %s", i+1,l3fwd_entry[i].gateway);
		printf("\n%d interface = %s", i+1,l3fwd_entry[i].interface);
		printf("\n%d fwdmetric = %d", i+1,l3fwd_entry[i].fwdmetric);
		printf("\n%d mtu = %d", i+1,l3fwd_entry[i].mtu);
		printf("\n");
	}	
	
#endif
	printf("\n");
	return 1;
}




int ip_ping_diagnostic(int vlaue)
{
   FILE *fp;
   char tmp[150];
   int counter = 0;
   char buf1[100], buf2[100];
   char res1[20], res2[20];
   char tok1[]=" ", tok2[]="/";
   char cmd[30];

   unsigned int transmitpkt=0,successpkt=0, failurepkt=0;
   unsigned int mintime=0, maxtime=0, avetime=0;


   system("rm -rf /var/run/ping.txt");    // modify by dungjiun to keep only the last record in ping.txt	

   if (nvram_get("ipping_host") == NULL){
	   printf("\nno host value (ip ping diagnostic failed .... !!!)\n");
   }
   else if (nvram_get("ipping_host") != NULL){

	   if (nvram_get("ipping_numberofrepetitions") != NULL && nvram_get("ipping_datablocksizes") != NULL){
		    sprintf(cmd, "ping -f -c %s -s %s %s", nvram_get("ipping_numberofrepetitions"), 
						nvram_get("ipping_datablocksizes"), nvram_get("ipping_host"));
	   }
	   else if (nvram_get("ipping_numberofrepetitions") == NULL && nvram_get("ipping_datablocksizes") != NULL){
		   sprintf(cmd, "ping -f -s %s %s", nvram_get("ipping_datablocksizes"), nvram_get("ipping_host")); 
	   }
	   else if (nvram_get("ipping_numberofrepetitions") != NULL && nvram_get("ipping_datablocksizes") == NULL){
		   sprintf(cmd, "ping -f -c %s %s", nvram_get("ipping_numberofrepetitions"), nvram_get("ipping_host"));
	   }
	   else if (nvram_get("ipping_numberofrepetitions") == NULL && nvram_get("ipping_datablocksizes") == NULL){
		   sprintf(cmd, "ping -f %s", nvram_get("ipping_host")); 
	   }
	   //printf("\ncmd = %s\n", cmd);
	   system(cmd);
   }	

   counter = atoi(nvram_get("ipping_numberofrepetitions"));
   
   sleep(2);

   while(1){
	   /* wait ping action finished */
	   if (get_task_pid("ping") == 0)   break;
	   sleep(1);
   }

   fp = fopen("/var/run/ping.txt","r");
   if (fp == NULL){
	      printf("\ncan't open /var/run/ping.txt");
		  return -1;
   }
   else{
       counter += 3; /* count PING xxx.xxx.xxx.xxx (xxx.xxx.xxx.xxx): xx data bytes

							  --- xxx.xxx.xxx.xxx ping statistics ---                */
	   while(fgets(tmp, 150, fp) != NULL){	

		   if (counter < 1){
			   if (counter == 0){             /* at  x packets transmitted, x packets received, x% packet loss line */
				   strcpy(buf1, tmp);
				   sprintf(res1, "%s", strtok(buf1, tok1));		/* x */
				   //printf("\ntransmit = %s", res1);
				   transmitpkt = atoi(res1);
                   sprintf(res1, "%s", strtok(NULL, tok1));	    /* packets */
				   sprintf(res1, "%s", strtok(NULL, tok1));		/* transmitted, */
				   sprintf(res1, "%s", strtok(NULL, tok1));		/* x */
				   //printf("\nsuccess = %s", res1);
				   successpkt = atoi(res1);
				   failurepkt = transmitpkt - successpkt;
				   
			   }
			   else if (counter == -1){		 /* at round-trip min/avg/max = 0.0/8.0/40.0 ms */
				   strcpy(buf2, tmp);
				   sprintf(res1, "%s", strtok(buf2, tok1));   /*  round-trip */
				   sprintf(res1, "%s", strtok(NULL, tok1));	  /* min/avg/max */
				   sprintf(res1, "%s", strtok(NULL, tok1));   /* =           */
				   sprintf(res1, "%s", strtok(NULL, tok1));	  /* 0.0/8.0/40.0 */
				   //printf("\ntime = %s", res1);

				   sprintf(res2, "%s", strtok(res1, tok2));
				   mintime = atoi(res2);
				   sprintf(res2, "%s", strtok(NULL, tok2));
				   avetime = atoi(res2);
				   sprintf(res2, "%s", strtok(NULL, tok2));
				   maxtime = atoi(res2);
				   
			   }			  
		   }
		   counter --;
	    }
	    nvram_set("ipping_successcount", itos(successpkt));
		nvram_set("ipping_failurecount", itos(failurepkt));
	    nvram_set("ipping_minresponsetime", itos(mintime));
		nvram_set("ipping_avresponsetime", itos(avetime));
		nvram_set("ipping_maxresponsetime", itos(maxtime));

   }
 
   return 1;
} 




int findstr (char * sub_str, char * str)
{
   int i = -1, str_index;
   int sub_str_len, str_len;
   if (str == NULL || sub_str == NULL) return -1;
   sub_str_len = strlen(sub_str);
   str_len = strlen(str);

   /* sub_str has to be smaller than str */
   if (sub_str_len > str_len) return -1;

   /* look through str for sub_str, stopping at end of string or when the length
           of sub_str exceeds the remaining number of characters to be searched */ 
   while (str[++i] != '\0' && (str_len - i + 1) > sub_str_len) 
   {
      for (str_index = 0; str_index < sub_str_len; ++str_index)
      {
           /* definitely not at location i */
	   if (str[i + str_index] != sub_str[str_index])
		   break;
           /* if last letter matches, then we know the whole thing matched */
	   else if (str_index == sub_str_len - 1)
	     {
		   return i;
	     }
      }
   }
   /* sub_str not present in str */
   return -1;
}



#if 0
/* get wan interface port status : it's enable as return 1 */
int get_wan_if_status()
{
	char *if_status=NULL;
	
	web_get_port_status(&if_status);
	if(findstr("Showtime", if_status) >=0 )
		return 1;
	else
		return 0;
}
#endif


