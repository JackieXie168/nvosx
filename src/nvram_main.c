
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "dni_bcmnvram.h"


extern void nvram_show(void);

#define BUFSIZE 16000		/* block sites max: 60*255=15300 */

static void
usage(void)
{
	fprintf(stderr, "usage: nvram [get name] [set name=value] [unset name] [show] [clean] [free]\n");
	exit(0);
}

int
main(int argc, char **argv)
{
	//char *cmd,*name,*value,buf[BUFSIZE],*ptr;
	char *name,*value, buf[BUFSIZE];
	char new_value[BUFSIZE];
	char *f_value;
	//int i,size,running=1,len,shmid;
	char *base = strrchr(argv[0], '/');

	base = base ? base + 1 : argv[0];
#if 0
	if (strstr(base, "iconv")) {
		return iconv(argv[1]);
	}
#endif
	
	/* Skip program name */
	--argc;
	++argv;

	if (!*argv)
		usage();

//	attach_share_memory();

	/* Process the remaining arguments. */
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3)) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
				{	
					#if 0 //For 5vt rc.config only			
					memcpy( (char *)new_value, value, strlen(value)-1);
					f_value = new_value + 1;					
					puts(f_value);
					#else
					puts(value);
					#endif		
				}
			}
		}
		else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
				strncpy(value = buf, *argv, sizeof(buf));
				name = strsep(&value, "=");
				//nvram_set(name, value);
				if (nvram_set(name, value) == ERR_NO_MEM) {
					printf("nvram full .... try to realloc ...\n");
					re_alloc();
					if (nvram_set(name, value) == ERR_NO_MEM) {
						printf("nvram full .... \n");
					}
				}
			}
		}
		else if (!strncmp(*argv, "free", 4)) {
			nvram_free();
		}
		else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv)
				nvram_unset(*argv);
		}
		else if (!strncmp(*argv, "commit", 5)) {
			nvram_commit();
		}
		else if (!strncmp(*argv, "show", 4) ||
			   !strncmp(*argv, "getall", 6)) {
			nvram_show();
		}
		else if (!strncmp(*argv, "getalltest", 10)) { 
			char *name;
			
			nvram_getall(buf, sizeof(buf));
			for (name = buf; *name; name = name + strlen(name) + 1)
			{	
				printf("%s\n", name);		
			}
		}		
		else if (!strncmp(*argv, "realloc", 7)) {
			re_alloc();
		}
		else if (!strncmp(*argv, "default", 7)) {
			nvram_default();
		}
#if 0
		else if (!strncmp(*argv, "default", 7)) {
			FILE *fp;
			fp = fopen(REGION_FILE_PATH, "r");
			if (fp) {
				fclose(fp);
				nvram_reload(DEFAULT_FILE_PATH_EU);
			}
			else 
				nvram_reload(DEFAULT_FILE_PATH);
		}
		else if (!strncmp(*argv, "reload", 6)) {
			nvram_reload(TMP_FILE_PATH);
		}
#endif
		if (!*argv)
			break;
	}
	detach_shm();
#if 0
	while (running)
	{
		fgets(line,BUFSIZE,stdin);

		ptr=line;
		clear_end(ptr);
		cmd=strsep(&ptr," ");
		if (!strncmp(cmd,"get",3))
			printf ("call nvram get to get %s= %s\n",ptr, nvram_get(ptr) );
		else if (!strncmp(cmd,"show",4))
			showvars("");
		else if (!strncmp(cmd,"set",3))
		{
			name=strsep(&ptr,"=");
			value=ptr;
			nvram_set(name,value);
		}
		else if (!strncmp(cmd,"commit",6))
		{
			nvram_commit();
		}
		else if (!strncmp(cmd,"md",2))
		{
			name=strsep(&ptr," ");
			value=ptr;
			len=atoi(value);
			dump_mem((void *)strtol(name,NULL,16),len);
		}
		else if (!strncmp(cmd,"end",3))
			running = 0;
		
	}
#endif //if 0 (for debug)
	return 0;
}

