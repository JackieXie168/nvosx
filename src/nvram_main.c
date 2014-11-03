/*
 * nvram_main.c
 *
 * Created by Jackie Xie on 2011-07-27.
 * Copyright 2011 Jackie Xie. All rights reserved.
 *
 */

#include <nvram.h>

extern void nvram_show(void);

#define BUFSIZE 1024
char config_file_path[1024];

static void
usage(void)
{
#ifndef TARGET_DEVICE
	fprintf(stderr, "usage: nvram [get <name>] [set <name>=<value>] [unset <name>] [show] [clean] [commit] [import <file>] [reload <file>] [restore <file>] [backup <file>] [free]\n");
#else
#if __linux__
	fprintf(stderr, "usage: nvram [get name] [set name=value] [unset name] [show] [clean] [commit] [backup <file>] [restore <file>] [free]\n");
#elif defined(darwin) || defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
	fprintf(stderr, "usage: xnvram [get <name>] [set <name>=<value>] [unset <name>] [show] [clean] [commit] [import <file>] [reload <file>] [restore <file>] [backup <file>] [free]\n");
#endif
#endif
	exit(0);
}

extern char flag_force_read_file;

int nvram_reload();

int
main(int argc, char **argv)
{
	//char *cmd,*name,*value,buf[BUFSIZE],*ptr;
	char *name,*value, buf[BUFSIZE];
	char new_value[BUFSIZE];
	char *f_value;
	//int i,size,running=1,len,shmid;
	sprintf( config_file_path,"%s",TMP_FILE_PATH);
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
				nvram_set(name, value);
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
			struct stat st = {0};
			if (stat(CONF_PATH, &st) == -1) {
				mkdir(CONF_PATH, 0700);
			}
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
			FILE *fp;
			fp = fopen(REGION_FILE_PATH, "r");
			if (fp) {
				fclose(fp);
				_nvram_reload(DEFAULT_FILE_PATH_EU);
			}
			else 
				_nvram_reload(DEFAULT_FILE_PATH);
		}
		else if (!strncmp(*argv, "backup", 6)) {
			if (*++argv) nvram_backup(*argv);
		}
		else if (!strncmp(*argv, "restore", 7)) {
			if (*++argv){
				_nvram_reload(*argv);
				nvram_clean();
			}
		}
		else if (!strncmp(*argv, "clean", 5)) {
			nvram_clean();
		}
#ifndef TARGET_DEVICE
		else if(!strncmp(*argv,"reload",6) && ++argv != 0x00 ){
			sprintf( config_file_path,"%s",  (char*) *argv );
			printf("\n config_file_path = %s \n",config_file_path);
			nvram_reload();
		}
		else if (!strncmp(*argv, "import", 6)) {
			if (*++argv) {
				if (*argv)
				{	
					printf("\nDo nvram_import(%s) ...\n", *argv);
					nvram_import(*argv);
				}
			}
		}
#else
		else if (!strncmp(*argv, "reload", 6)) {
			_nvram_reload(TMP_FILE_PATH);
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

