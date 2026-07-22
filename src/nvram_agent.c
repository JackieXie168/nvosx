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

#include <nvram.h>

int nvram_match(char *name, char *match) 
{
	char *value;

	value = nvram_get(name);
	return strcmp(value, match) == 0;
}

int nvram_invmatch(char *name, char *invmatch) 
{
	char *value;

	value  = nvram_get(name);
	return strcmp(value, invmatch);
}

static int nvram_fd = -1;
void init_nvram(void)
{
	if (nvram_fd >= 0)
		close(nvram_fd);

	nvram_fd = -1;
}

static int init_socket(void)
{
	int fd;
	struct sockaddr_in to;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = htons(UDP_PORT);
	to.sin_addr.s_addr = htonl(UDP_IPADDR);
	/*
	  * Optimization: UDP connectionless protocol sockets use connect() to associate 
	  * with 'UDP_IPADDR:UDP_PORT', instead of lookuping route table in kernel by 
	  * calling ip_route_output_flow(...) when send packets.
	  *
	  * And if the `datalib` is NOT running, the linux will send ICMP Port Unreachable,
	  * then select() can return back soon, and recvfrom() gets "Connection refused".
	  */
	if (connect(fd, (struct sockaddr *)&to, sizeof(to)) < 0) {
		close(fd);
		return -1;
	}

	/* Optimization: transmiting data from 'lo' is reliable, use UDP zero-csum. */
	int nocsum = UDP_CSUM_NOXMIT;
#if __linux__
	setsockopt(fd, SOL_SOCKET, SO_NO_CHECK, &nocsum, sizeof(nocsum));
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX) || defined(darwin)
	setsockopt(fd, SOL_SOCKET, NULL, &nocsum, sizeof(nocsum));
#endif
	return fd;
}

static char *do_nvram(char cmd, char *arg[])
{
	#define NUM_RETRIES	3
	int retries;
	int r, len, fd;
	fd_set readable;
	struct timeval timo;
	static char buf[NVRAM_SPACE];	/* so wasting memory ?_? */

	if ((fd = nvram_fd) == -1) {
		if ((fd = init_socket()) == -1)
			return "";
		nvram_fd = fd;
	}

	buf[0] = cmd;
	if (cmd == CMD_GET ||cmd == CMD_UNSET ||cmd == CMD_BACKUP ||cmd == CMD_RESTORE)
		len = snprintf(&buf[1], sizeof(buf) - 2, "%s", arg[0]) + 1;
	else if (cmd == CMD_SET)
		len = snprintf(&buf[1], sizeof(buf) - 2, "%s=%s", arg[0], arg[1]) + 1;
	else
		len = 1;
	if (sendto(fd, buf, len, 0, NULL, 0) < 1)
		return "";

	retries = 0;
again:
	timo.tv_sec = UDP_TIMEOUT;
	timo.tv_usec = 0;
	FD_ZERO(&readable);
	FD_SET(fd, &readable);
	r = select(fd + 1, &readable, NULL, NULL, &timo);
	if (r == 0)	/* Timeout */
		return "";
	if (r < 0) { /* On error, catch a signal ... wait again. */
		if (++retries < NUM_RETRIES)
			goto again;
		return "";
	}

	r = recvfrom(fd, buf, sizeof(buf) - 1, 0, NULL, 0);
	buf[r < 0 ? 0 : r] = '\0';

	return buf;
}

char *nvram_get(char *name)
{
	char *arg[1];
	arg[0] = name;
	return do_nvram(CMD_GET, arg);
}

void nvram_set(char *name, char *value)
{
	char *arg[2];
	char *action_ptr, *line=NULL;
	int len;

	arg[0] = name;
	arg[1] = value;
	
	//modified by dvd.chen to queue action for monitor.sh to later process.
	if (strcmp(name, "action") == 0) {		/* action queue */	
		if (value)
			len  = strlen(value);
		else
			len=0;
			
		action_ptr = NULL;
		action_ptr=nvram_get("action");
		if (action_ptr[0] != '\0') {
			len += strlen(action_ptr);
			len += 2;  //for in-between space and null-end char.
		}
		
		line=malloc(len);
		
		if (action_ptr[0] != '\0')
			sprintf(line,"%s %s", action_ptr, value);
		else
			sprintf(line,"%s",value);
			
		arg[1] = line;
	}
	
	do_nvram(CMD_SET, arg);
	
	if (strcmp(name, "action") == 0) {
		//printf("##nvram recv action=%s, len[%d]\n", line, len);
		system("killall -9 alarm");
	}
	
	if(line)
		free(line);
}

void nvram_unset(char *name)
{
	char *arg[1];
	arg[0] = name;
	do_nvram(CMD_UNSET, arg);
}

char *nvram_getall(void)
{
	return do_nvram(CMD_GETALL, NULL);
}

char *nvram_show(void)
{
	return do_nvram(CMD_SHOW, NULL);
}

void nvram_default(void)
{
	char *ret;

	printf("Resetting to Default... ");
	ret = do_nvram(CMD_DEFAULT, NULL);
	printf("%s\n", *ret == DFLT_SUCCESS ? "Done!" : "Fail!");	
}

void nvram_commit(void)
{
	char *ret;

	printf("Saving Data... ");
	ret = do_nvram(CMD_COMMIT, NULL);
	printf("%s\n", *ret == CMIT_SUCCESS ? "Done!" : "Fail!");
}

/*
  * converting the return value mainly for `echo $?`usage. 
  *
  * 0 : success; 1 : fail.
  */
int nvram_backup(char *ofile)
{
	char *ret, *arg[1];

	arg[0] = ofile;
	ret = do_nvram(CMD_BACKUP, arg);
	return *ret == BU_SUCCESS ? 0 : 1;
}

/* 
  * converting the return value mainly for `echo $?`usage. 
  *
  * 0 : success; 1 : file error; 2 : csum err; 3 : unknown.
  */
int nvram_restore(char *ifile)
{
	char *ret, *arg[1];

	arg[0] = ifile;
	ret = do_nvram(CMD_RESTORE, arg);

	if (*ret == RSTR_SUCCESS)
		return 0;
	else if (*ret == RSTR_ERRFILE)
		return 1;
	else if (*ret == RSTR_ERRCSUM)
		return 2;
	else
		return 3;
}
