/*
 * shutils.c
 *
 * Shell-like utility functions
 *
 * Created by Jackie Xie on 2011-07-27.
 * Copyright 2011 Jackie Xie. All rights reserved.
 *
 */

#include <stdarg.h>
#include <errno.h>
#if __linux__
#include <error.h>
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
#include <sys/sysctl.h>
#endif
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <nvram.h>
#include <shutils.h>

#if __linux__
#include <signal.h>
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
#include <sys/signal.h>
#define _NSIG NSIG 
#endif

#ifndef cprintf
#define cprintf	printf
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef CHK_MEMLEAK
void * debug_malloc(size_t size, const char *file, int line, const char *func)
{
	void *p;

	p = malloc(size);
	printf("%s:%d:%s:malloc(%ld): p=0x%lx\n",  file, line, func, size, (unsigned long)p);
	return p;
}

#define malloc(s) debug_malloc(s, __FILE__, __LINE__, __func__)
#define free(p)  do {						   \
	printf("%s:%d:%s:free(0x%lx)\n", __FILE__, __LINE__,	    \
	    __func__, (unsigned long)p);				\
	free(p);							\
} while (0)
#endif

/* get function */
char *return_empty(char *name)
{
	return "";
}

/* ======================(function header)========================
Function Name:char* itos (){}
Description:to convert integer to string
Arguments:unsigned n
Return:a string
written by jackiexie
Date:  2007/07/10
================================================================*/
char tmpbuf[17];
char* itos (int n)
{
  int i=0,j;
  //char* s;
  //char* u;
  char s[17];
  //s= (char*) malloc(17);
  //u= (char*) malloc(17);
  
  do{
    s[i++]=(char)( n%10+48 );
    n-=n%10;
  }
  while((n/=10)>0);
  for (j=0;j<i;j++)
  tmpbuf[i-1-j]=s[j];

  tmpbuf[j]='\0';
  return tmpbuf;
}

int isLetters(const char *text)
{
	if (!text) return 0;
	while (*text) {
		if (!isalpha(*text)) return 0;
		++text;
	}
	return 1;
}

/* ======================(function header)========================
Function Name:time_t time(time_t *timer){}
Description:get current time value 
Arguments:time_t *timer
Return:time_t
written by jackie xie
Date:   2007/07/10
================================================================*/
/****************************************************************
 *
 * time					2007/07/10  jackie xie
 *
 ****************************************************************/
time_t _time(time_t *timer)
{
   struct timeval xCurrTime;
   xCurrTime.tv_usec = *timer;
   gettimeofday(&xCurrTime, NULL);
   return((xCurrTime.tv_sec << 1000) + (xCurrTime.tv_usec >> 1000));
}

time_t get_time(time_t *t)
{ 
  time_t tRet;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  tRet = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
  
  if (t) {
    *t = tRet;
  }
  return tRet;
}

/* ======================(function header)========================
Function Name:int number_gen(){}
Description:Get a random number
Arguments:none
Return:integer
written by jackie xie
Date:   2007/07/10
================================================================*/
/****************************************************************
 *
 * number_gen				2007/07/10   jackie xie
 *
 ****************************************************************/
int number_gen( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= ( piState[iState1] + piState[iState2] )
    			& ( ( 1 << 30 ) - 1 );
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
    	iState1 = 0;
    if ( ++iState2 == 55 )
    	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}

/* ======================(function header)========================
Function Name:int number_range(){}
Description:Get a generated random number
Arguments:two integer numbers for a range ( int from, int to )
Return:integer
written by jackie xie
Date:   2007/07/10
================================================================*/
/****************************************************************
 *
 * Generate a random number		2007/06/28 jackie xie
 *
 ****************************************************************/
int number_range( int from, int to )
{
    int power;
    int number;
    if ( ( to = to - from + 1 ) <= 1 )
    	return from;
    for ( power = 2; power < to; power <<= 1 )
    	;
    while ( ( number = number_gen( ) & ( power - 1 ) ) >= to )
    	;
    return from + number;
}

/* ======================(function header)========================
Function Name:void init_gen( ){}
Description:seed the number generator
Arguments:none
Return:none
written by jackie xie
Date:   2007/07/10
================================================================*/
/****************************************************************
*
* This function is follow the Mitchell-Moore algorithm from 								
* Knuth Volume II.			2007/07/10   jackie xie
*
****************************************************************/
void init_gen( )
{
    int *piState;
    int iState;
    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;
    //cout<<"current time : "<<time( NULL )<<endl;
    //printf("current time :  %ld\n", (long)get_time( NULL ));
    piState[0]	= ( (int) time( NULL ) ) & ( ( 1 << 30 ) - 1 );
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        	piState[iState] = ( piState[iState-1] + piState[iState-2] )
        			& ( ( 1 << 30 ) - 1 );
    }
    return;
}

// To generate alphanumeric Random String
void random_string(char *string, const unsigned int length) {
	int i;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	/* Seed number for rand() */
	srand((unsigned int) time(0) + getpid());

	for (i = 0; i < length; ++i) {
		string[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	string[i] = '\0';	
}

/*Shell Sort*/
void Shell_Sort( int *array , int index )
{
  int i ;
  int tmp ;
  int length;
  int process;

  length = index / 2;

  while ( length != 0 )
  {
    for ( i = length ; i < index ; i++ )
    {
      tmp = array[i];
      process = i - length;

      while ( tmp < array[process] && process >= 0 && i <= index )
      {
        array[ process + length ] = array[process];
        process = process - length;
      }
      array[ process + length ] = tmp;
    }
    length = length / 2;
  }
}

int lowercase (char *sPtr)
{
	while ( *sPtr != '\0' ) {
		*sPtr = tolower((unsigned char) *sPtr);
		++sPtr;
	}
	return 0;
}

int uppercase (char *sPtr)
{
	while ( *sPtr != '\0' ) {
		*sPtr = toupper((unsigned char)*sPtr);
		++sPtr;
	}
	return 0;
}

/* ======================(function header)========================
Function Name:int trim(char *)()
Description:elimination the space of the
            string.
Arguments:char* str
Return:none-space string
written by jackie xie
Date:   2007/07/10
================================================================*/
int xtrim(char *s)
{
   char *esp,*sp=s,c; esp=sp;
   if(s!=NULL && *s)
   {
      do ; while( (*sp++)==32 );
      sp--;
      do {
         c=*sp++;
         *s++=c;
         if( (c!=32) && (c>0) ) esp=s;
         do ; while( (*sp++)==32 );
         sp--;
      }while (c>0);
      *esp=0;
   }
   return 0;
}

/* Remove leading whitespaces */
char *ltrim(char *const s)
{
	size_t len;
	char *cur;

	if(s && *s) {
		len = strlen(s);
		cur = s;

		while(*cur && isspace(*cur))
			++cur, --len;

		if(s != cur)
			memmove(s, cur, len + 1);
	}

	return s;
}

/* Remove trailing whitespaces */
char *rtrim(char *const s)
{
	size_t len;
	char *cur;

	if(s && *s) {
		len = strlen(s);
		cur = s + len - 1;

		while(cur != s && isspace(*cur))
			--cur, --len;

		cur[isspace(*cur) ? 0 : 1] = '\0';
	}

	return s;
}

/* Remove leading and trailing whitespaces */
char *trim(char *const s)
{
	rtrim(s);  // order matters
	ltrim(s);

	return s;
}

/* ======================(function header)========================
Function Name: char *GetStrBetweenStr(char *,
                                char *,char *)
Description: retrieve a string from between left position
                 and right position string.
Arguments:char *s,char *lstr,char *rstr
Return: string
written by jackie xie
Date:   2007/07/10
================================================================*/
char *GetStrBetweenStr(char *s,char *lstr,char *rstr)
{
   char *p=s,*lp=s,*rp;
   do
   {
      /*trim(lstr);
      trim(rstr);*/
      lp=strstr(lp,lstr);
      if(lp)
      {
         rp=lstr;
         do lp++; while(*rp++);
         lp--;
         rp=strstr(lp,rstr);
         if(rp)
         {
            if(lp!=rp)
            {
               do *p++=*lp++; while(lp<rp);
               lp--;
            }
            rp=rstr;
            do lp++; while(*rp++);
            lp--;
         }
      }
   } while(lp);
   *p=0;
   return s;
}

char *StrDup(const char *s)
{
  char *d = NULL;
  if (s) {
    d = (char *)malloc(sizeof(char*) * strlen(s) + 1);
    strcpy(d, s);
  }
  return d;
}

void StrFree(void *ptr)
{
  if (ptr) {
    free(ptr);
  }
}

/* ======================(function header)========================
Function Name: char *StrCat(char *, const char *){}
Description: copy string function. using this function
            before you must initial the source string
Arguments: char *src, const char *dest
Return: string
written by jackie xie
Date   :  2007/07/10
================================================================*/
char *StrCat(char *src, const char *dest)
{
	char *newstr = src;
	int length;

	length = (unsigned) strlen(src) + (unsigned) strlen(dest) + 1;
	src = (char *) realloc(src,length);

	while (*newstr)
	{
		newstr++;
	}

	while ((*(newstr++) = (*(dest++))))
	{
		;
	}
	return src;
}

char* __concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

unsigned char* ConvertStringIntoByte(char *pszStr, unsigned char* pbyNum)
{
	//const char cSep0 = NULL ; //Bytes separator in string like 00aabbccddee
	const char cSep1 = '-'; 	//Bytes separator in string like 00-aa-bb-cc-dd-ee
	const char cSep2 = ' '; 	//Bytes separator in string like 00 aa bb cc dd ee
	const char cSep3 = ':'; 	//Bytes separator in string like 00:aa:bb:cc:dd:ee
	const char cSep4 = '.'; 	//Bytes separator in string like 00.aa.bb.cc.dd.ee
	const char cSep5 = ','; 	//Bytes separator in string like 00,aa,bb,cc,dd,ee
	const char cSep6 = '/'; 	//Bytes separator in string like 00/aa/bb/cc/dd/ee	
	int iConunter;
	bool exist_delimiter = false;
	int strLength;
		
	strLength = strlen(pszStr);
		
	if(strLength > 13)
		exist_delimiter = true;
	
	for (iConunter = 0; iConunter < 6; ++iConunter)
	{
		unsigned int iNumber = 0;
		char ch;
			
		//Convert letter into lower case.
		ch = tolower (*pszStr++);

		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
		{
			//printf("delimiter is %c\n", ch);
			return NULL;
		}

		//Convert into number. 
		//         a. If chareater is digit then ch - '0'
		//		   b. else (ch - 'a' + 10) it is done because addition of 10 takes correct value.
		iNumber = isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);
		ch = tolower (*pszStr);

		if ((iConunter < 5 && (ch != cSep1 && ch != cSep2 && ch != cSep3 && ch != cSep4 && ch != cSep5 && ch != cSep6)) || (iConunter == 5 && ch != '\0' && !isspace(ch)))
		{
			++pszStr;

			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
			{
				return NULL;
			}

			iNumber <<= 4;
			iNumber += isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);
			ch = *pszStr;
		
			if(exist_delimiter)
				if (iConunter < 5 && (ch != cSep1 && ch != cSep2 && ch != cSep3 && ch != cSep4 && ch != cSep5 && ch != cSep6))
				{
					return NULL;
				}
		}
		/* Store result.  */
		pbyNum[iConunter] = (unsigned char) iNumber;
		/* Skip cSep.  */
		if(exist_delimiter)
			++pszStr;			
	}
	return pbyNum;
}

int xasprintf(char **bufp, const char *format, ...)
{
  va_list ap, ap1;
  int rv;
  int bytes;
  char *p;

  va_start(ap, format);
  va_copy(ap1, ap);

  bytes = vsnprintf(NULL, 0, format, ap1) + 1;
  va_end(ap1);

  *bufp = p = malloc(bytes);
  if ( !p )
    return -1;

  rv = vsnprintf(p, bytes, format, ap);
  va_end(ap);

  return rv;
}

int mkdir_r(const char *path)
{
	mode_t mode = (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);
	if (path == NULL) {
		return -1;
	}
	char *tmp = StrDup(path);
	char *pos = tmp;

	/* skip over slashes */
	if (strncmp(tmp, "/", 1) == 0) {
		pos += 1;
	} else if (strncmp(tmp, "./", 2) == 0) {
		pos += 2;
	}
	/* create directories recursively */
	for ( ; *pos != '\0'; ++ pos) {
		if (*pos == '/') {
			*pos = '\0';
			mkdir(tmp, mode);
			*pos = '/';
		}
	}
	/* stop to create directories recursively if it's at last slash */
	if (*(pos - 1) != '/') {
		mkdir(tmp, mode);
	}
	StrFree(tmp);
	return 0;
}

#if defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX) || defined(darwin)
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
	ssize_t linelen;
	char *newlineptr;
	char c;

	if (!lineptr || !n || !stream)
	{
		errno = EINVAL;
		return -1;
	}

	if (!*lineptr)
	{
		*n = GETDELIM_CHUNKSIZE;
		*lineptr = malloc(*n);
	}
	if (!*lineptr)
		return -1;

	linelen = 0;
	while (1)
	{
		/*
		 * Ensure adequate length. This has to be matched to the
		 * subsequent for loop.
		 */
		if (linelen + 2 > *n)
		{
			*n += GETDELIM_CHUNKSIZE;
			newlineptr = realloc(*lineptr, *n);
			if (!newlineptr)
			{
				*n -= GETDELIM_CHUNKSIZE;
				return -1;
			}
			*lineptr = newlineptr;
		}

		/*
		 * Ensure we have space for two new chars: the one we read and
		 * the terminating NUL.
		 */
		errno = 0;
		for (; linelen < *n - 1; linelen ++)
		{
			c = fgetc(stream);
			if (c == EOF)
			{
				if (errno)
				{
					/*
					 * Should we just set EINVAL for everything? We're
					 * supposed to set EINVAL for a bad stream
					 * descriptor... and I assume/hope that EBADF fits
					 * the description... --binki
					 */
					if (errno == EBADF)
						errno = EINVAL;
					return -1;
				}
				if (feof(stream))
					if (linelen)
					{
						/* we hit EOF _instead_ of a delimiter */
						(*lineptr)[linelen] = '\0';
						return linelen;
					}
			else
				/* since we've read no data, time to tell the caller it's EOF */
				return -1;
			}
			(*lineptr)[linelen] = c;
			if (c == delim)
			{
				linelen ++;
				/* our for loop ensures that we have this second char to put the '\0' into */
				(*lineptr)[linelen] = '\0';
				return linelen;
			}
		}
	}
}

#if 1
ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
	return getdelim(lineptr, n, '\n', stream);
}
#else		/* An other implemenation of getline */
ssize_t getline(char **linep, size_t *np, FILE *stream)
{
	char *p = NULL;
	size_t i = 0;
	int ch = 0;

	if (!linep || !np) {
		errno = EINVAL;
		return -1;
	}

	if (!(*linep) || !(*np)) {
		*np = 120;
		*linep = (char *)malloc(*np);
		if (!(*linep)) {
			return -1;
		}
	}

	flockfile(stream);

	p = *linep;
	for (ch = 0; (ch = getc_unlocked(stream)) != EOF;) {
		if (i > *np) {
			/* Grow *linep. */
			size_t m = *np * 2;
			char *s = (char *)realloc(*linep, m);

			if (!s) {
				int error = errno;
				funlockfile(stream);
				errno = error;
				return -1;
			}

			*linep = s;
			*np = m;
		}

		p[i] = ch;
		if ('\n' == ch) break;
		i += 1;
	}
	funlockfile(stream);

	/* Null-terminate the string. */
	if (i > *np) {
		/* Grow *linep. */
			size_t m = *np * 2;
			char *s = (char *)realloc(*linep, m);

			if (!s) {
				return -1;
			}

			*linep = s;
			*np = m;
	}

	p[i + 1] = '\0';
	return ((i > 0)? i : -1);
}
#endif
#endif

char *readin(FILE *in)
{
    char tmp[80];
    char *result="";
    while( fgets( tmp, 80, in)!=NULL ) {
        xasprintf(&result, "%s%s", result, tmp);
    }
    return result;
}

int copyFile(const char *Fin, const char *Fout)
{
	int inF, ouF;
	char line[2048];
	int bytes;

	if((inF = open(Fin, O_RDONLY)) == -1) {
		printf("Failed to open file %s\n", Fin);
		return -1;
	}

	unlink(Fout);													
	if((ouF = open(Fout, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
		printf("Failed to open file %s\n", Fout);
		return -1;
	}
	while((bytes = read(inF, line, sizeof(line))) > 0)
		write(ouF, line, bytes);	
	close(inF);
	close(ouF);
	return 0;
}

int getFileSize(char* filename, long *filesize)
{
	FILE *pFile;
	long size;
	
	pFile = fopen (filename,"rb");
	if (pFile==NULL){
		perror ("Error opening file");
		return -1;
	}
	else
	{
		fseek (pFile, 0, SEEK_END);
		size=ftell (pFile);
		fclose (pFile);
	}
	//printf ("Size of %s : %ld bytes.\n", filename, size);
	*filesize = size;
	
	return 0;
}

/* ======================(function header)========================
Function Name: int matchStrPosAt (const char *, const char *)
Description: find the sub-string from the source string.
Arguments: const char *, const char *
Return: Returns "the array element number"(or "position") where 
      "sub-string" matches before in "original string", or return 
      "-1" if substr is not present in "original string".
written by jackie xie
Date   :  2007/07/10
================================================================*/
/* 
 *	delimiters = ±n：return position before n-th matched substr.
 *	delimiters =   0 ：do nothing.
 *	delimiters =  -1 ：returns the array element number where "substr" occurs in "str".
 */
int matchStrPosAt (const char * substr, char * str, int delimiters)
{
	int i = -1, k, str_index, total = 0;
	int substr_len = strlen(substr), str_len = strlen(str);
	int matchpos = -1;
	int delimiter[str_len], ind_delim = 0;
	//int n = abs(delimiters);

	for(k=0; k<=str_len ; k++)
		delimiter[k] = -1;

	if (str == NULL || substr == NULL) return -1;

	/* substr has to be smaller than str */
	if (substr_len > str_len) return -1;

	/* look through str for substr, stopping at end of string or when the length
	 * of substr exceeds the remaining number of characters to be searched */ 
	while (str[++i] != '\0' && (str_len - i + 1) > substr_len) 
	{
		for (str_index = 0; str_index < substr_len; ++str_index)
		{
			/* definitely not at location i */
			if (str[i + str_index] != substr[str_index])
				break;
			/* if last letter matches, then we know the whole thing matched */
			else if (str_index == substr_len - 1)
			{
				if(delimiters >= 1)
				{
					i += substr_len -1;
					delimiter[++ind_delim] = i;
					//printf("delimiter[%d] = %d\n", ind_delim, delimiter[ind_delim]);
				}
				else if(delimiters == 0)
					return i;
				else if(delimiters == -1)
					total++;
			}
		}
	}

	if(delimiters == 0)
		return -1;						/* substr not present in str */
	else if(delimiters == -1)
		return total;					/* the total numbers of substr */
	else if(delimiters < ind_delim)
		matchpos = delimiter[delimiters];
	else /*if(delimiters == ind_delim)*/
		matchpos = delimiter[ind_delim];

	return matchpos;
}

/* replaces the first occurence of "in" within the string "source" with
 * the string "new" and returns empty string if no replacement was made
 * limitation ---> if the in is a subset of out and strlen(out) is greater than 
 *						  strlen(in) then we'll got the unexpected result. 
 */
char *replacestr(const char *s, char *in, char *out, int delimiters)
{
	int replace_index;
	int source_len, in_len, out_len, in_idx;
 	int total_len;
	int i = delimiters, delta = 0;
	char *source;
	char *_new;

	if(!strcmp(in ,out))
		return (char *)s;

	in_len = strlen(in);
	out_len = strlen(out);
	delta = abs(in_len - out_len);
	if(i == -2) delimiters = 1;
	in_idx = matchStrPosAt(in, (char *)s, delimiters);
	if(in_len < out_len){
		if(i != -2)
			replace_index = in_idx;
		else
 			replace_index = in_idx - in_len + 1;
#if defined(___DEBUG___)
 		printf("cond1\n");
#endif
 	}
	else if (((in_len > 1) && (out_len > 1) && (delta == 0)) || (in_len > out_len)){
		if(i != -2)
			replace_index = in_idx;
		else
 			replace_index = in_idx - out_len - (delta - 1);
#if defined(___DEBUG___)
 		printf("cond2\n");
#endif
 	}
 	else{
 		replace_index = in_idx;
#if defined(___DEBUG___)
 		printf("cond3\n");
#endif
	}

#if defined(___DEBUG___)
	printf("in_len = %d\tout_len = %d\tdelta = %d\tposition = %d\n", in_len, out_len, delta, replace_index);
#endif

	if (replace_index < 0)
		return "";
	else
	{
		source = StrDup (s);
		source_len = strlen(source);
		total_len = sizeof(char*) * (source_len - in_len + out_len) + 1;
		//total_len = source_len - in_len + out_len + 1;

		/*if(total_len == source_len)
			total_len = total_len + 1;*/

		//if ((*_new = (char *)calloc(total_len,sizeof(char))) == NULL)

		if ((_new = (char *)malloc(sizeof(char*) * total_len + 1)) == NULL)
			//my_perror("malloc", FATAL);
			return NULL;

		(_new)[total_len - 1] = '\0';

		/* copy part before match */
		for (i = 0; i < replace_index; ++i)
			(_new)[i] = source[i];

		/* insert new string */
		for (i = replace_index; i < replace_index + out_len; ++i)
			(_new)[i] = out[i - replace_index];

 		/* copy part after match */
		for (i = replace_index + out_len; i < total_len - 1; ++i)
		(_new)[i] = source[(i - out_len + in_len)];
	}
	StrFree(source);
	source = NULL;
	return _new;
}

/* replaces at most 'max' occurences of "in" within the string "source" with
** the string "out".  A replacement may contain another occurence of 
** "in", in which case that will be replaced before an following occurence
** (i.e., it works left to right) */ 
char *replaceall(const char *source, char *in, char *out, int delimiters, int max)
{
	int i;
	char *_new;

	if (max < 1) return (char *)source;

	_new = replacestr(source, in, out, delimiters);

	for (i = 1; i < max; ++i)
	{
		_new = replacestr(StrDup(_new), in, out, delimiters);
		if (!strcmp(_new,"") || _new == NULL)
			break;
	}
	return _new;
}

unsigned long iptoul(char *ip)
{
	unsigned long ul = 0, t;
	char *p;
	
	do{
		t = strtoul(ip, &p, 0);
		ul = ul * 256 + t;
		ip = p + 1;
	} while (*p == '.');
	
	return ul;
}
	
char *ultoip(unsigned long ul)
{
	static char ip[16];
	char t[16];
	char *tp;
	unsigned long tl;
	ip[0] = '\0';
	
	do{
		tp = t;
		tl = ul % 256;
		ul /= 256;
		if (ul) *t = '.', ++tp;
		sprintf(tp, "%lu", tl);
		strcat(t, ip);
		strcpy(ip, t);
	} while (ul);
	
	return ip;
}

/* ======================(function header)========================
Function Name: int strcutail (char *str, const char *n, int pos)
Description: To remove the sub-string which is starting at 
			 n-th delimiter(or include it) to the end of input string.
Arguments: char *, const char *
Return: Returns the string which was cut the tailed sub-string off.
written by Jackie Xie
Date   :  2011/07/15
================================================================*/
char *strcutail (char *str, const char *n, int pos)
{
	int i, _newStrLen = (pos != 0)?matchStrPosAt(n, str, abs(pos)): -1;
	char *_new;
	
	if(_newStrLen >= 0){
		if (pos >= 0)
			++_newStrLen;			
		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;

		(_new)[_newStrLen] = '\0';		

		/* copy sub-string from the head of the string to the specified delimiter */
		for (i = 0; i < _newStrLen; ++i)
			(_new)[i] = str[i];

		strcpy(str, _new);
#ifndef TARGET_DEVICE
		StrFree(_new);	
#endif
		_new = NULL;
	}
	return str;
}


/* ======================(function header)========================
Function Name: int strmhead (char *str, const char *n, int pos)
Description: To cut the front of sub-string which is starting at the
			  matched delimiter and ending of the input string.
Arguments: char *, const char *
Return: The string which was cut the front of input string till the 
				specified delimiter.
written by Jackie Xie
Date   :  2011/08/12
================================================================*/
char *strmhead (char *str, const char *n, int pos)
{
	int i, _matchedStrLen = (pos != 0)?matchStrPosAt(n, str, abs(pos)): -1;
	int str_len = strlen(str);
	int _newStrLen = 0;
	
	char *_new;
	
	if(_matchedStrLen >= 0){
		if (pos >= 0)
			_matchedStrLen -= strlen(n);
			
		_newStrLen = str_len - _matchedStrLen;

		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;

		(_new)[_newStrLen+1] = '\0';		

		/* copy sub-string from specified delimiter */
		for (i = 0; i < _newStrLen; ++i)
			(_new)[i] = str[_matchedStrLen +1 + i];
		strcpy(str, _new);
		StrFree(_new); _new = NULL;
	}
	
	return str;
}

char *index_str (char *str, const char *n, int index)
{
	int i, str_len = strlen(str), delm_len = strlen(n);
	int _newStrLen = 0;
	int idx1, idx2;
	char *_new;
	
	index = abs(index);
	if(index == 0 || index > matchStrPosAt(n, str, -1) + 1)
		return NULL;
	else if(index == 1){
		idx1 = 0;
		idx2 = matchStrPosAt(n, str, index) - delm_len - 1;
	}
	else if(index > 1 && index <= matchStrPosAt(n, str, -1)){
		idx1 = matchStrPosAt(n, str, index - 1) + 1;
		idx2 = matchStrPosAt(n, str, index) - delm_len - 1;
	}
	else if(index == matchStrPosAt(n, str, -1) + 1){
		idx1 = matchStrPosAt(n, str, index - 1) + 1;
		idx2 = 	str_len - 1;
	}
	_newStrLen = idx2 - idx1 + 1;
	if(_newStrLen < 0)
		_newStrLen = str_len - 1;
	if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
		return NULL;
	(_new)[_newStrLen + 1] = '\0';		
	for (i = 0; i <= _newStrLen; ++i)
		(_new)[i] = str[idx1+i];

	strcpy(str, _new);
	//StrFree(_new); _new = NULL;
	return str;
}

char *insert_str(char *str, char *value, char *delm, int index)
{
	int i, idx, tokens;
	int str_len = strlen(str), val_len = strlen(value), delm_len = strlen(delm);
	int _newStrLen = str_len + val_len + delm_len + 1;
	char *_new;

	if(index == 0) return str;
	if(str_len == 0){
		_newStrLen = val_len;
		delm_len = 0;
	}

	index = abs(index);
	tokens = matchStrPosAt(delm, str, -1) + 1;
	idx = matchStrPosAt(delm, str, index-1) + 1;
	if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
		return NULL;
	(_new)[_newStrLen] = '\0';		

	for(i = 0; i <= _newStrLen; i++){
		if(index == 1){
			if(i <= val_len - 1)
				(_new)[i] = value[i];
			else if(i > val_len - 1 && i <= val_len + delm_len - 1 && str_len != 0)
				(_new)[i] = delm[i - val_len];
			else if(i > val_len + delm_len - 1 && str_len != 0)
				(_new)[i] = str[i - val_len - delm_len];
		}
		else if(index > 1 && index <= tokens){
			if(i <= idx -1)
				(_new)[i] = str[i];
			else if(i >= idx && i <= idx + val_len -1)
				(_new)[i] = value[i - idx];
			else if(i > idx + val_len -1 && i < idx + val_len + delm_len)
				(_new)[i] = delm[i - idx - val_len];
			else if(i >= idx + val_len + delm_len)
				(_new)[i] = str[i - val_len - delm_len];
		}
		else if(index > tokens){
			if(i <= str_len - 1)
				(_new)[i] = str[i];
			else if(i > str_len - 1 && i <= str_len + delm_len - 1)
				(_new)[i] = delm[i - str_len];
			else if(i > str_len + delm_len - 1)
				(_new)[i] = value[i - str_len - delm_len];
		}
	}
	str = StrDup(_new);
	StrFree(_new); _new = NULL;
	return str;	
}

/*
────────────────────────────────────────────────
			index		1	2	3		4			5			6				7				8					9					10
────────────────────────────────────────────────
delete					0 11 222 3333 44444 555555 6666666 77777777 888888888 9999999999
────────────────────────────────────────────────
index '0'	: 			(null)
index '1' 	: 			11 222 3333 44444 555555 6666666 77777777 888888888 9999999999
index '2' 	: 			0 222 3333 44444 555555 6666666 77777777 888888888 9999999999
index '3' 	: 			0 11 3333 44444 555555 6666666 77777777 888888888 9999999999
index '4' 	: 			0 11 222 44444 555555 6666666 77777777 888888888 9999999999
index '5' 	: 			0 11 222 3333 555555 6666666 77777777 888888888 9999999999
index '6' 	: 			0 11 222 3333 44444 6666666 77777777 888888888 9999999999
index '7' 	: 			0 11 222 3333 44444 555555 77777777 888888888 9999999999
index '8' 	: 			0 11 222 3333 44444 555555 6666666 888888888 9999999999
index '9' 	: 			0 11 222 3333 44444 555555 6666666 77777777 9999999999
index '10'	: 			0 11 222 3333 44444 555555 6666666 77777777 888888888
index '11'	: 			(null)
────────────────────────────────────────────────
*/
char *delete_str(char *name, char *delm, int index)
{
	int i, idx, idx1, idx2, newpos = 0;
	int name_len = strlen(name), delm_len = strlen(delm);
	int _newStrLen;
	char *_new;

	idx = matchStrPosAt(delm, name, -1);
	//printf("idx = %d\n", idx);
	index = abs(index);
	if(idx <= 0)
		return "";
	else if(index == 0 || idx +1 < index)
		return name;
	else if(index == 1){
		//return strmhead(name, delm, 1);
		idx1 = matchStrPosAt(delm, name, index) + 1;
		_newStrLen = name_len - idx1;
		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;
		(_new)[_newStrLen + 1] = '\0';
		for(i = 0; i <= _newStrLen; i++)
			(_new)[i] = name[i+idx1];
	}
	else if(index >= idx + 1){
		//return strcutail(name, delm, matchStrPosAt(delm, name, -1));
		idx2 = matchStrPosAt(delm, name, index - 1) - delm_len;
		//printf("idx2 = %d\n", idx2);
		_newStrLen = idx2;
		//_newStrLen = idx2 - delm_len + 1;
		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;
		(_new)[_newStrLen + 1] = '\0';
		for(i = 0; i <= _newStrLen; i++)
			(_new)[i] = name[i];
	}
	else if(index > 1 && index < idx+1){
		idx1 = matchStrPosAt(delm, name, index - 1) + 1;
		idx2 = matchStrPosAt(delm, name, index);
		_newStrLen = name_len - delm_len - idx2 + idx1 + idx + 2;
		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;
		(_new)[_newStrLen + 1] = '\0';		
		for(i = 0; newpos <= _newStrLen; i++){
			if(i >= idx1 && idx2 >= i)
				continue;
			else
				(_new)[newpos++] = name[i];
		}
	}
	strcpy(name, _new);
	StrFree(_new); _new = NULL;
	return name;	
}

char *delete_val(char *name, char *value, char *delm)
{
	int i, c;
	char *tmp = NULL, *del = NULL;
	
	c = matchStrPosAt(delm, name, -1) + 1;
	if(c == 0) return "";
	for (i = 1; i <= c ; i++) {
		tmp = StrDup(name);
		del = StrDup(index_str(tmp, delm, i));
		if(!strcmp(del, value)){
			StrFree(tmp); tmp = NULL;
			printf("value %s is matched in %s\n", value, name);
			tmp = StrDup(delete_str(name, delm, i));
			strcpy(name, tmp);
			printf("delete '%s' which is spcified at position %d\n", value, i);
			break;
		}
	}
	StrFree(tmp); StrFree(del); tmp = del = NULL;
	return name;
}

#if 1
char *modify_str(char *name, char *value, char *delm, int index)
{
	int i, idx, idx1, idx2, newpos = 0;
	int name_len = strlen(name), delm_len = strlen(delm), vlen = strlen(value);
	int _newStrLen;
	char *_new;

	idx = matchStrPosAt(delm, name, -1);
	//printf("delete_str : idx = %d\n", idx);
	index = abs(index);
	if(idx <= 0)
		return value;
	else if(index == 0 || idx +1 < index)
		return name;
	else if(index == 1){
		//return strmhead(name, delm, 1);
		idx1 = matchStrPosAt(delm, name, index) - delm_len;
		//printf("%d\n", idx1);
		_newStrLen = name_len - idx1 + vlen + 1 + delm_len;
		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;
		(_new)[_newStrLen + 1] = '\0';

		newpos = idx1;
		for(i = 0; i <= _newStrLen; i++){
			if(i < vlen)
				(_new)[i] = value[i];
			else
				(_new)[i] = name[++newpos];
		}
	}
	else if(index >= idx + 1){
		//return strcutail(name, delm, matchStrPosAt(delm, name, -1));
		idx2 = matchStrPosAt(delm, name, index);
		//printf("idx2 = %d\n", idx2);
		_newStrLen = idx2 + vlen + 2;
		//_newStrLen = idx2 - delm_len + 1;
		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;
		(_new)[_newStrLen + 1] = '\0';

		newpos = 0;
		for(i = 0; i <= _newStrLen; i++){
			if(i <= idx2)
				(_new)[i] = name[i];
			else
				(_new)[i] = value[newpos++];
		}
	}
	else if(index > 1 && index < idx+1){
		idx1 = matchStrPosAt(delm, name, index - 1) + 1;
		idx2 = matchStrPosAt(delm, name, index) - delm_len + 1;
		_newStrLen = name_len + idx1 - idx2 + vlen + 1;

		if ((_new = (char *)malloc(sizeof(char*) * _newStrLen + 1)) == NULL)
			return NULL;
		(_new)[_newStrLen + 1] = '\0';

		newpos = 0;
		for(i = 0; i <= _newStrLen; i++){
			if(i < idx1)
				(_new)[i] = name[i];
			else if(i >= idx1 && (idx1 + vlen) > i)
				(_new)[i] = value[newpos++];
			else
				(_new)[i] = name[idx2++];
		}
	}
	//name = strdup(_new);
	strcpy(name, _new);
	//StrFree(_new); _new = NULL;
	return name;	
}
#else
char *modify_str(char *str, char *value, char *delm, int index)
{
#if 1
	char *tmp = StrDup(str);
	tmp = StrDup(delete_str(tmp, delm, index));
	//printf("modify_str : '%s'\n", tmp);
	tmp = StrDup(insert_str(tmp, value, delm, index));
	//printf("modify_str : '%s'\n", tmp);
	str = StrDup(tmp);
	StrFree(tmp);
#else
	char tmp[512], buf[512];

	strcpy(tmp, str);
	strcpy(buf, delete_str(tmp, delm, index));
	strcpy(tmp, insert_str(buf, value, delm, index));
	str = StrDup(tmp);
#endif
	return str;
}
#endif

char *str2digits(char *sval, char *delimiter, int length)
{
	int i, c = 0, find = 0;
	char *val = NULL, *tmp = NULL, *del = NULL;

	c = matchStrPosAt(delimiter, sval, -1) + 1;
	val = StrDup("");
	for(i = 1; i <= c; i++){
		del = StrDup(sval);
		tmp = StrDup(index_str(del, delimiter, i));
		if(strlen(tmp) == strspn(tmp, "0123456789")) {
			tmp = StrDup(insert_str(val, tmp, delimiter, i));
			val = StrDup(tmp);
			find++;
		}
		if(find == length) break;
	}
	sval = StrDup(val);
	StrFree(val); StrFree(tmp); StrFree(del);
	val = tmp = del = NULL;
	return sval;
}

int str2id(char *sval, char *delimiter)
{
	int i, c = 0;
	char *val = NULL, *tmp = NULL, *del = NULL;

	c = matchStrPosAt(delimiter, sval, -1) + 1;
	val = StrDup("");
	for(i = 1; i <= c; i++){
		del = StrDup(sval);
		tmp = index_str(del, delimiter, i);
		if(strlen(tmp) == strspn(tmp, "0123456789")) {
			tmp = insert_str(val, tmp, "", i);
			val = StrDup(tmp);
		}
	}
	//StrFree(val); StrFree(tmp); StrFree(del);
	//val = tmp = del = NULL;
	return atoi(val);
}

int val_exist(char *name, char *value, char *delm)
{
	int i, c, ret = 0;
	char *tmp, *id;
	
	c = matchStrPosAt(delm, name, -1) + 1;
	//if(c == 1) return ret;
	for (i = 1; i <= c ; i++) {
		tmp = StrDup(name);
		id = StrDup(index_str(tmp, delm, i));
		if(!strcmp(id, value)){
			ret = i;
			break;
		}
	}
	//StrFree(tmp);
	return ret;
}

char *get_one_line(char *s, int n, FILE *f)
{
	char *ptr = s;
	char buf[1];
	int bread;

 	if (n <= 0) return NULL;
	while (--n)
	{
		if(!((bread = fread(buf, sizeof(char), sizeof(char), f)) > 0))
		{
			if (ptr == s)  return NULL;
			break;
		}
	 
		if ((*ptr++ = *buf) == '\n') break;
	}
	 
	*ptr = '\0';
	return s;
}

int mac_validator(const char* value) {
	/* validate MAC format: XX:XX:XX:XX:XX:XX or XXXXXXXXXXXX
	 * XX is valid haxadecimal number (case insensitive)
	 */
	const char* q;
	int cnt = 5;
	if (value == NULL)
		return -9;
	q = value;
	for (; *q; q++) {
		if(strstr(q, ":")){
			if (*q == ':') {
				--cnt;
				if ((q - value) % 3 != 2) {
					return -1;
				}
			} 
		}
		else if (*q >= 'A' && *q <= 'F') {
		} else if (*q >= 'a' && *q <= 'f') {
		} else if (*q >= '0' && *q <= '9') {
		} else
			return -2;
	}
	if(strstr(q, ":"))
		if ((q - value) != 17)
			return -3;
	if(cnt == 0 || cnt == 5)
		return 0;
	else
		return -1;
}

/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
char *
fd2str(int fd)
{
	char *buf = NULL;
	size_t count = 0, n=0;

	do {
		buf = realloc(buf, count + 512);
		if (!buf) {
			sleep(1);
			continue;
		}	
		n = read(fd, buf + count, 512);
		if (n < 0) {
			free(buf);
			buf = NULL;
		}
		count += n;
	} while (n == 512);

	close(fd);
	if (buf)
		buf[count] = '\0';
	return buf;
}

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
char *
file2str(const char *path)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return NULL;
	}

	return fd2str(fd);
}

int str2file(char *path, char *string)
{
int fd;
int iLen;

	if( !string )
		return -1;

	iLen = strlen( string );
	
	if ((fd = open(path, O_CREAT | O_WRONLY | O_TRUNC ) )== -1)  {
		perror(path);
		return -1;
	}

	if( write( fd, string, iLen )  != iLen )
		return -1;

	close( fd );
	return iLen;
}

/* 
 * Waits for a file descriptor to change status or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
int
waitfor(int fd, int timeout)
{
	fd_set rfds;
	struct timeval tv = { timeout, 0 };

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	return select(fd + 1, &rfds, NULL, NULL, (timeout > 0) ? &tv : NULL);
}


/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
int
_eval(char *const argv[], char *path, int timeout, int *ppid)
{
	pid_t pid;
	int status;
	int fd;
	int flags;
	int sig;

	switch (pid = fork()) {
	case -1:	/* error */
		perror("fork");
		return errno;
	case 0:		/* child */
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		/* Clean up */
		ioctl(0, TIOCNOTTY, 0);
		close(STDIN_FILENO);
		setsid();

		/* Redirect stdout to <path> */
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				/* append to <path> */
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				/* overwrite <path> */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
		}

		/* execute command */
		dprintf("%s\n", argv[0]);
		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	/* parent */
		if (ppid) {
			*ppid = pid;
			return 0;
		} else {
			waitpid(pid, &status, 0);
			//printf("_eval::pid[%d] status[%d]\n", pid, status);
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

int vEval(char *const argv[])
{
pid_t pid;
int status;

	if((pid = vfork()) == 0)
	{
		// Child
		execvp(argv[0], argv);
		perror( argv[0] );
		exit( errno );
	}
	else if( pid == -1 )
	{
		perror("vfork");
		return errno;
	}
	else
	{
		// Parent
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		else
			return status;
	}
}

/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file
 * @return	0 on success and errno on failure
 */
int
kill_pidfile(char *pidfile)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];

	if (fp && fgets(buf, sizeof(buf), fp)) {
		pid_t pid = strtoul(buf, NULL, 0);
		fclose(fp);
		return kill(pid, SIGTERM);
  	} else
		return errno;
}


/**********************************
return value:
	0:	fail to open var/run/XXX.pid file
	pid: pid number
***********************************/
int get_task_pid(char *strTask)
{
	int pid; 
	char filename[80];
	char *strPid;

	
	snprintf(filename, sizeof(filename), "/var/run/%s.pid", strTask);
	if( (strPid = file2str( filename )) )
	{
		pid = atoi(strPid);
		free( strPid );
	}
	else
		pid = 0;;

	return pid;
}


/**********************************
return value:
	0:	task of this pid is dead
	1: 	task of this pid is alive
***********************************/
int pid_is_running(int pid)
{
	char filename[80];
	FILE *fp;
	
	snprintf(filename, sizeof(filename), "/proc/%d/status", pid);

	if((fp = fopen(filename, "r"))){
		//fgets(line, sizeof(line), fp);
        	/* Buffer should contain a string like "Name:   binary_name" */
		//sscanf(line, "%*s %s", name);
        	fclose(fp);
		return 1;
        }
	
	 //cprintf("cannot open %s\n", filename);
        return 0;
}


/********************************************************************
return value:
	0:	task is dead
	1:	task is alive
	2:	cannot open XXX.pid file(this task is never brought up from system start)
*********************************************************************/
int task_is_running(char *strTask)
{
	int pid;
	
	if( (pid=get_task_pid(strTask)) )
	{
		return pid_is_running(pid);
	}
	else
	{
		//cprintf("fail to open var/run/%s.pid\n", strTask);
		//return 2;
		return 0;
	}
}

long GetSysUpTime()
{
#if __linux__
struct sysinfo info;

	sysinfo(&info);
	return info.uptime;
#elif defined(darwin) || defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
	unsigned long UpTime;
	int mib[2] = {CTL_KERN, KERN_BOOTTIME};
	size_t len;
	struct timeval	uptime;
	int	now;
	
	len=sizeof(uptime);
	
	if(sysctl(mib,2,&uptime,(size_t *)&len,NULL,0) != 0)
	{
		(void)fprintf(stderr, "Could not get uptime\n");
		return -1;
	}
	
	now=time(NULL);
	
	UpTime=(double)(now-uptime.tv_sec);
	
	return UpTime;
#endif
}
