/*
 * Shell-like utility functions
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dni_bcmnvram.h>
#include <netinet/ip_icmp.h>
#include <dni_shutils.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include <linux/sockios.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <signal.h>
#include <sys/sysinfo.h>



#define cprintf     printf






/*
 * Reads file and returns contents
 * @param   fd  file descriptor
 * @return  contents of file or NULL if an error occurred
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
 * @param   path    path to file
 * @return  contents of file or NULL if an error occurred
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
 * @param   fd  file descriptor
 * @param   timeout seconds to wait before timing out or 0 for no timeout
 * @return  1 if descriptor changed status or 0 if timed out or -1 on error
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
 * @param   argv    argument list
 * @param   path    NULL, ">output", or ">>output"
 * @param   timeout seconds to wait before timing out or 0 for no timeout
 * @param   ppid    NULL to wait for child termination or pointer to pid
 * @return  return value of executed command or errno
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
    case -1:    /* error */
        perror("fork");
        return errno;
    case 0:     /* child */
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
    default:    /* parent */
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
 * @param   pidfile PID file
 * @return  0 on success and errno on failure
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


/*
 * Convert Ethernet address binary data to string representation
 * @param   e   binary data
 * @param   a   string in xx:xx:xx:xx:xx:xx notation
 * @return  a
 */
char *
ether_etoa(const unsigned char *e, char *a)
{
    char *c = a;
    int i;

    for (i = 0; i < ETHER_ADDR_LEN; i++) {
        if (i)
            *c++ = ':';
        c += sprintf(c, "%02X", e[i] & 0xff);
    }
    return a;
}


/**********************************
return value:
    0:  fail to open var/run/XXX.pid file
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
    0:  task of this pid is dead
    1:  task of this pid is alive
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
    0:  task is dead
    1:  task is alive
    2:  cannot open XXX.pid file(this task is never brought up from system start)
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

unsigned long GetSysFreeMem()
{
    struct sysinfo info;

    sysinfo(&info);

    return info.freeram + info.bufferram;

}

long GetSysUpTime()
{
struct sysinfo info;

    sysinfo(&info);
    return info.uptime;
}


/*
 * re865x Ioctl (to call kernel functions including rtl8651 table driver APIs
 */
int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    unsigned int args[4];
    struct ifreq ifr;
    int sockfd;

    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("fatal error socket\n");
        return -3;
    }
    strcpy((char*)&ifr.ifr_name, name);
    ((uint32 *)(&ifr.ifr_data))[0] = (uint32)args;
    if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
    {
        perror("device ioctl:");
        close(sockfd);
        return -1;
    }
    close(sockfd);
    return 0;
} /* end re865xIoctl */




/* ======================(function header)========================
Function Name:char* itos (){}
Description:to convert integer to string
Arguments:unsigned n
Return:a string
written by jackyxie
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
#ifdef DNI_LOG
/*********************************************************************
*
@(#)Function:   writeLog
*
* Description:
*
*   Write message to DNI log file.
*
* Input Parameters:
*
* Output Parameters:
*   none.
*
* Error Return:
*   none.
*
* Note:
*
*********************************************************************/
#include "../dni-log/dni_log.h"
int
writeLog(char *file_name, const char *fmt, ...)
{
    FILE *out_file;
    int len;
    va_list argptr;
    int nbytes;
    char message[200];

    va_start(argptr, fmt);
    nbytes = vsprintf(message, fmt, argptr);
    va_end(argptr);

    len = strlen(message);

    if(len <= 0)
        perror("error : send message -> NULL");
    if((out_file=fopen(file_name,"w")) == NULL){
        perror("error : open log file error !!!");
        return -1;
    }
    fwrite(message, 1, len,out_file);
    fclose(out_file);
    return 0;
}
#endif/*DNI_LOG*/


int
_calcsum_(const char *ifname)
{
        FILE *fin;
        unsigned char ch;
        unsigned char checksum = 0;
        int fdin, len = 0;
        int exitcode = 0;

        if ((fin = fopen(ifname, "rb")) == NULL){
			exit(errno);
		}
        fdin = fileno(fin);

        while (read(fdin, &ch, 1))
		{
			checksum = (checksum + ch) & 0xFF;
			len++;
		}
        checksum = ~checksum;
	/* this output for shell script */
	printf("checksum = 0x%02X--%03o, len = %d\n", checksum, checksum,len);

        if (checksum != 0x00)
                exitcode = checksum;
        fclose(fin);
        return exitcode;
}

