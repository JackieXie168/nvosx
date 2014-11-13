/*
 * nvram.c
 *
 * Created by Jackie Xie on 2011-07-27.
 * Copyright 2011 Jackie Xie. All rights reserved.
 *
 */

#include <nvram.h>
#include <unistd.h> //For execv function

#ifndef TARGET_DEVICE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

int sem_id;					/*semaphore identifier*/
int sem_id_realloc;		/*semaphore identifier*/
void *pointer;				/*pointer to current poisition in share memory*/
void *ptr_start;			/*pointer to start poisition of share memory*/
static char nullstr[1];	/* zero length string */
char shm_flag=1;			/*check to attach share memory */
char realloc_flag=0;		/*prevent re_alloc function causing deadlock*/
char flag_reload_nvram=0;
int *var_start;				/*start address of hash table*/

#define BOUNDARY_4X(x)	(x = (int)(x + 3) & 0xfffffffc)	/* X4 alignment */
#define INIT_BUF_SIZE_LARGE 1638400
#define INIT_BUF_SIZE_SMALL 2048

//static inline void* ckmalloc (int sz)          { return xmalloc(sz);     }
#if __linux__
union semun {
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
typedef union {
#endif
	int val;   /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array;  /* Array for GETALL, SETALL */
#if __linux__
};
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
}semun;
#endif

inline void set_sem(int semid)
{
	union semun sem_union;
	sem_union.val=1;
	if (semctl(semid,0,SETVAL,sem_union)==-1)
		printf ("set sem error\n");
}

inline void sem_up(int semid)
{
	struct sembuf sem_b;

	sem_b.sem_num=0;
	sem_b.sem_op=-1;
	sem_b.sem_flg=SEM_UNDO;
	if (semop(semid,&sem_b,1)==-1)
		printf ("semp p process error\n");
}

inline void sem_down(int semid)
{
	struct sembuf sem_b;

	sem_b.sem_num=0;
	sem_b.sem_op=1;
	sem_b.sem_flg=SEM_UNDO;
	if (semop(semid,&sem_b,1)==-1)
		printf ("semp v process error\n");
}

inline void detach_shm(void)
{
#if defined(darwin) || defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
	union semun sem_union;
	int shmid = shmget((key_t)NVRAMKEY,(size_t)SHARESIZE,IPC_CREAT|IPC_EXCL|0666);
#endif
	/* detach the share memory*/
	if (shmdt(ptr_start)==-1)
		perror ("shmdt failed\n");
	shm_flag=1;

#if 0 //defined(darwin) || defined(__FreeBSD__) || defined(__APPLE__) || defined(MACOSX)
	/* Lastly, delete the shared memory. */
	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		perror ("shmdt1");
	}
	if (semctl(sem_id, 0, IPC_RMID, sem_union) < 0)
	{
		perror("free resources: semid ");
	}
#endif
}

void attach_share_memory()
{
	int shmid;
	/*create share memory*/
	shmid=shmget((key_t)NVRAMKEY,(size_t)SHARESIZE,IPC_CREAT|IPC_EXCL|0666);
	if(shmid==-1)
	{
//		perror("shmget 1");
		if ((shmid=shmget((key_t)NVRAMKEY,(size_t)SHARESIZE,0666))==-1)
			perror ("attach_share_memory failed, shmid is -1");
	}
	pointer=shmat(shmid,(void *)0, 0);
	
	shm_flag=0;
	
	ptr_start=pointer;
	/*set semaphore*/
	if ((sem_id = semget(NVRAMKEY, 1, IPC_CREAT | 0666)) == -1)
	{
		perror("semget error");
	}
	/*set semaphore*/
	if ((sem_id_realloc = semget(NVRAMKEY+1, 1, IPC_CREAT | 0666)) == -1)
	{
		perror("semget error");
	}
	
	nvram_init();
}

void *get_curr_pos()
{
	char *tmp=(char *)ptr_start;
	int offset;
	/*skip MAGIC_ID*/
	//tmp+=6;
	tmp+=8;
	
	offset=atol(tmp);

	return (void *)(ptr_start+offset);
}

void *ckmalloc(size_t size)
{
	char *p,*ptr;
	char *tmp=ptr_start;
	char line[8];
	int i;
	ptr=(char *)get_curr_pos();
	/* add 1 byte for NULL character*/
	p=ptr+size+1;
	
		BOUNDARY_4X(p);
	
	/*if out of share memory then re-allocate share memory*/
	if (p>=tmp+SHARESIZE)
	{
		//printf ("out of memory...realloc memory....\n");
		return ERR_NO_MEM;

		re_alloc();
		ptr=(char *)get_curr_pos();
		p=ptr+size+1;
			BOUNDARY_4X(p);	
		/*if still out of share memory then return NULL*/
		if (p>=tmp+SHARESIZE)
		{
			//printf ("out of memory...can't alloc memory...\n");
			return NULL;
		}
	}
	
	tmp+=(strlen(MAGIC_ID)+1);
	i=atol(tmp);
	i+=(size+1);
		BOUNDARY_4X(i);
	sprintf(line,"%d-",i);
	line[7]=0;
	strcpy(tmp,line);
//	p=(char *)pointer;
	pointer=(void *)p;

	return (void *)ptr;
}

inline void *get_addr(int offset)
{
//	printf ("get_addr :: offset %x\n",offset);
	char *ptr;
	ptr=(char *)ptr_start;
	if (offset > (SHARESIZE-1)||offset<=0) {
		return NULL;
	}	
	return (void *)(ptr+offset);
}

inline int get_offset(void *ptr)
{
	int i;
	char *tmp1,*tmp2;
	tmp1=(char *)ptr;
	tmp2=(char *)ptr_start;
	i=tmp1-tmp2;
//	printf ("get_offset :: %x\n",i);
	if ((i>0)&&(i<SHARESIZE))
		return i;
	else
		return 0;
}

void
clear_end(char *value)
{
	while(*value)
	{
		if((*value=='\r')||(*value=='\n')||(*value=='\0'))
		{
			*value='\0';
			break;
		}
		value++;
	}
	return;
}

void
hashvar(const char *p,int **ptr)
{
	unsigned int hashval;

	hashval = ((unsigned char) *p) << 4;
	while (*p && *p != '=')
		hashval += (unsigned char) *p++;
	*ptr=&var_start[hashval % VTABSIZE];
	//printf ("hashvar :: ptr[%x]\n",(int)(*ptr));
	return;
}

static int
varequal(const char *p, const char *q)
{
	if (p==NULL)
		return 0;
	while (*p == *q++) {
		if (*p++ == '=')
			return 1;
	}
	if (*p == '=' && *(q - 1) == '\0')
		return 1;
	return 0;
}

static struct varinit *
findvar(struct varinit *vp, const char *name)
{
	for (; vp; vp = (struct varinit *)get_addr((vp->next_offset))) {
		if (varequal((char *)get_addr(vp->name_offset), name)) {
			break;
		}
	}
	return vp;
}

static char tmpbuf[17];
char* itoa (int n)
{
  int i=0,j;
  char s[17];
  
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

int exists(char *FileName)
{
    FILE *file_p;

    if(file_p = fopen(FileName, "r"))
    {
		fclose(file_p);
		printf("\nFile does exist !!!\n");
		return TRUE;
    }
	printf("\nFile doesn't exist !!!\n");
    return FALSE;
}

int nvram_import(char *FileName)
{
	char *value,*name;
	char line[409600];
    FILE *fp_ptr;

	if(exists(FileName)){
		fp_ptr=fopen(FileName,"r+");
		while(fgets(line,sizeof(line),fp_ptr))
		{
			value=line;
			name=line;
			strsep(&value,"=");
			if (value)
			{
				clear_end(value);
				if(name)
				{
					nvram_set(name,value);
					printf("%s = %s\n", name, value);
				}
			}
		}
		fclose(fp_ptr);
		nvram_commit();
		return 0;
	}
	else{
		printf("\nFile \"%s\" does not exist !!!\n", FileName);
		return -1;
	}
}

/*
	nvram log meaning - To record nvram operation status on nvram.log file
	value = 0	 - do not need commit again.
	value = 1 	- need commit when value was changed in memory.
	value = 2 	- delete nvram.conf when do nvram clean.
	vaule = 4	- update nvram.conf when do nvram commit after some value was changed.
*/
int get_nvram_log()
{
	FILE *file_p;
	char tmp[64]="\0";
	int need_commit;
	
	if(file_p = fopen(LOG_FILE_PATH,"r"))
	{
	   fgets(tmp, sizeof(tmp), file_p);
	   //printf("\ntmp=%s\n", tmp);
	   need_commit = atoi(tmp);
	   //printf("\nget_commit() :  need_commit=%d\n", need_commit);
	   fclose(file_p);
	}
	return need_commit;
}

int set_nvram_log(int status)
{
	FILE *file_p;
	int need_commit;

	//INTOFF;
	if(file_p= fopen(LOG_FILE_PATH,"w"))
	{
		fseek(file_p,0,SEEK_END);
		need_commit = status;
		fputs(itoa(need_commit), file_p);
		//printf("\nset_commit(%d) : need_commit=%d\n", need_commit, need_commit);
		fclose(file_p);
		return 0;
	}
	else
		return ERR_NO_MEM;
	//INTON;
}

void nvram_accessfile()
{
	char *value,*name;
	char line[2048];
	FILE *fp_ptr=NULL;
	
	if(!exists(TMP_FILE_PATH)) {
		fp_ptr = fopen(DEFAULT_FILE_PATH,"a+");
		close(fp_ptr);
	}

	/*open NVRAM file*/
	if(get_nvram_log() == 1 || get_nvram_log() == 4){
		fp_ptr=fopen(TMP_FILE_PATH,"w+");
		printf("\nUpdate nvram.conf with a \"w+\" access mode ...\n");
	}
	else{
		fp_ptr=fopen(TMP_FILE_PATH,"a+");
		printf("\nUpdate nvram.conf with a \"a+\" access mode ...\n");
	}
	
	if (fp_ptr==NULL)
	{
		printf ("open %s error..\n",TMP_FILE_PATH);
	}
	else
	{
		printf ("\n\nNVRAM : R/W %s  file !!!\n", TMP_FILE_PATH);
		while(fgets(line,sizeof(line),fp_ptr))
		{
			value=line;
			name=line;
			strsep(&value,"=");
			if (value)
			{
				clear_end(value);
				if(name)
				{
					nvram_set(name,value);
					printf("%s%s\n", name,value);
				}
			}
		}
	}
	fclose(fp_ptr);
	//rename(TMP_FILE_PATH, BACKUP_FILE_PATH);
}

char *
nvram_free()
{
	char *ptr=NULL, *p=NULL;
	unsigned long total, remain, used;
	char tmpbuf1[64]={'\0'};
	char tmpbuf2[128]={'\0'};

	if (shm_flag)
		attach_share_memory();
	ptr=(char *)get_curr_pos();

	/*if out of share memory then re-allocate share memory*/
	if(!ptr_start || !ptr){
		printf ("memory doesn't get...\n");
	}else if (ptr>=(ptr_start+SHARESIZE)) {
		printf ("out of memory...realloc memory....\n");
	}else{
		total = SHARESIZE;
		used = (unsigned long)(ptr - (char *)ptr_start);
		remain = SHARESIZE - used;
		p = tmpbuf2;

		if(total/1000){
			sprintf(tmpbuf1, "Total=%lu.%03d KB, ", total/1000, total%1000);
		}else
			sprintf(tmpbuf1, "Total=%lu B, ", total);
		strcat(p,tmpbuf1);
		p+=strlen(tmpbuf1);
		if(used > 1000){
			sprintf(tmpbuf1, "Used=%lu.%03d KB, ", used/1000, used%1000);
		}else
			sprintf(tmpbuf1, "Used=%lu B, ", used);
		strcat(p,tmpbuf1);
		p+=strlen(tmpbuf1);
		if(remain>1000){
			sprintf(tmpbuf1, "Remain=%lu.%03d KB", remain/1000, remain%1000);
		}else
			sprintf(tmpbuf1, "Remain=%lu B", remain);
		strcat(p,tmpbuf1);
		//p+=strlen(tmpbuf1);
			
		printf("%s\n", tmpbuf2);
	}
	return NULL;
}

char *
nvram_get(name)
	const char *name;
{
	int *offset;
	struct varinit *v,*vp;
	
	if (shm_flag)
		attach_share_memory();
	INTOFF;
	hashvar(name,&offset);
	vp=(struct varinit*)get_addr(*offset);
	if ((v = findvar(vp, name))) {
		if (v->validated == 1) {
			INTON;
			//printf("nvram_get found %s :: %s\n",name,(char *)get_addr(v->text_offset));
			return (char *)get_addr(v->text_offset);
		}
	}
	//printf ("nvram_get found no var\n");
	INTON;
	return return_null(NULL);
}

//extern char flag_reload_nvram;
int nvram_reload( )
{
	flag_reload_nvram=1;
	attach_share_memory();
}

int
nvram_set(name, val)
	const char *name, *val;
{
//	const char *p;
	int len,*ptr;
	int namelen;
	char *line,*nameeq;
	int name_offset,value_offset;
	struct varinit *vp, *vpp;
	char *action_ptr;	

	if (shm_flag)
		attach_share_memory();
	
//	p = name;
//	p=strchr(p,'\0');
	namelen = strlen(name);
	if (val)
		len  = strlen(val);
	else
		len=0;

	action_ptr = NULL;
#ifdef TARGET_DEVICE
	if (strcmp(name, "action") == 0) {		/* action queue */
		action_ptr=nvram_get("action");
		if (action_ptr) {
			len += strlen(action_ptr);
			len += 2;
		}	
	}		
#endif

	line=malloc(namelen+len+2);

	if (line==NULL)
	{
		printf("nvram_set :: malloc error...\n");
		return ERR_NO_MEM;
	}

	if (action_ptr)
		sprintf(line,"%s=%s %s",name, action_ptr, val);
	else	
		sprintf(line,"%s=%s",name,val);
		
	if (!realloc_flag)
		INTOFF;
	hashvar(line,&ptr);
	vpp=(struct varinit*)get_addr(*ptr);
	vp = findvar(vpp, line);

	if (vp)
	{
//		cprintf ("nvram_set found var :: set %s=%s\n",name,val);
		char *tmp=(char *)get_addr(vp->text_offset);
		/*if the length of new value is larger than old one, it will allocate a new memory area*/
		if ( (strlen(tmp)>=len) || (vp->len >= len) )
		{
			if (len)
			{
				memcpy(tmp , val, len);
				tmp[len]='\0';
			}
//			else
				tmp[len]='\0';
		}
		else
		{
			nameeq = ckmalloc(len);
			if (nameeq == ERR_NO_MEM) {
				if (!realloc_flag)
					INTON;
				return ERR_NO_MEM;
			}

			if (nameeq==NULL)
			{
				if (!realloc_flag)
					INTON;
				return ERR_NO_MEM;
			}
			value_offset=get_offset(nameeq);

			if (len) 
				memcpy(nameeq , val, len);
//			else 
				nameeq[len] = '\0';
			vp->text_offset = value_offset;
			vp->len = len;			
		}
		vp->validated = 1;
	}
	else
	{
		unsigned short value_len = len;
		/*allocate memory for value and get the offset*/
		nameeq = ckmalloc(len);
		if (nameeq == ERR_NO_MEM) {
			if (!realloc_flag)
				INTON;
			return ERR_NO_MEM;
		}
		if (nameeq==NULL)
		{
			if (!realloc_flag)
				INTON;
			return ERR_NO_MEM;
		}
		value_offset=get_offset(nameeq);
		if (len) 
			memcpy(nameeq , val, len);
//		else 
			nameeq[len] = '\0';

		/*allocate memory for name and get the offset*/
		len = namelen + 2;              /* 2 is space for '=' and '\0' */
		nameeq=ckmalloc(len);
		if (nameeq == ERR_NO_MEM) {
			if (!realloc_flag)
				INTON;
			return ERR_NO_MEM;
		}
		if (nameeq==NULL)
		{
			if (!realloc_flag)
				INTON;
			return ERR_NO_MEM;
		}
		name_offset=get_offset(nameeq);
		memcpy(nameeq, name, namelen);
		nameeq[namelen] = '=';

		/*allocate memory for varinit and get the offset*/
		vp = ckmalloc(sizeof (*vp));
			if (vp == ERR_NO_MEM) {
				if (!realloc_flag)
					INTON;
				return ERR_NO_MEM;
			}
		if (vp==NULL)
		{
			if (!realloc_flag)
				INTON;
			return ERR_NO_MEM;
		}
		vp->name_offset = name_offset;
		vp->text_offset = value_offset;
		vp->next_offset = get_offset(vpp);
		vp->len = value_len;
		vp->validated = 1;
		
		/*record current varinit offset to the entry of hash table*/
		*ptr = get_offset(vp);
		//printf ("nvram_set found no var :: set %s=%s\n",(char *)get_addr(name_offset),
		//	(char *)get_addr(value_offset));
	}
	if (!realloc_flag)
		INTON;
	free(line);

#ifdef TARGET_DEVICE
	//send a signal to update nvram in monitor.sh immediately
	if (strcmp(name, "action") == 0) {
		//system("killall -SIGUSR1 sleep.sh");
		system("killall -9 alarm");
	}
#endif
	set_nvram_log(1);
	
	return 0;

}

//NOTE!!! the caller function is responsible to free the returned ptr.
char *
nvram_malloc_get(name)
	const char *name;
{

	char *pNvramVal;
	char *retPtr;
	int len;

	if( (pNvramVal = nvram_get(name)) == NULL )
		return NULL;
	len = strlen(pNvramVal);
	if( (retPtr = malloc(len+1)) == NULL )
	{
		printf("##nvram_malloc_get:: malloc failed !!!\n");
		return NULL;
	}
	strncpy(retPtr, pNvramVal, len);
	return retPtr;
}

int
nvram_unset(const char *s)
{
	/*pointer to current varinit struct*/
	struct varinit *vp;
	/*pointer to previous varinit struct*/
	struct varinit *vpp;
	int *ptr;

	if (shm_flag)
		attach_share_memory();

	INTOFF;

	hashvar(s,&ptr);
	vp=(struct varinit *)get_addr(*ptr);
	
	if (vp == NULL)
		goto exit;
	
	if (varequal((char *)get_addr(vp->name_offset), s))
	{
		//*ptr=vp->next_offset;
	}
	else
	{
		for (vpp=vp; vp; vpp=vp,vp = (struct varinit *)get_addr((vp->next_offset))) {
			if (varequal((char *)get_addr(vp->name_offset), s)) {
				break;
			}
		}

		if (vp)
		{
			//vpp->next_offset=vp->next_offset;
		}
	}
	if (vp) 
		vp->validated = 0;
exit:	
	INTON;

	return (0);
}

extern void
nvram_show()
{
 
	struct varinit *vp;
	int i=0;

	if (shm_flag)
		attach_share_memory();
	
	INTOFF;
	//printf ("call nvram_show\n");
	for ( i=0; i < VTABSIZE ; i++)
	{
		vp = (struct varinit *)get_addr(var_start[i]);

		for ( ;vp ; vp = (struct varinit*)get_addr(vp->next_offset))
		{
			char *name,*value;
			int len;
			
			//printf("vp->name_offset = %08x\n", vp->name_offset);
			//printf("vp->text_offset = %08x\n", vp->text_offset);	
					
			if (vp->validated == 1) {
				name=(char *)get_addr(vp->name_offset);
				value=(char *)get_addr(vp->text_offset);
				len = strlen(value);
				printf("%s%s\n", name,value);
			}
		}
	}
	INTON;

}

int
nvram_unsetall(void)
{
	nvram_clean();
}

int
xnvram_reload(void)
{
	int fd, fd_tmp, len, i;
	char *file_path = TMP_FILE_PATH;
	unsigned char buf[2];
	char *conf;

	printf("read nvram cfg \n");
	if(chdir(_PATH_CONFIG) < 0) {
		fprintf(stderr, "nvram reload: can not change dir to %s\n", _PATH_CONFIG);
		return 0;
	}

	if((fd = open(_PATH_CONFIG_MTD, O_RDONLY | O_SYNC)) < 0) {
		fprintf(stderr, "can not open %s\n", _PATH_CONFIG_MTD);
		return 0;
	}

	if(lseek(fd,65533,SEEK_SET) < 0) {
		fprintf(stderr,"can not set offset=65533 to %s\n",_PATH_CONFIG_MTD);
		close(fd);
		return 0;
	}

	len = read(fd, buf, 2);
	if(len != 2) {
		fprintf(stderr, "can not get config file length, read %d\n",len);
		close(fd);
		return 0;
	}

	len = (int)buf[0]*256 + (int)buf[1];

	if(len <= 0 || len >= 65536) {
		fprintf(stderr,"there is no config value in %s\n",_PATH_CONFIG_MTD);
		return 0;
	}

	if(lseek(fd, 0, SEEK_SET) < 0) {
		fprintf(stderr,"can not set offset=0 to %s\n",_PATH_CONFIG_MTD);
		return 0;
	}

	unlink(file_path);
	if((fd_tmp = open(file_path, O_RDWR | O_CREAT | O_TRUNC, 0755)) < 0) {
		fprintf(stderr, "can not creat %s\n", file_path);
		return 0;
	}

	if(!(conf = (char *)malloc(len))){
		fprintf(stderr, "nvram reload : can not malloc space for %s\n", file_path);
		close(fd);
		close(fd_tmp);
		unlink(file_path);
		return 0;
	}

	if((i = read(fd, conf, len)) != len) {
		fprintf(stderr, "nvram reload: can not get config file\n");
		close(fd);
		close(fd_tmp);
		unlink(file_path);
		return 0;
	}

	close(fd);
	if((i = write(fd_tmp, conf, len)) != len) {
		fprintf(stderr, "nvram reload: can not set config file\n");
		close(fd_tmp);
		unlink(file_path);
		return 0;
	}

	free(conf);
	close(fd_tmp);

	return 1;
}

int nvram_backup(char *ofile)
{
	struct varinit *vp;
	int i;
	const char *sep = nullstr;
	FILE *fp=NULL;
	char *argv[]={ "backup",NULL,NULL,(char *)0};
	
	if (shm_flag)
		attach_share_memory();

#ifndef TARGET_DEVICE
	mkdir_r(CONF_PATH);
	printf("\nnvram_backup() : %d\n", get_nvram_log());
	printf("Configuration is backup !\n");
	if(get_nvram_log() >= 1){
		nvram_accessfile();
		set_nvram_log(0);
		printf ("\n\nNVRAM : Configures have backup to %s !!!\n", ofile);
	}
#endif

	fp=fopen(ofile, "w+");
	if (fp==NULL)
	{
		printf ("open %s error..\n",ofile);
		return -1;
	}
	INTOFF;
	
	for ( i=0; i < VTABSIZE ; i++) {
		vp = (struct varinit *)get_addr(var_start[i]);
		for (; vp ; vp = (struct varinit *)get_addr(vp->next_offset)) {
			char *name,*value;
			name=(char *)get_addr(vp->name_offset);
#if defined(NVRAM_GOT_FROM_HW)
				kk = 0;
				while (nvram_by_pass_var[kk] != NULL) {
					if( !strcmp(name, nvram_by_pass_var[kk]) ) {
						by_pass=1;
						break;
					}
					kk++;
				}

				if(by_pass)	{
					by_pass = 0;
					continue;
				}
#endif
			value=(char *)get_addr(vp->text_offset);
			fprintf(fp,"%s%s%s\n", sep, name, value);
		}
	}
	INTON;
	fclose(fp);
	{
		FILE *fp;
		
		fp = fopen("/tmp/commit", "r");
		if (fp)
			system("/tmp/commit");
		else	
			system("/usr/sbin/commit");
	}
	return 0;
}

int nvram_restore(char *ifile)
{
	return _nvram_reload(ifile);
}

//#define NVRAM_GOT_FROM_HW	1
#if defined(NVRAM_GOT_FROM_HW)
//added by dvd.chen to filter out HW related nvram vars
#define NVRAM_BY_PASS_NUM 10
static char *nvram_by_pass_var[]=
{
	"lan_hwaddr=",
	"wan_hwaddr=",
	"wan0_hwaddr=",
	"wan_factory_mac=",
	"hwaddr_2=",
	"hwaddr_3=",
	"hwaddr_4=",
	"hwaddr_5=",
	"wps_pin=",
	"fw_version=",
	NULL
};
#endif

int
nvram_commit()
{
	struct varinit *vp;
	int i;
	const char *sep = nullstr;
	FILE *fp=NULL;
	char *argv[]={ "commit",NULL,NULL,(char *)0};
	//char *argv[]={ "ls","-al","/etc",(char *)0};
	
	INTOFF;
	if (shm_flag)
		attach_share_memory();

	//fp=fopen(DEVICE_PATH,"w+");
#ifndef TARGET_DEVICE
	//printf ("\n\nNVRAM : Create configuration path : %s !!!\n", CONF_PATH);
	mkdir_r(CONF_PATH);
	printf("nvram_commit() : commit=%d\n", get_nvram_log());
	printf("Configuration is commited !\n");
	if(get_nvram_log() >= 1){
		nvram_accessfile();
		set_nvram_log(0);
		printf ("\n\nNVRAM : Configures have commited to %s  !!!\n", TMP_FILE_PATH);
	}
#if 0
	char tmp="";
	int fd = open(TMP_FILE_PATH, O_CREAT|O_EXCL|O_WRONLY|O_TRUNC, S_IWOTH|S_IROTH);
	if(fd==-1)
		printf("\nAn error has occurred\n");
	write(fd,tmp,sizeof(tmp));
	close(fd);
#endif	
#endif

//#ifndef TARGET_DEVICE
//	if(!exists(TMP_FILE_PATH))
//#else
	fp=fopen(TMP_FILE_PATH, "w+");
	if (fp==NULL)
//#endif
	{
//#ifndef TARGET_DEVICE
//		fp = fopen(TMP_FILE_PATH, "w+");
//		printf("\nCreate the file : %s\n\n", TMP_FILE_PATH);
//#else
		printf ("open %s error..\n",TMP_FILE_PATH);
		return -1;
//#endif
	}

#if 0
	fp=fopen(TMP_FILE_PATH, "w+");
	if (fp==NULL)
	{
		printf ("file open error\n");
		INTON;
		return -1;
	}
#endif

	//INTOFF;

	for ( i=0; i < VTABSIZE ; i++) {
		vp = (struct varinit *)get_addr(var_start[i]);
		for (; vp ; vp = (struct varinit *)get_addr(vp->next_offset)) {
			char *name,*value;
			if (vp->validated == 1) {
				name=(char *)get_addr(vp->name_offset);
#if defined(NVRAM_GOT_FROM_HW)
					// To filter out HW related nvram vars
					//printf("%s\n", name);
					kk = 0;
					while (nvram_by_pass_var[kk] != NULL) {
						if( !strcmp(name, nvram_by_pass_var[kk]) ) {
							by_pass=1;
							break;
						}
						kk++;
					}

					if(by_pass)	{
						//printf("nvram_commit:: by_pass[%s]\n", name);
						by_pass = 0;
						continue;
					}
#endif
				value=(char *)get_addr(vp->text_offset);
				fprintf(fp,"%s%s%s\n", sep, name, value);
			}
		}
	}
	INTON;
	fclose(fp);
	return 0;
}

int nvram_getall(char *buf, int count)
{
	int i,len=0;
	char *line;
	struct varinit *vp;
	
	if (count==0)
		return 0;
	
	if (shm_flag)
		attach_share_memory();
	if (!realloc_flag)
		INTOFF;
	for (i=0; i < VTABSIZE ; i++)
	{
		vp = (struct varinit *)get_addr(var_start[i]);
		for (; vp ; vp = (struct varinit *)get_addr(vp->next_offset)) {
			char *name,*value;
			if (vp->validated == 1) {
				name=(char *)get_addr(vp->name_offset);
				if (strlen(name) == 0) {
					printf("%s %d\n", __FUNCTION__, __LINE__);
					continue;
				}
				value=(char *)get_addr(vp->text_offset);
					line=malloc(strlen(name)+strlen(value)+2);
				if (line==NULL)
					goto END;
				sprintf(line,"%s%s", name, value);
				strcpy(buf,line);
				buf += strlen(line);
				*buf = '\0';
				buf++;
				len+=(strlen(line)+1);
				free(line);
			}
		}
	}
END:
	if (!realloc_flag)
		INTON;
	return len;
}

int nvram_setall(char *buf, int count)
{
	char *name, *value, *end, *eq;

	name = buf;
	end = buf + count - 2;
	end[0] = end[1] = '\0';

        for (; *name; name = value + strlen(value) + 1) {
              if ((eq = strchr(name, '=')) == NULL)
                       break;

              *eq = '\0';
              value = eq + 1;
              nvram_set(name, value);
              *eq = '=';
        }
	return 0;
}

int _nvram_reload(char *file)
{
	char *value,*name;
	char *line;
	int len = INIT_BUF_SIZE_LARGE;
	FILE *fp=NULL;

	printf("Read nvram config file : %s ...\n", file);
	//fp=fopen(DEFAULT_FILE_PATH,"r+");
	fp=fopen(file,"r");
	if (fp==NULL) {
			printf ("open %s error..\n",file);
	}
	else {
		printf("Restore nvram config file : %s ...\n", file);
		line = malloc(INIT_BUF_SIZE_LARGE);
		// if memory size is not available,  we may try to allocate smaller size
		if (line == NULL)
		{
			line = malloc(INIT_BUF_SIZE_SMALL);
			len = INIT_BUF_SIZE_SMALL;
		}
		while(fgets(line,len,fp)) {
			value=line;
			name=line;
			strsep(&value,"=");
			if (value) {
				clear_end(value);
				nvram_set(name,value);
			}
		}
		fclose(fp);
		free(line);
	}
	printf("Restore %s done ...\n", file);

	return 0;
}

/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */
int
nvram_match(char *name, char *match) {
	const char *value = BCMINIT(nvram_get)(name);
	return (value && !strcmp(value, match));
}

/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */
int
nvram_invmatch(char *name, char *invmatch) {
	const char *value = BCMINIT(nvram_get)(name);
	if (value == 0) return 1;
	return (value && strcmp(value, invmatch));
}

void dump_mem(void *p,int len)
{
	int i;
	char *ptr;
	ptr=(char *)p;
	for (i=0;i<len;i++)
		printf ("%c",ptr[i]);
	printf ("\n");
}

void init_share_ptr()
{
	char *p=(char *)ptr_start,*ptr;
	int i;

	/*add 1 for NULL character*/
	p+=(strlen(MAGIC_ID)+1);
	ptr=p;
	for (;;ptr++)
		if ((*ptr>='0')&&(*ptr<='9'))
			break;
	/*get current offset*/
	i=atol(ptr);
	ptr=(char *)ptr_start;
	pointer=ptr+i;

//	printf ("get current offset :: %d\n",i);
	p+=8; // 8 bytes for restoring current offset for pointer

	/*hash table is allocated after MAGIC_ID and current offset (8 bytes)*/
	var_start=(int*)p;
	
}

int
nvram_init()
{
	char *value,*name,*p;
//	char line[2048];
	char *line;
	int len = INIT_BUF_SIZE_LARGE;
//	int varid;
	FILE *fp_ptr=NULL;

	//printf ("pointer :: %x\n",(int) pointer);
	//printf("nvram_init\n");
#ifndef TARGET_DEVICE
	if (flag_reload_nvram || strncmp(pointer,MAGIC_ID,strlen(MAGIC_ID))!=0)
#else
	if (strncmp(pointer,MAGIC_ID,strlen(MAGIC_ID))!=0)
#endif
	{
		line = malloc(INIT_BUF_SIZE_LARGE);
		// if memory size is not available,  we may try to allocate smaller size
		if (line == NULL)
		{
			line = malloc(INIT_BUF_SIZE_SMALL);
			len = INIT_BUF_SIZE_SMALL;
		}
		//printf("start new process...\n");
		memset(pointer,0,SHARESIZE);
		set_sem(sem_id);
		set_sem(sem_id_realloc);
		//printf ("set semp ok...\n");
		INTOFF;
		p=ckmalloc(strlen(MAGIC_ID));
		strcpy(p,MAGIC_ID);
		//8 bytes for restoring current offset of pointer
		//ckmalloc will add 1 byte
		ckmalloc(7);
		var_start=ckmalloc(sizeof(int)*VTABSIZE);
		INTON;
//		if (varid==-1)
//			printf ("create share memory(var) error\n");
		/* to default */
		/*open NVRAM_DEFAULT file*/
		fp_ptr=fopen(DEFAULT_FILE_PATH,"r");
		if (fp_ptr==NULL)
		{
			printf ("open %s error..\n",DEFAULT_FILE_PATH);
		}
		else
		{
			while(fgets(line,len,fp_ptr))
			{
				value=line;
				name=line;
				strsep(&value,"=");
				if (value)
				{
					clear_end(value);
					nvram_set(name,value);
				}
			}
			fclose(fp_ptr);
		}
		
		/*open NVRAM file*/
//#ifndef TARGET_DEVICE
//		if(!exists(TMP_FILE_PATH))
//#else
		fp_ptr=fopen(TMP_FILE_PATH,"r+");
		if (fp_ptr==NULL)
//#endif
		{
//#ifndef TARGET_DEVICE
//			fp_ptr = fopen(TMP_FILE_PATH,"w");
//			printf("\nCreate the file : %s\n\n", TMP_FILE_PATH);
//#else
			printf ("open %s error..\n",TMP_FILE_PATH);
			fp_ptr = fopen(TMP_FILE_PATH,"w+");
//#endif
		}
		else
		{
			while(fgets(line,len,fp_ptr))
			{
				value=line;
				name=line;
				strsep(&value,"=");
				if (value)
				{
					clear_end(value);
					nvram_set(name,value);
				}
			}
			fclose(fp_ptr);
		}
		free(line);
	}
	else
	{
		//printf ("found MAGIC ID\n");
		init_share_ptr();
	}
	if(!get_nvram_log())
		set_nvram_log(0);
	
	return 0;
}

void
nvram_clean(void)
{
#ifndef TARGET_DEVICE
#if 0
	char *value,*name;
	char line[2048];
	FILE *fp_ptr=NULL;
	
	/*open NVRAM file*/
//#ifndef TARGET_DEVICE
//	if(!exists(TMP_FILE_PATH))
//#else
	fp_ptr=fopen(TMP_FILE_PATH,"r+");
	if (fp_ptr==NULL)
//#endif
	{
//#ifndef TARGET_DEVICE
//		fp_ptr = fopen(TMP_FILE_PATH,"w");
//		printf("\nCreate the file : %s\n\n", TMP_FILE_PATH);
//#else
		printf ("open %s error..\n",TMP_FILE_PATH);
//#endif
	}
	else
	{
		while(fgets(line,sizeof(line),fp_ptr))
		{
			value=line;
			name=line;
			strsep(&value,"=");
			if (value)
			{
				clear_end(value);
				nvram_unset(name);
			}
		}
	}
	fclose(fp_ptr);
	rename(TMP_FILE_PATH, BACKUP_FILE_PATH);
	if(get_nvram_log() == 1)
		set_nvram_log(4);
	else
		set_nvram_log(2);
#else
	system("remove_shm.sh");
	//detach_shm();
#endif
#else
	/*erase MAGIC_ID, current pointer offset, and hash table*/
	//int len=(strlen(MAGIC_ID)+1)+8+(sizeof(struct varinit *)*VTABSIZE+1);
	//memset(ptr_start,0,len);
	memset(pointer,0,SHARESIZE);
#endif
}

void nvram_default(void)
{

	char *line,*value,*name;
	char *buf,*ptr;
	int len = INIT_BUF_SIZE_LARGE;
	FILE *fp_ptr=NULL;

	printf("To nvram default ...\n");
	if (shm_flag)
		attach_share_memory();
	//printf ("before realloc...poniter [%x]...\n",(int)pointer);
	INTOFF_REALLOC;
	pointer=ptr_start;

	nvram_clean();
	/*start initial data*/
	ptr=ckmalloc(strlen(MAGIC_ID));
	strcpy(ptr,MAGIC_ID);
	ckmalloc(7);
	var_start=ckmalloc(sizeof(int)*VTABSIZE);

	/*restore variables*/
	line = malloc(INIT_BUF_SIZE_LARGE);
	// if memory size is not available,  we may try to allocate smaller size
	if (line == NULL)
	{
		line = malloc(INIT_BUF_SIZE_SMALL);
		len = INIT_BUF_SIZE_SMALL;
	}

	fp_ptr=fopen(DEFAULT_FILE_PATH,"r");
	if (fp_ptr==NULL)
	{
		printf ("open %s error..\n",DEFAULT_FILE_PATH);
	}
	else
	{
		while(fgets(line,len,fp_ptr))
		{
			value=line;
			name=line;
			strsep(&value,"=");
			if (value)
			{
				clear_end(value);
				nvram_set(name,value);
			}
		}
		fclose(fp_ptr);
	}

#if 0
	/*open NVRAM file*/
	fp_ptr=fopen(TMP_FILE_PATH,"r+");
	if (fp_ptr==NULL)
	{
		printf ("open %s error..\n",TMP_FILE_PATH);
	}
	else
	{
		while(fgets(line,len,fp_ptr))
		{
			value=line;
			name=line;
			strsep(&value,"=");
			if (value)
			{
				clear_end(value);
				nvram_set(name,value);
			}
		}
		fclose(fp_ptr);
	}
#endif

	free(line);
	INTON_REALLOC;
}

void re_alloc(void)
{
	char *line,*value,*name;
	char *buf,*ptr;
	int len,total_len;

	if (shm_flag)
		attach_share_memory();
	//printf ("before realloc...poniter [%x]...\n",(int)pointer);
	buf=malloc(SHARESIZE);
	if (buf==NULL)
	{
		perror("malloc");
		return ;
	}
	/*save current settings to buf*/
	INTOFF_REALLOC;
	realloc_flag=0;
	total_len=nvram_getall(buf,SHARESIZE);
	printf("%s, %d, pos=%x total_len=%d\n", __FUNCTION__, __LINE__, get_curr_pos(), total_len);
	pointer=ptr_start;

	nvram_clean();

	/*start initial data*/
	ptr=ckmalloc(strlen(MAGIC_ID));
	strcpy(ptr,MAGIC_ID);
	ckmalloc(7);
	var_start=ckmalloc(sizeof(int)*VTABSIZE);
	
	ptr=buf;
	/*restore variables*/
	while(total_len > 0)
	{
		len=strlen(ptr);
		line=malloc(len+1);
		if (line==NULL)
		{
			//printf("re_alloc :: malloc error....\n");
			break;
		}
		sprintf(line,"%s",ptr);
//		len=strlen(line);
		if (len==0)
			break;
		value=line;
		name=line;
		strsep(&value,"=");
		if (value)
		{
			clear_end(value);
			//nvram_set(name,value);
			//if (strstr(name, "action") == NULL) {
				if (nvram_set(name,value) == ERR_NO_MEM) {
					free(line);
					break;
				}
			//}
		}
		/*move to next data*/
		len++;
		ptr+=len;
		total_len-=len;
		free(line);
	}
	realloc_flag=0;
	INTON_REALLOC;
	
	free(buf);
	//printf ("after realloc...poniter [%x]...\n",(int)pointer);
}

