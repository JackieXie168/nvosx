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
 */

#ifndef _nvram_h_
#define _nvram_h_

#ifndef _LANGUAGE_ASSEMBLY
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <semaphore.h>
//#include <typedefs.h>
#include <typedefs.h>
#include <unistd.h>
#include <shutils.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nvram_header {
	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	uint32 config_refresh;	/* 0:15 config, 16:31 refresh */
	uint32 config_ncdl;	/* ncdl values for memc */
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

#define VTABSIZE 127
#define INIT_BUF_SIZE_LARGE 1638400
#define INIT_BUF_SIZE_SMALL 2048

struct varinit {
	int name_offset;		/*offset of name string : ex. wan_proto=*/
	int text_offset;		/*offset of value string : ex. pppoe*/
	int next_offset;		/*offset of next varinit struct*/
	unsigned short len;
	unsigned short validated;
};

#define INTOFF sem_up(sem_id)		/*set semaphore*/
#define INTON sem_down(sem_id)		/*unset semaphore*/
#define INTOFF_REALLOC sem_up(sem_id_realloc)		/*set semaphore*/
#define INTON_REALLOC sem_down(sem_id_realloc)	/*unset semaphore*/

/*share memory identifier (note. must greater than share memory size)*/
#define NVRAMKEY 655350
/*share memory size*/
#define SHARESIZE 65536*30
#define MAGIC_ID "<NVRAM>"

#if __linux__ || defined(__CYGWIN__)
#define _PATH_CONFIG						concat(safe_getenv("HOME"), "/tmp/conf")
#define CONF_PATH							concat(_PATH_CONFIG, "/")
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX) || defined(darwin)
#define _PATH_CONFIG						concat(safe_getenv("HOME"), "/var/conf")
#define CONF_PATH							concat(_PATH_CONFIG, "/")
//#define SO_NO_CHECK     				0xb
#define SO_NO_CHECK     				0x100a
#elif defined(_MSC_VER) || defined(__MINGW32__)
#define _PATH_CONFIG						concat(safe_getenv("HOMEDRIVE"), safe_getenv("HOMEPATH"), "\\temp")
#define CONF_PATH							concat(_PATH_CONFIG, "\\")
#include <inttypes.h>
#define inline
#endif

#ifdef TARGET_DEVICE
#define _PATH_CONFIG						"/tmp"
#define _PATH_CONFIG_MTD					"/dev/mtd/5"
#define TMP_FILE_PATH 						"/flash/nvram.config"
#define DEFAULT_FILE_PATH 					"/etc/nvram/nvram.config"
#define DEFAULT_FILE_PATH_EU				"/etc/nvram_eu/nvram.config"
#define REGION_FILE_PATH 					"/tmp/firmware_region"
//#define TMP_FILE_PATH "/tmp/config/nvram.config"
//##define TMP_FILE_PATH "/var/run/rc.conf"
#else
#define LOG_FILE_PATH 						concat(CONF_PATH, "nvram.log")
#define BACKUP_FILE_PATH 					concat(CONF_PATH, "nvram.bak")
#define DEFAULT_FILE_PATH 					concat(CONF_PATH, "nvram.default")
#define DEFAULT_FILE_PATH_EU 				concat(CONF_PATH, "nvram_eu.conf")
#define REGION_FILE_PATH 					concat(CONF_PATH, "firmware_region")
#define ACTION_FILE     					concat(CONF_PATH, "action")
#define TMP_FILE_PATH 						concat(CONF_PATH, "nvram.conf")
#define _PATH_CONFIG_MTD					TMP_FILE_PATH
#endif

#define COMMIT_PROG						concat(safe_getenv("HOME"), "/bin/commit")

#define ERR_NO_MEM -1

//void *xmalloc(size_t sz);
void dump_mem(void*,int);
int nvram_init();
void attach_share_memory(void);
void nvram_clean(void);
void re_alloc(void);
void detach_shm(void);

/* NOTE: when successfully `fork()', calling this to re-initialized the nvram. */
extern void init_nvram(void);
extern int nvram_invmatch(char *name, char *match);
extern int nvram_match(char *name, char *match);
extern int nvram_commit(void);
extern char *nvram_free();
extern char *nvram_get(const char *name);
extern int nvram_set(const char *name, const char *val);
extern int nvram_unset(const char *s);

/*
 * Initialize NVRAM access. May be unnecessary or undefined on certain
 * platforms.
 */
#if 0
extern int BCMINIT(nvram_init)(void *sbh);
#endif

/*
 * Disable NVRAM access. May be unnecessary or undefined on certain
 * platforms.
 */
extern void BCMINIT(nvram_exit)(void);

/*
 * Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
extern char * BCMINIT(nvram_get)(const char *name);

/* 
 * Get the value of an NVRAM variable.
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
#define nvram_safe_get(name) (BCMINIT(nvram_get)(name) ? : "")

#define nvram_safe_get_x(sid, name) (nvram_get_x(sid, name) ? : "")
#define nvram_safe_get_f(file, field) (nvram_get_f(file, field) ? : "")

/* 
 * Get the value of an NVRAM variable.
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
//#define nvram_safe_get(name) (nvram_get(name) ? : "")

#define nvram_safe_unset(name) ({ \
	if(nvram_get(name)) \
		nvram_unset(name); \
})

#define nvram_safe_set(name, value) ({ \
	if(!nvram_get(name) || strcmp(nvram_get(name), value)) \
		nvram_set(name, value); \
})

/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */
#if 0
static INLINE int
nvram_match(char *name, char *match) {
	const char *value = BCMINIT(nvram_get)(name);
	return (value && !strcmp(value, match));
}
#endif
/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */
#if 0
static INLINE int
nvram_invmatch(char *name, char *invmatch) {
	const char *value = BCMINIT(nvram_get)(name);
	return (value && strcmp(value, invmatch));
}
#endif
/*
 * Set the value of an NVRAM variable. The name and value strings are
 * copied into private storage. Pointers to previously set values
 * may become invalid. The new value may be immediately
 * retrieved but will not be permanently stored until a commit.
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 */
extern int BCMINIT(nvram_set)(const char *name, const char *value);

/*
 * Unset an NVRAM variable. Pointers to previously set values
 * remain valid until a set.
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
extern int BCMINIT(nvram_unset)(const char *name);

/*
 * Commit NVRAM variables to permanent storage. All pointers to values
 * may be invalid after a commit.
 * NVRAM values are undefined after a commit.
 * @return	0 on success and errno on failure
 */
extern int BCMINIT(nvram_commit)(void);

/*
 * Get all NVRAM variables (format name=value\0 ... \0\0).
 * @param	buf	buffer to store variables
 * @param	count	size of buffer in bytes
 * @return	0 on success and errno on failure
 */
extern int BCMINIT(nvram_getall)(char *buf, int count);

#endif /* _LANGUAGE_ASSEMBLY */

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	20
#define NVRAM_SPACE		0x1600000

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _nvram.h_ */
