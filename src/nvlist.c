/*
 * nvlist.c
 *
 * Created by Jackie Xie on 2012-02-23.
 * Copyright 2011 Jackie Xie. All rights reserved.
 *
 */

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <nvram.h>
#include <shutils.h>

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

int main(int argc, char* argv[])
{
	int randnum = 0;
	char* nvram = NULL, *value = NULL;
	char* delimiter = NULL, *tmp, *del;
	int position=0, c, i = 0, func = 0, val;
	char help_msg[2048] = "Usage: nvlist [-s | -x <nvram variable>] {[-a | -m <values> -p <position>] | [ -n <to get # of digits>] | [-i <position>] | [-v <value>] | [-e <value>] | [ -r <position>] | [ -o <new delimiter>]} [-d delimiter]\n\n";

	init_gen( );
	strcat(help_msg, "nvlist command summary\n");
	strcat(help_msg, "\tnvlist is a function to insert/delete/modify/view nvram variable by delimiter(s).\n");
	strcat(help_msg, "\t<nvram variable>：input a nvram variable.\n");
	strcat(help_msg, "\tdelimiter：a delimiter which is a seperator.\n");
	strcat(help_msg, "\t-s : to specify a nvram variable.\n");
	strcat(help_msg, "\t-x : to get total counts of token by specified delimiter.\n");
	strcat(help_msg, "\t-d : to specify a delimiter.\n");
	strcat(help_msg, "\t-a：insert a value to the nvram variable by delimiter.\n");
	strcat(help_msg, "\t-r : remove a value from a nvram variable by specfied position.\n");
	strcat(help_msg, "\t-v : remove a value from a nvram variable by specfied value.\n");
	strcat(help_msg, "\t-e : to check a value in a nvram variable is exist or not.\n");
	strcat(help_msg, "\t-o : to replace delimiters in a token list by new delimiters.\n");
	strcat(help_msg, "\t-n : to get digits in a nvram variable by speficied counts.\n");
	strcat(help_msg, "\t-m : to change a value by spcified position from a nvram variable.\n");
	strcat(help_msg, "\t-i : to return the value which is at the specified position from a nvram variable by delimiter.\n\n");

	if(argc <= 1 || ((isgraph(*argv[1]) || ispunct(*argv[1])) && *argv[1]!='-')){
		fprintf(stderr, "%s", help_msg);
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
				fprintf(stderr, "%s", help_msg);
				exit(0);
				break;
			default:
				fprintf(stderr, "%s", help_msg);
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
			fprintf(stderr, "%s", help_msg);
			exit(0);
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
			fprintf(stderr, "%s", help_msg);
			exit(0);
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
		fprintf(stderr, "%s", help_msg);
		exit(0);
	}

	//StrFree(tmp);
	return 0;
}
