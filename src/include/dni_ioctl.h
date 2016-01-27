
#ifndef __DNI_IOCTL_H__
#define __DNI_IOCTL_H__

/* for SIOCDEVPRIVATE */
enum 
{
    AG7100_IOCTL_START=0,

    AG7100_IOCTL_PORT_BASE_VLAN,
	AG7100_IOCTL_PROTO_BASE_VLAN ,
	AG7100_IOCTL_CONFIG_VLAN_TBL,
	AG7100_IOCTL_PORT_BASE_VLAN_STATUS ,
	AG7100_IOCTL_LIST_VLAN_RULE,
	AG7100_IOCTL_DEL_ALL_VLAN_TBL  

};

/**
 * For RTL865X_IOCTL_CPE5_IOSWORD , RTL865X_IOCTL_CPE5_IOGWORD
 */
typedef struct cpe5_ioc_cmd
{
	unsigned int address;
	unsigned int value;
	unsigned int data_len;

}cpe5_ioc_cmd_t;


typedef struct vlan_param
{
	char *ifname;
#define DNI_SET_VLAN_CMD_ADD (1)
#define DNI_SET_VLAN_CMD_REMOVE (0)

	unsigned int cmd; /* 0:Remove , 1: Add */

	unsigned int vid;
	unsigned int fid;
	unsigned int member;
	unsigned int untag_member;

} vlan_param_t;


/**
 * Ares.yeh comment:
 *  RTK network interface is binding with VLAN concept.
 *  Maximum network interface for 865xC is 8.
 * 
 *  RTL865X_IOCTL_SET_NET_IF
 */

typedef struct
{
	unsigned char  ether_addr[6]; /* Starting MAC address*/
	unsigned short  macAddrNumber; /* Number of MAC address*/
	unsigned short  vid; /* vid of this network interface */
	unsigned int    inAclStart, inAclEnd, outAclStart, outAclEnd;
	unsigned int    mtu;
	unsigned int    enableRoute:1, valid:1;
	//unsigned int flag_mask; 
	//unsigned char cmd; /* 0 : del netif   , 1: dd net if   2: modify netif */
} ioctl_netif_param_t;

	#define IOCTL_NETIF_ETHADDR (1 <<1)
	#define IOCTL_NETIF_VID (1 <<2)
	#define IOCTL_NETIF_INACL_START (1 <<3)
	#define IOCTL_NETIF_INACL_END  (1 <<4)
	#define IOCTL_NETIF_OUTACL_START  (1 <<5)
	#define IOCTL_NETIF_OUTACL_END (1 <<6)
	#define IOCTL_NETIF_MTU (1 <<7)
	#define IOCTL_NETIF_ENABLE_ROUTE (1 <<8)
	#define IOCTL_NETIF_DEL (0)
	#define IOCTL_NETIF_SET (1)
	#define IOCTL_NETIF_MOD (2)


typedef struct 
{
	unsigned char duplex:1;	 /* 1: full , 0:half*/
	unsigned char autoNeg:1; /* 1: Auto Negotiation Enable , 0 : Disable*/
	unsigned char linkState:1; /* 1: Link up , 0: Link down*/
	unsigned char linkSpeed10M:1;
	unsigned char linkSpeed100M:1;
	unsigned char linkSpeed1000M:1;
}ioctl_port_status_t;


typedef struct 
{
	 int portpri;
	 int dot1qpri;
	 int dscppri;
	 int aclpri;
	 int natpri;
}ioctl_priority_decision_weight_t;



enum 
{
	QUEUE_SCHED_POLICY_SP=0,
	QUEUE_SCHED_POLICY_WFQ 
};



int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);
extern int build_lan_hosts_info(int);



#endif /* __DNI_IOCTL_H__ */

