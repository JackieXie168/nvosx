/*
 * Copyright(c)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *	this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	this list of conditions and the following disclaimer in the documentation
 *	and/or other materials provided with the distribution.
 * 3. Neither the name of the vendors nor the names of its contributors
 *	may be used to endorse or promote products derived from this software
 *	without specific prior written permission.
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
 
/***********************************************************
 *																																	*
 *		nvramd：Small application for saving data on embedded system. It runs as		*
 *						multi-clients & server mode with socket-call.										*
 *																																	*
 *		Usage：																												*
 *				int main(int argc, char *argv[])																	*
 *				{																													*
 *					srv_nvram_loop();																							*
 *					return 0;																									*
 *				}																													*
 *																																	*
 ***********************************************************/

#include <nvram.h>

#define RAND_MAGIC					NVRAM_MAGIC
#define PATH_NVRAM_MTD		_PATH_NVRAM_MTD
#define HTABLE_SIZE					1024									/* MUST be power of 2 */
#define NAME_SPACE					0x80000								/* 32KB seems to be enough */
#define BUFF_SPACE						(NVRAM_SPACE * 2)		/* Mainly for TOO-BIG size value */

extern struct nvram_tuple defaults_nvram[];
extern int mtd_write(const char *mtd, char *buf, int len);
extern int mtd_read(const char *mtd, char *buf, int len);

static int buff_offset = 0;
static int name_offset = 0;
static char nvram_buff[BUFF_SPACE];
static char nvram_name[NAME_SPACE] __attribute__((aligned(4)));
static struct nvram_tuple *nvram_hash[HTABLE_SIZE];

#define hashtbl(i)	(nvram_hash[i])
#define val_buff(offset)		((void *)&nvram_buff[offset])
#define ctrl_buff(offset)		((void *)&nvram_name[offset])
#define WORD_ALIGN(s)	(((s) + 3) & ~3)	/* Aligned with 4 bytes */

static void restore_defaults(void);

/* 'WORD_ALIGN' will make sure the length of data is multiple of 4. */
static uint32 calc_sum(uint8 *pdata, uint32 nbytes, uint32 crc)
{
	uint32 *p = (uint32 *) pdata;

	nbytes >>= 2;

	while (nbytes-- > 0)
		crc += *p++;

	return ~crc;
}

static void calc_magic(uint8 *pdata, uint32 nbytes)
{	
	uint32 *p = (uint32 *)pdata;	

	nbytes >>= 2;		

	srand((uint32)RAND_MAGIC);		

	while(nbytes-- > 0)
		*p++ ^= (uint32)rand();
}

/* 'ELF' hash function */
static uint32 name_hash(char *name)
{
	uint32 h = 0, g;

	while(*name) {
		h = (h<<4) + *name++;

		if ((g = (h & 0xF0000000)))
			h ^= g>>24;

		h &=~g;
	}

	return h & (HTABLE_SIZE - 1);
}

static void __nvram_free(void)
{
	int i;

	for (i = 0; i < HTABLE_SIZE; i++)
		hashtbl(i) = NULL;

	buff_offset = 0;
	name_offset = 0;
}

static struct nvram_tuple *__nvram_realloc(struct nvram_tuple *t, char *name, char *value)
{
	int len = strlen(value) + 1;

	if ((buff_offset + len) > BUFF_SPACE)
		return NULL;

	if (t == NULL) {
		int nlen = WORD_ALIGN(strlen(name) + sizeof(struct nvram_tuple) + 1);

		if ((name_offset + nlen) > NAME_SPACE)
			return NULL;

		t = ctrl_buff(name_offset);
		t->name = (char *) &t[1];
		t->value = NULL;
		strcpy(t->name, name);

		name_offset += nlen;
	}

	if (!t->value ||strcmp(t->value, value)) {
		t->value = val_buff(buff_offset);
		strcpy(t->value, value);

		buff_offset += len;
	}

	return t;
}

static int __nvram_set(char *name, char *value)
{
	uint32 i;
	struct nvram_tuple *t, *u;
 
	i = name_hash(name);

	for (t = hashtbl(i); t && strcmp(t->name, name); t = t->next);

	if ((u = __nvram_realloc(t, name, value)) == NULL)
		return -1;

	if (t == NULL) {
		u->next = hashtbl(i);
		hashtbl(i) = u;
	}

	return 0;
}

static char * __nvram_get(char *name)
{
	uint32 i;
	struct nvram_tuple *t;

	i = name_hash(name);

	for (t = hashtbl(i); t && strcmp(t->name, name); t = t->next);

	return (t ? t->value : "");
}

static void __nvram_setall(struct nvram_header *header)
{
	char *name, *value, *end, *eq;

	/* name=value\0 ... \0\0 */
	name = (char *) &header[1];
	end = (char *) header + NVRAM_SPACE - 2;
	end[0] = end[1] = '\0';

	for (; *name; name = value + strlen(value) + 1) {
		if ((eq = strchr(name, '=')) == NULL)
			break;

		*eq = '\0';
		value = eq + 1;
		__nvram_set(name, value);
		*eq = '=';
	}
}

static void __nvram_rehash(struct nvram_header *header)
{
	__nvram_free();

	__nvram_setall(header);
}

static void __nvram_getall(struct nvram_header *header)
{
	int i, total = 0;
	char *ptr, *end;
	struct nvram_tuple *t;

	ptr = (char *)&header[1];
	memset(ptr, 0, NVRAM_SPACE - sizeof(struct nvram_header));

	end = (char *)header + NVRAM_SPACE - 2;

	for (i = 0; i < HTABLE_SIZE; i++) {
		for (t = hashtbl(i); t; t = t->next) {
			if (t->name && t->value) {
				fprintf(stderr, "DEBUG GETALL: %s=%s\n", t->name, t->value);
				total++;
			}
			if ((ptr + strlen(t->name) + strlen(t->value) + 2) > end)
				break;

			ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
		}
	}
	fprintf(stderr, "DEBUG GETALL: total vars = %d, raw_len = %ld\n",
			total, (long)(ptr - (char *)&header[1]));

	ptr += 2;

	header->magic = NVRAM_MAGIC;
	header->len = WORD_ALIGN(ptr - (char *)&header[1]);
	header->crc_ver_init = calc_sum((uint8 *)&header[1], header->len, 0);
}

static void __nvram_commit(struct nvram_header *header)
{
	__nvram_getall(header);

	__nvram_rehash(header);
}

static int __nvram_save(struct nvram_header *header)
{
	int len;

	len = sizeof(struct nvram_header) + header->len;

	return mtd_write(PATH_NVRAM_MTD, (char *)header, len);
}

static void srv_nvram_backup(int fd, struct sockaddr_in *from, socklen_t slen, char *ofile)
{	
	FILE *fp;
	char result = BU_FAILURE;
	struct nvram_header *hd = NULL;

	if ((fp = fopen(ofile, "wb")) == NULL ||(hd = malloc(NVRAM_SPACE)) == NULL)
		goto ret;

	__nvram_getall(hd);

	calc_magic((uint8 *)&hd[1], hd->len);

	fwrite(hd, 1, hd->len + sizeof(struct nvram_header), fp);

	result = BU_SUCCESS;

ret:
	if (fp) fclose(fp);
	if (hd) free(hd);

	sendto(fd, &result, sizeof(result), 0, (struct sockaddr const *)from, slen);
}

static void srv_nvram_restore(int fd, struct sockaddr_in *from, socklen_t slen, char *ifile)
{	
	int rlen;
	FILE *fp;
	char result = RSTR_ERRFILE;
	struct nvram_header *hd = NULL;

	if ((fp = fopen(ifile, "rb")) == NULL ||(hd = malloc(NVRAM_SPACE)) == NULL)		
		goto ret;	

	rlen = fread(hd, 1, NVRAM_SPACE, fp) - sizeof(struct nvram_header);

	if (rlen < 0 ||hd->magic != NVRAM_MAGIC ||hd->len != rlen)
		goto ret;	

	calc_magic((uint8 *)&hd[1], hd->len);

	result = RSTR_ERRCSUM;
	if (calc_sum((uint8 *)&hd[1], hd->len, hd->crc_ver_init))
		goto ret;
		
	restore_defaults();
	__nvram_setall(hd);
	__nvram_save(hd);

	result = RSTR_SUCCESS;

ret:
	if (fp) fclose(fp);
	if (hd) free(hd);

	sendto(fd, &result, sizeof(result), 0, (struct sockaddr const *)from, slen);
}

static void srv_nvram_set(int fd, struct sockaddr_in *from, socklen_t slen, char *buf)
{
	char result = 1; /* means nothing, just for sync with client */
	char *name, *value;
	struct nvram_header *header;

	if ((value = strchr(buf, '=')) == NULL)
		goto ret;

	name = buf; *value++ = '\0';

	if (__nvram_set(name, value) && (header = malloc(NVRAM_SPACE))) {
		__nvram_commit(header); /* consolidate space and try again */
		__nvram_set(name, value);

		free(header);
	}
ret:
	sendto(fd, &result, sizeof(result), 0, (struct sockaddr const *)from, slen);
}

static void srv_nvram_unset(int fd, struct sockaddr_in *from, socklen_t slen, char *name)
{
	uint32 i;
	char result = 1; /* means nothing, just for sync with client */
	struct nvram_tuple *t;
	struct nvram_tuple **pprev;

	i = name_hash(name);

	for (pprev = &hashtbl(i), t = *pprev; t && strcmp(t->name, name);
		pprev = &t->next, t = *pprev);

	if (t != NULL)
		*pprev = t->next;

	sendto(fd, &result, sizeof(result), 0, (const struct sockaddr *)from, slen);
}

static void srv_nvram_get(int fd, struct sockaddr_in *from, socklen_t slen, char *name)
{
	char *value;

	value = __nvram_get(name);

	sendto(fd, value, strlen(value) + 1, 0, (struct sockaddr const *)from, slen);
}

static void srv_nvram_getall(int fd, struct sockaddr_in *from, socklen_t slen)
{
	int len = 1;
	char *value = "";
	struct nvram_header *header;

	if ((header = malloc(NVRAM_SPACE)) != NULL) {
		__nvram_getall(header);

		value = (char *)&header[1];
		len = (int)header->len;
	}

	sendto(fd, value, len, 0, (const struct sockaddr *)from, slen);

	if (header != NULL)
		free(header);
}

static void srv_nvram_show(int fd, struct sockaddr_in *from, socklen_t slen)
{
    int i, len = 1;
    char *value = "";
    struct nvram_tuple *t;
    char *ptr, *end;
    char *buf = NULL;

    buf = malloc(NVRAM_SPACE);
    if (buf == NULL)
        goto send;

    ptr = buf;
    end = buf + NVRAM_SPACE - 2;

    for (i = 0; i < HTABLE_SIZE; i++) {
        for (t = hashtbl(i); t; t = t->next) {
            if (t->name && t->value) {
                int n = snprintf(ptr, end - ptr, "%s=%s\n", t->name, t->value);
                if (n < 0 || ptr + n >= end)
                    break;
                ptr += n;
            }
        }
    }
    *ptr = '\0';
    len = ptr - buf + 1;
    value = buf;

send:
    sendto(fd, value, len, 0, (const struct sockaddr *)from, slen);
    if (buf) free(buf);
}

static void srv_nvram_commit(int fd, struct sockaddr_in *from, socklen_t slen)
{
	struct nvram_header *hd;
	char result = CMIT_FAILURE;

	if ((hd = malloc(NVRAM_SPACE)) != NULL) {
		__nvram_commit(hd);

		if (__nvram_save(hd) == 0)
			result = CMIT_SUCCESS;

		free(hd);
	}

	sendto(fd, &result, sizeof(result), 0, (struct sockaddr const *)from, slen);
}

static void srv_nvram_default(int fd, struct sockaddr_in *from, socklen_t slen)
{
	struct nvram_header *hd;
	char result = DFLT_FAILURE;

	if ((hd = malloc(NVRAM_SPACE)) != NULL) {
		restore_defaults();

		__nvram_commit(hd);

		if (__nvram_save(hd) == 0)
			result = DFLT_SUCCESS;

		free(hd);
	}

	sendto(fd, &result, sizeof(result), 0, (struct sockaddr const *)from, slen);
}

static void restore_defaults(void)
{
	struct nvram_tuple *t;
	int count=0;

	__nvram_free();

	for (t = defaults_nvram; t->name; t++) {
		__nvram_set(t->name, t->value);
		count++;
	}
	fprintf(stderr, "DEBUG: restored %d defaults\n", count);
}

static void srv_nvram_load(void)
{
	int rlen;
	struct nvram_header *header;

	printf("Loading data from " PATH_NVRAM_MTD " ...\n");

	header = malloc(NVRAM_SPACE);
	if (header == NULL)
		goto def;

	rlen = mtd_read(PATH_NVRAM_MTD, (char *)header, NVRAM_SPACE)
			- sizeof(struct nvram_header);
	if (rlen < 0 ||header->magic != NVRAM_MAGIC ||rlen < header->len)
		goto def;

	if (calc_sum((uint8 *)&header[1], header->len, header->crc_ver_init))
		goto def;

	printf("The data configuration is Valid\n");

	restore_defaults();

	__nvram_setall(header);

	free(header);

	return;

def:
	if (header) free(header);

	printf("Resetting to default values ...\n");

	restore_defaults();
}

static void _srv_nvram_loop(void)
{
	int r, fd;
	char	cmd;
	socklen_t slen;
	fd_set readable;
	struct sockaddr_in	addr;
	struct sockaddr_in	from;
	static char buf[NVRAM_SPACE];	/* so wasting memory ?_? */

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(UDP_PORT);
	addr.sin_addr.s_addr = htonl(UDP_IPADDR);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return;

	/* Optimization: transmiting data from 'lo' is reliable, use UDP zero-csum. */
	int nocsum = UDP_CSUM_NOXMIT;
#if __linux__
	setsockopt(fd, SOL_SOCKET, SO_NO_CHECK, &nocsum, sizeof(nocsum));
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &nocsum, sizeof(nocsum));
#endif
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		goto ret;
	}
	srv_nvram_load();

	printf("The data center is Running ...\n");
	daemon(1, 1);

	while (1) {
		FD_ZERO(&readable);
		FD_SET(fd, &readable);

		if (select(fd + 1, &readable, NULL, NULL, NULL) < 1)
			continue;

		slen = sizeof(struct sockaddr_in);
		r = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, &slen);
		if (r < 1)
			continue;
		buf[r] = '\0';

		cmd = buf[0];

		if (cmd == CMD_GET)
			srv_nvram_get(fd, &from, slen, buf + 1);
		else if (cmd == CMD_SET)
			srv_nvram_set(fd, &from, slen, buf + 1);
		else if (cmd == CMD_UNSET)
			srv_nvram_unset(fd, &from, slen, buf + 1);
		else if (cmd == CMD_COMMIT)
			srv_nvram_commit(fd, &from, slen);
		else if (cmd == CMD_DEFAULT)
			srv_nvram_default(fd, &from, slen);
		else if (cmd == CMD_GETALL)
			srv_nvram_getall(fd, &from, slen);
		else if (cmd == CMD_BACKUP)
			srv_nvram_backup(fd, &from, slen, buf + 1);
		else if (cmd == CMD_RESTORE)
			srv_nvram_restore(fd, &from, slen, buf + 1);
		else if (cmd == CMD_SHOW)
			srv_nvram_show(fd, &from, slen);
	}

ret:
	printf("Can't start the data center!\n");
	close(fd);
}

void srv_nvram_loop()
{
	_srv_nvram_loop();
}
#if 0
int main(int argc, char *argv[])
{
	_srv_nvram_loop();
	
	return 0;
}
#endif