#define WAN_DEV   "eth0"
#define LNA_DEV   "eth1"
#define _PATH_PROCNET_DEV 	 "/proc/net/dev"




struct lease_t {
	unsigned char chaddr[16];
    unsigned int yiaddr;	/* network order */
	unsigned int expires;	/* host order */
    char	hostname[32];
};


struct hosttbl {
	
	char ipaddr[20];
	char hwaddr[20];
	char hostname[32];
	long expire;
	char isactive;
	char sourcetype[10];       /* is Dhcp or Static */
	char intftype[10];   /* interface type, ethernet/usb/802.11/other */
};



struct new_net_device_stats
{
	unsigned int rx_packets;	/* total packets received       */
	unsigned int tx_packets;	/* total packets transmitted    */
	unsigned int rx_bytes;	/* total bytes received         */
	unsigned int tx_bytes;	/* total bytes transmitted      */
	unsigned int rx_errors;	/* bad packets received         */
	unsigned int tx_errors;	/* packet transmit problems     */
	unsigned int rx_dropped;	/* no space in linux buffers    */
	unsigned int tx_dropped;	/* no space available in linux  */
	unsigned int rx_multicast;	/* multicast packets received   */
	unsigned int rx_compressed;
	unsigned int tx_compressed;
	unsigned int collisions;

	/* detailed rx_errors: */
	unsigned int rx_length_errors;
	unsigned int rx_over_errors;	/* receiver ring buff overflow  */
	unsigned int rx_crc_errors;	/* recved pkt with crc error    */
	unsigned int rx_frame_errors;	/* recv'd frame alignment error */
	unsigned int rx_fifo_errors;	/* recv'r fifo overrun          */
	unsigned int rx_missed_errors;	/* receiver missed packet     */
	/* detailed tx_errors */
	unsigned int tx_aborted_errors;
	unsigned int tx_carrier_errors;
	unsigned int tx_fifo_errors;
	unsigned int tx_heartbeat_errors;
	unsigned int tx_window_errors;
};



struct wlan_state
{
	unsigned int rx_packets;
	unsigned int tx_packets;
	unsigned int rx_bytes;
	unsigned int tx_bytes;
};



struct l3_fwd_entry
{
	int enable;
	char status[10];
	char type[10];
	char destIP[20];
	char destmask[20];
	char sourceIP[20];
	char sourcemask[20];
	char gateway[20];
	char interface[20];
	int fwdmetric;
	int mtu;
};



extern int build_lan_hosts_info(int value);

extern int get_lan_rx_tx_status(struct new_net_device_stats *stats);
extern int get_wan_rx_tx_status(struct new_net_device_stats *stats);

extern int get_wlan_tx_rx_status(struct wlan_state *stats);
extern int get_l3_fwd_entry(int value);
extern int ip_ping_diagnostic();

#define MAX_LAN_HOST   		15

extern struct hosttbl lan_host_table[MAX_LAN_HOST];
extern int lan_host_num;

#define MAX_L3_FWD_ENTRY    10

extern struct l3_fwd_entry l3fwd_entry[MAX_L3_FWD_ENTRY];
extern int l3_fwd_num;


