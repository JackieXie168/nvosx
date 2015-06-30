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
#include <invram.h>

static void put_str(char *str)
{	
	int len= strlen(str);	

	if (len && (str[len-1] == '\r')) len--;	

	printf("%.*s\n", len, str);
}

static void invram_list(char *prefix, char *buf)
{
	int i;
	char *p, *value, name[512];

	p = buf;
	for (i = 1; ; i++) {
		sprintf(name, "%s%d", prefix, i);
		value = invram_get(name);
		if (*value == '\0')
			break;

		p += sprintf(p, "%s ", value);
	}

	if (p == buf)
		*p = '\0';
	else
		*--p = '\0'; /* Remove the last white-space */
}

int main(int argc, char *argv[])
{
	int size;
	char *name, *value, buf[NVRAM_SPACE];

	--argc;	
	++argv;	

	if (!*argv) {		
		fprintf(stdout, "usage: ----------------\n"
				"	show \n"
				"	commit \n"
				"	default \n"
				"	get name \n"
				"	set name=value \n"
				"	unset name \n"
				"	backup output-file-name \n"
				"	restore input-file-name \n"
				"	list name-prefix(name as name1 name2 ...)\n\n");
		return 0;
	}

	if (!strncmp(*argv, "get", 3)) {
		if (*++argv)
			put_str(invram_get(*argv));
	}
	else if (!strncmp(*argv, "set", 3)) {
		if (*++argv) {
			strcpy(name = buf, *argv);
			value = strchr(buf, '=');
			if (value == NULL)
				return 0;
			*value++ = '\0';
			invram_set(name, value);
		}
	}
	else if (!strncmp(*argv, "commit", 6)) {
		invram_commit();
	}
	else if (!strncmp(*argv, "unset", 5)) {
		if (*++argv)
			invram_unset(*argv);
	}
	else if (!strncmp(*argv, "default", 7)) {
		invram_default();
	}
	else if (!strncmp(*argv, "backup", 6)) {
		if (*++argv)
			return invram_backup(*argv);
	}
	else if (!strncmp(*argv, "restore", 7)) {
		if (*++argv)
			return invram_restore(*argv);
	}
	else if (!strncmp(*argv, "list", 3)) {
		if (*++argv) {
			invram_list(*argv, buf);
			put_str(buf);
		}
	}
	else if (!strncmp(*argv, "show", 4)) {
		value = invram_getall();				
		for (name = value; *name; name += strlen(name) + 1)
			put_str(name);
		size = sizeof(struct nvram_header) + (int) name - (int) value;
		printf("size: %d bytes (%d left)\n", size, NVRAM_SPACE - size);
	}

	return 0;
}
