/*
 * nvram_main.c
 *
 * Created by Jackie Xie on 2011-07-27.
 * Copyright 2011 Jackie Xie. All rights reserved.
 *
 */

#include <nvram.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <shutils.h>

extern const char	*__progname;

extern void nvram_show(void);

#define BUFSIZE 1024
char config_file_path[1024];

static void
usage(void)
{
	char help_msg[2048];

	sprintf(help_msg, "%s command summary :\n\n", __progname);
	sprintf(help_msg, "%s\t%s is a utility to set/get value to a given name to/from memory ,  and could parsing contents of nvram variable by delimiter(s).\n", help_msg, __progname);
	fprintf(stderr, "%s", help_msg);
#ifndef TARGET_DEVICE
	fprintf(stderr, "Usage : \n\t%s [get <name>] [set <name>=<value>] [unset <name>] [show] [clean] [commit] [import <file>] [reload <file>] [restore <file>] [backup <file>] [free]\n", __progname);
#else
#if __linux__
	fprintf(stderr, "Usage : \n\t%s [get name] [set name=value] [unset name] [show] [clean] [commit] [backup <file>] [restore <file>] [free]\n", __progname);
#elif defined(darwin) || defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
	fprintf(stderr, "Usage : \n\t%s [get <name>] [set <name>=<value>] [unset <name>] [show] [clean] [commit] [import <file>] [reload <file>] [restore <file>] [backup <file>] [free]\n", __progname);
#endif
#endif

	sprintf(help_msg, "\t%s [-s | -x <nvram variable>] {[-a | -m <values> -p <position>] | [ -n <to get # of digits>] | [-i <position>] | [-v <value>] | [-e <value>] | [ -r <position>] | [ -o <new delimiter>]} [-d delimiter]\n\n", __progname);
	strcat(help_msg, "\tparsing options.\n");
	strcat(help_msg, "\t\t<nvram variable>：input a nvram variable.\n");
	strcat(help_msg, "\t\tdelimiter：a delimiter which is a seperator.\n");
	strcat(help_msg, "\t\t-s : to specify a nvram variable.\n");
	strcat(help_msg, "\t\t-x : to get total counts of token by specified delimiter.\n");
	strcat(help_msg, "\t\t-d : to specify a delimiter.\n");
	strcat(help_msg, "\t\t-a：insert a value to the nvram variable by delimiter.\n");
	strcat(help_msg, "\t\t-r : remove a value from a nvram variable by specfied position.\n");
	strcat(help_msg, "\t\t-v : remove a value from a nvram variable by specfied value.\n");
	strcat(help_msg, "\t\t-e : to check a value in a nvram variable is exist or not.\n");
	strcat(help_msg, "\t\t-o : to replace old delimiters in a nvram variable by new ones.\n");
	strcat(help_msg, "\t\t-n : to get digits in a nvram variable by speficied counts.\n");
	strcat(help_msg, "\t\t-m : to change a value by spcified position from a nvram variable.\n");
	strcat(help_msg, "\t\t-i : to return the value which is at the specified position from a nvram variable by delimiter.\n\n");
	fprintf(stderr, "%s", help_msg);
	exit(0);
}

extern char flag_force_read_file;

int nvram_reload();


int id_exist(char *buf, int inst)
{
	char val[10], *id;
	int ret=1;
	
	sprintf(val, "%d", inst);

	id = strtok(buf, " ");
	while(id)
	{
		if(strcmp(val, id) == 0)
		{
			ret = 0;
			break;
		}
		id=strtok(NULL, " ");
		if(id == NULL)
			return return_empty(NULL);
	}

	return ret;
}

int get_ival(char *val, int num)
{
	char *qid, *id;
	int i;

	qid = StrDup(val);
	if(strlen(qid) > 0){
		id=strtok(qid, " ");
		i=0;			
		while(id){
			i++;
			if(i == num){
				return atoi(id);
			}
			id=strtok(NULL, " ");
		}
		return 0;
	}
	return 0;
}

int is_number(char *pchar)
{
	int i, is_num = 0;
	int lnum = strlen(pchar);

	for(i = 0; i <= lnum; i++)
	{
		if(!isalnum(*pchar)) break;
		printf("%c - ", *pchar);
		if(!isdigit(*pchar))
		{
			is_num++;
			break;
		}
		pchar++;
	}
	printf("%s\n", pchar);
	// printf("\nis_num = %i\n", is_num);
	return is_num;
}

int val2id(char *valist, int num, char *delm)
{
	char *id;
	int i;

	id = StrDup(valist);
	if(!val_exist(id, itos(num), delm))
		return num;
	else
		return 0;
}

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
		else if (!strncmp(*argv, "reload", 6) && argc > 1) {
			_nvram_reload(TMP_FILE_PATH);
		}
#endif
		else {
			*--argv;
			++argc;
			int randnum = 0;
			char* nvram = NULL, *value = NULL;
			char* delimiter = NULL, *tmp, *del;
			int position=0, c, i = 0, func = 0, val;
			char help_msg[2048] = "Usage: nvlist [-s | -x <nvram variable>] {[-a | -m <values> -p <position>] | [ -n <to get # of digits>] | [-i <position>] | [-v <value>] | [-e <value>] | [ -r <position>] | [ -o <new delimiter>]} [-d delimiter]\n\n";

			init_gen( );

			if(argc <= 1 || ((isgraph(*argv[1]) || ispunct(*argv[1])) && *argv[1]!='-')){
				usage();
				exit(0);
			}

			while ((c = getopt(argc, argv, "a:d:e:i:m:n:o:p:r:v:s:x:h")) != -1){
				switch (c) {
					case 'a':
						func = 1;
						value = optarg;
						break;
					case 'r':
						func = 2;
						position = atoi(optarg);
						break;
					case 'm':
						func = 3;
						value = optarg;
						break;
					case 's':
						nvram = optarg;
						break;
					case 'i':
						func = 4;
						position = atoi(optarg);
						break;
					case 'v':
						func = 5;
						value = optarg;
						position = 1;
						break;
					case 'x':
						func = 6;
						nvram = optarg;
						break;
					case 'e':
						func = 7;
						value = optarg;
						position = 1;
						break;
					case 'n':
						func = 8;
						val = atoi(optarg);
						position = 1;
						break;
					case 'o':
						func = 9;
						value = optarg;
						break;
					case 'p':
						position = atoi(optarg);
						break;
					case 'd':
						delimiter = optarg;
						break;
					case 'h':
					default:
						usage();
						exit(0);
						break;
				}
			}
			if(nvram_get(nvram) == NULL && delimiter != NULL && nvram != NULL){
				if(func == 1 && value != NULL){
					nvram_set(nvram, "");
					//printf("Inserting '%s' at position 1 ...\n", value);
					//tmp = StrDup(nvram_get(nvram));
					//value = insert_str(tmp, value, delimiter, 1);
					//tmp = StrDup(insert_str(nvram_get(nvram), value, delimiter, 1));
					//nvram_set(nvram, value);
					nvram_set(nvram, StrDup(insert_str(nvram_get(nvram), value, delimiter, 1)));
					printf("%s\n", nvram_get(nvram));
				}
				else{
					usage();
					//exit(0);
				}
			}
			else if(delimiter != NULL && position > 0 && nvram != NULL){
				if(func == 1 && value != NULL){
					//printf("Inserting %s at position %d ...\n", value, position);
					//tmp = StrDup(nvram_get(nvram));
					//value = insert_str(tmp, value, delimiter, 1);
					//nvram_set(nvram, value);
					nvram_set(nvram, insert_str(nvram_get(nvram), value, delimiter, position));
					printf("%s\n", nvram_get(nvram));
				}
				else if(func == 2){
					//printf("Deleting position %d of %s ... \n", position, nvram);
					//tmp = StrDup(nvram_get(nvram));
					//value = delete_str(nvram_get(nvram), delimiter, position);
					//nvram_set(nvram, value);
					nvram_set(nvram, delete_str(nvram_get(nvram), delimiter, position));
					printf("%s\n", nvram_get(nvram));
				}
				else if(func == 3 && value != NULL){
					//printf("Modifying position %d by %s ... \n", position, value);
					//tmp = StrDup(nvram_get(nvram));
					//value = modify_str(nvram_get(nvram), value, delimiter, position);
					//nvram_set(nvram, value);
					nvram_set(nvram, StrDup(modify_str(nvram_get(nvram), value, delimiter, position)));
					printf("%s\n", nvram_get(nvram));
				}
				else if(func == 4){
					//printf("\nReading the values from nvram variable %s ... \n", nvram);
					tmp = StrDup(nvram_get(nvram));
					printf("%s\n", index_str(tmp, delimiter, position));
				}
				else if(func == 5){
					/*c = matchStrPosAt(delimiter, StrDup(nvram_get(nvram)), -1) + 1;
					for (i = 1; i <= c ; i++) {
						tmp = StrDup(nvram_get(nvram));
						printf("tmp = %s\n", tmp);
						del = StrDup(index_str(tmp, delimiter, i));
						printf("%s[%d] = %s\n", nvram, i, del);
						if(!strcmp(del, value)){
							printf("value %s is matched in %s\n", value, nvram_get(nvram));
							nvram_set(nvram, delete_str(nvram_get(nvram), delimiter, i));
							printf("delete '%s' which is spcified at position %d\n", value, i);
							break;
						}
					}*/
					nvram_set(nvram, delete_val(nvram_get(nvram), value, delimiter));
					printf("%s\n", nvram_get(nvram));
				}
				else if(func == 7){
					tmp = StrDup(nvram_get(nvram));
					if(val_exist(tmp, value, delimiter))
						printf("%s is exist\n", value);
					else
						printf("%s is not exist\n", value);
				}
				else if(func == 8){
					tmp = StrDup(nvram_get(nvram));
					printf("%s\n", str2digits(tmp, delimiter, val));
				}
				else{
					usage();
					//exit(0);
				}
			}
			else if(func == 6 && delimiter != NULL && nvram != NULL){
				position = matchStrPosAt(delimiter, nvram_get(nvram), -1) + 1;
				printf("%d\n", position);
				/*printf("Number of token counts is %d\n", position);
				for(i = 1; i <= position; i++){
					tmp = StrDup(nvram_get(nvram));
					printf("\t%s\n", index_str(tmp, delimiter, i));
				}*/
			}
			else if(func == 9 && delimiter != NULL && nvram != NULL && value != NULL){
				/* this code section is fix the old delimiter(s) is a subset of new delimiter(s) issue */
				randnum = number_range(6, 16);
				tmp = StrDup(nvram_get(nvram));
				if(strstr(value, delimiter) && strlen(value) > strlen(delimiter)){
					char temp_deli[]="olddeliisasubsetofnewdeli";
					random_string(temp_deli, randnum);
					nvram_set(nvram, replaceall(tmp, delimiter, temp_deli, -2, matchStrPosAt(delimiter, tmp, -1)));
					strcpy(tmp, nvram_get(nvram));
					nvram_set(nvram, replaceall(tmp, temp_deli, value, -2, matchStrPosAt(temp_deli, tmp, -1)));
				}
				else
					nvram_set(nvram, replaceall(tmp, delimiter, value, -2, matchStrPosAt(delimiter, tmp, -1)));
				printf("%s\n", nvram_get(nvram));
			}
			else{
				usage();
				//exit(0);
			}
			exit(0);
		}
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

