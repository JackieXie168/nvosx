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
 */

#ifndef _shutils_h_
#define _shutils_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#if __linux__
#include <sys/types.h>
#include <sys/stat.h>
#endif

//#define ___DEBUG___

#define swap(a,b) {		\
		(a) += (b); 			\
		(b) = (a) - (b);		\
		(a) -= (b);				\
}

#if defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX) || defined(darwin)
#define GETDELIM_CHUNKSIZE (64)
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#define IP_QUAD(ip)  (ip)>>24,((ip)&0x00ff0000)>>16,((ip)&0x0000ff00)>>8,((ip)&0x000000ff)
/*
printf("My IP Address (0x%x) is %d.%d.%d.%d\n", my_ip, IP_QUAD(my_ip));
*/
static int  rgiState[57]; // leave this alone

char *return_null(char *name);
char *return_empty(char *name);
char* itos (int);
int isLetters(const char *text);
time_t _time(time_t *);
time_t get_time(time_t *);
int number_gen(void);
int number_range(int, int);
void init_gen( );
void random_string(char *string, const unsigned int length);
void Shell_Sort( int *, int);
int lowercase (char *sPtr);
int uppercase (char *sPtr);
int xtrim(char *);
char *ltrim(char *const s);
char *rtrim(char *const s);
char *trim(char *const s);
char *GetStrBetweenStr(char *, char *, char *);
char *StrDup(const char *s);
void StrFree(void *);
char *StrCat(char *, const char *);
char* __concat(int count, ...);
int matchStrPosAt(const char *, char *, int);
char* replacestr(const char* s, char* in, char* out, int delimiters);
char* replaceall(const char * source, char * in, char * out, int delimiters, int max);
unsigned char* ConvertStringIntoByte(char *pszStr, unsigned char* pbyNum);
int xasprintf(char **bufp, const char *format, ...);
char *readin(FILE *in);
int copyFile(const char *Fin, const char *Fout);
int getFileSize(char* filename, long *filesize);
unsigned long iptoul(char *ip);
char * ultoip(unsigned long ul);
char *strcutail (char *str, const char *n, int pos);
char *strmhead (char *str, const char *n, int pos);
char *index_str (char *str, const char *n, int index);
char *idxOfElement(char dst[], const char *src, const char *delimiter, int idx);
char *insert_str(char *str, char *value, char *delm, int index);
char *delete_str(char *name, char *delm, int index);
char *delete_val(char *name, char *value, char *delm);
char *modify_str(char *str, char *value, char *delm, int index);
int val_exist(char *name, char *value, char *delm);
char *str2digits(char *sval, char *delimiter, int length);
int str2id(char *sval, char *delimiter);
char *get_one_line(char *s, int n, FILE *f);
int mac_validator(const char* value);

#define NUMARGS(...)  (sizeof((char *[]){__VA_ARGS__})/sizeof(char *))
#define concat(...)  (__concat(NUMARGS(__VA_ARGS__), __VA_ARGS__))

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

/* 
 * Waits for a file descriptor to become available for reading or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
extern int waitfor(int fd, int timeout);
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

extern int task_is_running(char *strTask);
extern int get_task_pid(char *strTask);

#ifndef isblank
/* Check for a blank character; that is, a space or a tab */
#define isblank(c) ((c) == ' ' || (c) == '\t')
#endif

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

extern long GetSysUpTime();

struct variable {
        char *name;
        char *validatename;
        char *longname;
        int (*validate)(char *value, struct variable *v);
        char **argv;
        int nullok;
        int event;
};

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _shutils_h_ */
