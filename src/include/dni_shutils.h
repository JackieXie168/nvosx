/*
 * Shell-like utility functions
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: dni_shutils.h,v 1.1 2007/04/17 10:13:27 timo Exp $
 */

#ifndef _shutils_h_
#define _shutils_h_
#include	<string.h>
#include	<netinet/in.h>

/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
extern char * fd2str(int fd);

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
extern char * file2str(const char *path);

/*
 * Write file with input string
 * @param	path	path to file
 * @param2	string to be written
 * @return	written length if success, -1 otherwise
 */
extern int str2file(char *path, char *string);
#if 0
/* 
 * Waits for a file descriptor to become available for reading or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
extern int waitfor(int fd, int timeout);
#endif //#if 0
/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
extern int _eval(char *const argv[], char *path, int timeout, pid_t *ppid);

#if 0
/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @return	stdout of executed command or NULL if an error occurred
 */
extern char * _backtick(char *const argv[]);

/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file
 * @return	0 on success and errno on failure
 */
extern int kill_pidfile(char *pidfile);

/*
 * fread() with automatic retry on syscall interrupt
 * @param	ptr	location to store to
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully read
 */
extern int safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param	ptr	location to read from
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully written
 */
extern int safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
#endif
/*
 * Convert Ethernet address string representation to binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
extern int ether_atoe(char *a, unsigned char *e);

/*
 * Convert Ethernet address binary data to string representation
 * @param	e	binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @return	a
 */
extern char * ether_etoa(const unsigned char *e, char *a);

/*
 * Concatenate two strings together into a caller supplied buffer
 * @param	s1	first string
 * @param	s2	second string
 * @param	buf	buffer large enough to hold both strings
 * @return	buf
 */
static inline char * strcat_r(const char *s1, const char *s2, char *buf)
{
	strcpy(buf, s1);
	strcat(buf, s2);
	return buf;
}	

extern char * trim(char *s);

/* trim leading and trailing white space */

extern int wan_connect_pooling();
extern int ping( char *strPingAddr, int iSec, int iUsec, int iTryTimes);
extern int queryDNS(int iSec, int iUsec, int iTryTimes);
extern unsigned long GetSysFreeMem();
extern long GetSysUpTime();
extern unsigned short getlocalfreeport(char *wan0_ip, unsigned short port_start, 
									unsigned port_end);
extern int getWanState(char *ifname);
extern int task_is_running(char *strTask);
extern int getWanLinkState( int s, void /* struct ifreq */ *pIfreq, int *iSpeed );

extern void log_WAN_connect(char *);

extern int get_task_pid(char *strTask);

// Get common information of the specified interface
/* Note that <netinet/in.h> defines struct in_addr
struct in_addr
{
	unsigned long s_addr;
};

All items are network order except flag argument.
For those items you don't want to access, NULL pointer is OK.
*/
int ifConfigGet
	(
	char *ifname,        				/* name of interface, i.e. ei0 */
	struct in_addr *inAddr,      		/* buffer for Internet address */
	struct in_addr *inNetmask,		/* buffer for interface netmask */
	struct in_addr *inPPPDstaAddr,		/* buffer for destination peer address */
	struct in_addr *inBroadcastAddr,	/* buffer for broadcast address */
	short int *sFlags     				/* buffer for interface flags */
	);

/* Check for a blank character; that is, a space or a tab */
#define isblank(c) ((c) == ' ' || (c) == '\t')

/* Strip trailing CR/NL from string <s> */
#define chomp(s) ({ \
	char *c = (s) + strlen((s)) - 1; \
	while ((c > (s)) && (*c == '\n' || *c == '\r')) \
		*c-- = '\0'; \
	s; \
})

/* Simple version of _backtick() */
#define backtick(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_backtick(argv); \
})

/* Simple version of _eval() (no timeout and wait for child termination) */
#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, ">/dev/console", 0, NULL); \
})

/* Copy each token in wordlist delimited by space into word */
#define foreach(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, " ")], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '); \
	     strlen(word); \
	     next = next ? &next[strspn(next, " ")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '))

/* Return NUL instead of NULL if undefined */
#define safe_getenv(s) (getenv(s) ? : "")

/* Print directly to the console */
#ifdef	RELEASE_CODE
#define cprintf(fmt, args...)
#else
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
#endif // #ifdef	RELEASE_CODE

/* Debug print */
#ifdef DEBUG
#define dprintf(fmt, args...) cprintf("%s: " fmt, __FUNCTION__, ## args)
#else
#define dprintf(fmt, args...)
#endif

//dvd.chen for return value of getLinkState() and getWanState()
#define LINK_INIT	0
#define LINK_DOWN	1
#define LINK_UP		2
#define LINK_DOWNING	3

#define WAN_UNKNOW		(-1)
#define WAN_INIT			0
#define WAN_DISCONNECTED	1
#define WAN_DOWN 			2
#define WAN_CONNECTING	3
#define WAN_CONNECTED		4

#define sys_restart() kill(1, SIGHUP)

#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"
#define ETH_ALEN	6
#define ETH_P_ARP	0x0806
#define ETH_P_IP	0x0800
#define LOG_ERR	"error"
struct ethhdr1 {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
} __attribute__((packed));

struct arpMsg {
	struct ethhdr1 ethhdr;	 		/* Ethernet header */
	u_short htype;				/* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;				/* protocol type (must be ETH_P_IP) */
	u_char  hlen;				/* hardware address length (must be 6) */
	u_char  plen;				/* protocol address length (must be 4) */
	u_short operation;			/* ARP opcode */
	u_char  sHaddr[6];			/* sender's hardware address */
	u_char  sInaddr[4];			/* sender's IP address */
	u_char  tHaddr[6];			/* target's hardware address */
	u_char  tInaddr[4];			/* target's IP address */
	u_char  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
};
/* ******************************************************* */
// LED relevant GPIO definitions
/* ******************************************************* */

#define PREG_BASE		0xB8400000	// Base address to Peripheral Reg
#define PER_BASE 		PREG_BASE
#define TEST_LEDPIN		15
#define WAN_CONNECT_LEDPIN	16
#define LED_ON			0
#define LED_OFF			1
#define LED_BLINKING	2



/**********************************************************/
/* GPIO registers                                                                                 */
/* add by alfa 5/18,2005 */
/**********************************************************/
#define GPIO_DATA_8to0_REG		((volatile unsigned int *)(PER_BASE + 0xE0)) 
#define GPIO_CFG1_REG			((volatile unsigned int *)(PER_BASE + 0xE4))
#define GPIO_CFG2_REG			((volatile unsigned int *)(PER_BASE + 0xE8))
#define GPIO_CFG3_REG			((volatile unsigned int *)(PER_BASE + 0x170))
#define GPIO_DATA_15to9_REG	((volatile unsigned int *)(PER_BASE + 0x174))
#define GPIO_DATA_23to16_REG	((volatile unsigned int *)(PER_BASE + 0x178))

void initLED (int iGPIOPin);
void setLedLight(int iGPIOPin, int iMode);
#define	KillLedProc(a) eval( "killall", "setled" )
#define sys_reboot() kill(1, SIGQUIT)

void get_prom_env(char *key, char *ret_val, int max_len);

#define FW_UPDATE_URL_125G ""
#define FW_UPDATE_URL_54G ""


#endif /* _shutils_h_ */
