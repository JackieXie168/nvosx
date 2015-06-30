/*
 * Copyright(c)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the vendors nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NVRAM variable manipulation
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: invram.h,v 1.3.2.2 2008/06/11 08:36:04 jackie Exp $
 */

#ifndef _invram_h_
#define _invram_h_

#ifndef _LANGUAGE_ASSEMBLY
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <semaphore.h>
//#include <typedefs.h>

/* the restore result */
#define RSTR_SUCCESS	1
#define RSTR_ERRFILE		2
#define RSTR_ERRCSUM	3

/* the backup result */
#define BU_SUCCESS	1
#define BU_FAILURE	2

/* commit */
#define CMIT_SUCCESS		1
#define CMIT_FAILURE		2

/* resetting to default */
#define DFLT_SUCCESS		1
#define DFLT_FAILURE		2

#define UDP_PORT	2313
#define UDP_IPADDR	0x7F000001	/* 127.0.0.1 */
#define UDP_TIMEOUT	10
#define UDP_CSUM_NOXMIT	1

typedef unsigned char  uint8;
typedef unsigned int   uint32;

typedef enum { 
	ACT_IDLE,
	ACT_TFTP_UPGRADE,
	ACT_WEB_UPGRADE,
	ACT_WEBS_UPGRADE,
	ACT_SW_RESTORE,
	ACT_HW_RESTORE
} ACTIONS;

struct nvram_header {
	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;			/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	uint32 config_refresh;		/* 0:15 config, 16:31 refresh */
	uint32 config_ncdl;			/* ncdl values for memc */
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

enum {
	CMD_SET = 1,
	CMD_GET,
	CMD_GETALL,
	CMD_UNSET,
	CMD_COMMIT,
	CMD_BACKUP,
	CMD_RESTORE,
	CMD_DEFAULT
};

#define VTABSIZE 127

struct nvram_struct 
{
	char	*name;
	char	*value;

	struct nvram_struct	*next;
};

#if __linux__
//#define TARGET_DEVICE					DV_E5322S_R
#define _PATH_CONFIG						"/var/OpenAgentV2/conf"
#define CONF_PATH							_PATH_CONFIG "/"
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX) || defined(darwin)
#define _PATH_CONFIG						"/var/conf"
#define CONF_PATH							_PATH_CONFIG "/"
//#define SO_NO_CHECK     				0xb
#define SO_NO_CHECK     				0x100a
//#define SO_NO_CHECK     					11
#elif defined(__CYGWIN__) || defined(_MSC_VER)
#define _PATH_CONFIG						"c:\temp"
#define CONF_PATH							_PATH_CONFIG "\"
#endif

#ifdef TARGET_DEVICE
#define _PATH_CONFIG						"/tmp"
#define _PATH_CONFIG_MTD			"/dev/mtd/5"
#define TMP_FILE_PATH 					"/flash/nvram.config"
#define DEFAULT_FILE_PATH 		"/etc/nvram/nvram.config"
#define DEFAULT_FILE_PATH_EU	"/etc/nvram_eu/nvram.config"
#define REGION_FILE_PATH 			"/tmp/firmware_region"
//#define TMP_FILE_PATH "/tmp/config/nvram.config"
//##define TMP_FILE_PATH "/var/run/rc.conf"
#else
#define LOG_FILE_PATH 					CONF_PATH "nvram.log"
#define BACKUP_FILE_PATH 			CONF_PATH "nvram.bak"
#define DEFAULT_FILE_PATH 		CONF_PATH "nvram.conf"
#define DEFAULT_FILE_PATH_EU 	CONF_PATH "nvram_eu.conf"
#define REGION_FILE_PATH 			CONF_PATH "firmware_region"
#define ACTION_FILE     					CONF_PATH "action"
#define TMP_FILE_PATH 					DEFAULT_FILE_PATH
#define _PATH_NVRAM_MTD			TMP_FILE_PATH
#endif

#define ERR_NO_MEM -1

/* nvram agent functions */
extern int invram_match(char *name, char *match);
extern int invram_invmatch(char *name, char *match);
extern void invram_set(char *name, char *value);
extern void invram_unset(char *name);
extern void invram_default(void);
extern void invram_commit(void);
extern char *invram_get(char *name);
extern char *invram_getall(void);
extern int invram_backup(char *ofile);
extern int invram_restore(char * ifile);

/* nvram daemon functions */
extern void xnvram_loop();

#define ACTION(cmd)     		buf_to_file(ACTION_FILE, cmd)

#endif /* _LANGUAGE_ASSEMBLY */
#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_SPACE			0x160000
#endif /* _invram_h_ */
