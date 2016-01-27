#include "dni_bcmnvram.h"
#include <dni_shutils.h>
#include <fcntl.h>
#include <unistd.h> //For execv function


//#include <linux/compile.h>
// modified by Kane
// because block_KeyWord_DomainList may need almost 16KB (255 * 61 = 15555) to store the key words,
// we should enlarge the buffer size to avoid the settings lost.
#define INIT_BUF_SIZE_LARGE 16384
#define INIT_BUF_SIZE_SMALL 2048

int sem_id;		/*semaphore identifier*/
int sem_id_realloc;		/*semaphore identifier*/
void *pointer;	/*pointer to current poisition in share memory*/
void *ptr_start;	/*pointer to start poisition of share memory*/
static char nullstr[1];         /* zero length string */
char shm_flag=1;	/*check to attach share memory */
char realloc_flag=0;		/*prevent re_alloc function causing deadlock*/

int *var_start;			/*start address of hash table*/

#define BOUNDARY_4X(x)	(x = (int)(x + 3) & 0xfffffffc)	/* X4 alignment */

//static inline void* ckmalloc (int sz)          { return xmalloc(sz);     }
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
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
	/* detach the share memory*/
	if (shmdt(ptr_start)==-1)
		perror ("shmdt1");
	shm_flag=1;
}

// Ares temp hardcode to skip compile sequence problem
//const char *version_string = OS_VERSION " (" LINUX_COMPILE_BY " " __DATE__ " )";
//#define OS_VERSION	"1.00.00"
#define OS_VERSION	"1.0.1"
#define REVISION	"RC0"

const char *version_string = OS_VERSION REVISION;
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
	//nvram_set("os_version", version_string);

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

char pointer_buf[50];

char *
nvram_get(name_org)
	const char *name_org;
{
	int *offset;
	struct varinit *v,*vp;
	char *name;

	if (shm_flag)
		attach_share_memory();
	INTOFF;
	if (strcmp(name_org, "__pointer__") == 0) {		/* to check share memory used offset */
		char *off;
		off=get_curr_pos();
		off=(int)((int)off-(int)ptr_start);
		sprintf(pointer_buf, "offset=%d size=%d", off, SHARESIZE);
		return (char *)pointer_buf;
	}
	else if (strcmp(name_org, "_action_") == 0) {		/* action queue */
		name = "action";
	}
	else
		name = name_org;
	hashvar(name,&offset);
	vp=(struct varinit*)get_addr(*offset);
	if ((v = findvar(vp, name))) {
		if (v->validated == 1) {
			if (strcmp(name_org, "_action_") == 0) {		/* action queue */
				v->validated = 0;
			}
			
			INTON;				
			return (char *)get_addr(v->text_offset);
		}
	}
	//printf ("nvram_get found no var\n");
	INTON;
	return NULL;
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
	char tmp_act[128], *act;

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
	if (strcmp(name, "action") == 0) {		/* action queue */

		action_ptr=nvram_get("action");
		if (action_ptr) {
			if (strcmp(val, "3") == 0) {		/* Only keep action=3 one time */
				strcpy(tmp_act, action_ptr);
				ptr=tmp_act;
				do {
					act=strsep(&ptr, " ");
					if ((act != NULL) && (strcmp(act, "3") == 0))
						return 0;
				} while (ptr);
			}	
			len += strlen(action_ptr);
			len += 2;
		}
	}

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
					if (action_ptr)
						sprintf(tmp,"%s %s",  action_ptr, val);
					else
						memcpy(tmp , val, len);
				    tmp[len]='\0';
			}
			else
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

			if (len){
				if (action_ptr)
					sprintf(nameeq,"%s %s",  action_ptr, val);
				else
					memcpy(nameeq , val, len);
			}
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

	//send a signal to update nvram in monitor.sh immediately
	if (strcmp(name, "action") == 0) {
		//system("killall -SIGUSR1 sleep.sh");
		system("killall -9 alarm");
	}
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

void
nvram_unset_CSRF_all(const char *s,int type)
{
	struct varinit *vp;
	int i=0;

	if (shm_flag)
		attach_share_memory();
	
	char *name_tmp = (char*)malloc(sizeof(char));
	
	
	INTOFF;

	for ( i=0; i < VTABSIZE ; i++)
	{
		vp = (struct varinit *)get_addr(var_start[i]);				
		for ( ;vp ; vp = (struct varinit*)get_addr(vp->next_offset))
		{
			char *name,*unset_type0;
			if (vp->validated == 1) {
				name=(char *)get_addr(vp->name_offset);
				//xxx*
				if(type==1)
				{	
					if (!strncmp(name, s , strlen(s))){
						strcat(name_tmp,name);
					}
				}
				//*xxx
				else
				{
					int stl;
					if((strlen(name)>strlen(s)))
					{
						unset_type0=name+(strlen(name)-strlen(s)-1);
						stl = strlen(unset_type0)-1;
						if (!strncmp(unset_type0 , s , stl)){
							strcat(name_tmp,name);
						}
					}	
				}
			}
		}
	}
	INTON;
	
	char *unset_tmp = strtok(name_tmp,"=");
	while(unset_tmp != NULL)
	{
		nvram_unset(unset_tmp);
		unset_tmp = strtok(NULL,"=");
	}

	free(name_tmp);
	free(unset_tmp);

	return;	
}

#define _PATH_CONFIG_MTD  "/dev/mtd/7"
int
nvram_unsetall(void)
{
        ;
}
int
dni_nvram_reload(void)
{
	int fd, fd_tmp, len, i;
	char *file_path = TMP_FILE_PATH;
	unsigned char buf[2];
	char *conf;

	printf("read dni nvram cfg \n");
	if(chdir("/tmp") < 0) {
		fprintf(stderr, "nvram reload: can not change dir to /tmp\n");
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
				
//end of dvd.chen
#endif

int
nvram_commit()
{
	struct varinit *vp;
	int i,kk;
	const char *sep = nullstr;
	FILE *fp=NULL;
	char *argv[]={ "commit",(char *)0};
	//char *argv[]={ "ls","-al","/etc",(char *)0};
	int by_pass = 0;

		
	INTOFF;
	
	if (shm_flag)
		attach_share_memory();

	//fp=fopen(DEVICE_PATH,"w+");
	//printf("open config file = %s \n", TMP_FILE_PATH);
	fp=fopen(TMP_FILE_PATH, "w+");
	if (fp==NULL)
	{
		printf ("file open error\n");
		INTON;
		return -1;
	}

	//INTOFF;
	//printf ("call nvram commit\n");
	for ( i=0; i < VTABSIZE ; i++) {
		vp = (struct varinit *)get_addr(var_start[i]);
		for (; vp ; vp = (struct varinit *)get_addr(vp->next_offset)) {
			char *name,*value;
			if (vp->validated == 1) {
				name=(char *)get_addr(vp->name_offset);
#if defined(NVRAM_GOT_FROM_HW)
				//added by dvd.chen to filter out HW related nvram vars
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
				//end of dvd.chen
#endif
				value=(char *)get_addr(vp->text_offset);
				fprintf(fp,"%s%s%s\n", sep, name, value);
			}
		}
	}

	//INTON;

	fclose(fp);
/***********For Timo Test Only**********/
/*
 *  This call the prepared shell script to create mtdblock
 */
	//execv("/bin/ls", argv);
	//execv("/etc/nvram/commit", argv);
	#if 0
	{
#if 1//for test
	char cmd_write[128];
	unsigned char checksum;

	//printf("Aaron test %s to %s writed configuration \n",TMP_FILE_PATH, _PATH_CONFIG_MTD);
		//sprintf(cmd_write, "/sbin/mtd write  %s %s > /dev/null 2>&1",
		checksum=_calcsum_(_PATH_CONFIG_MTD);
		sprintf(cmd_write, "echo -n -e '\\'%03o >> %s",
				checksum,TMP_FILE_PATH);
		printf("cmd = %s \n", cmd_write);
        system(cmd_write);

		_calcsum_(TMP_FILE_PATH);
		sprintf(cmd_write, "/sbin/mtd write  %s %s",
				TMP_FILE_PATH, _PATH_CONFIG_MTD);
#else
		char cmd_write[64];
		sprintf(cmd_write, "/sbin/mtd write  %s %s",
				TMP_FILE_PATH, _PATH_CONFIG_MTD);
#endif
		printf("cmd = %s \n", cmd_write);
        system(cmd_write);
	}
	#else
	{
		FILE *fp;
		
		fp = fopen("/tmp/commit", "r");
		if (fp) {
			fclose(fp);
			system("/tmp/commit");
		}
		else	
			system("/usr/sbin/commit");
	}
	INTON;
	#endif
/*****End*****/
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

#if 1
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


int nvram_reload(char *file)
{
	char *value,*name;
	char *line;
	int len = INIT_BUF_SIZE_LARGE;
	FILE *fp=NULL;

	//fp=fopen(DEFAULT_FILE_PATH,"r+");
	fp=fopen(file,"r");
	if (fp==NULL) {
			printf ("open %s error..\n",file);
	}
	else {
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

	return 0;
}

#endif


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
	if (strncmp(pointer,MAGIC_ID,strlen(MAGIC_ID))!=0)
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
		fp_ptr=fopen(REGION_FILE_PATH, "r");
		if (fp_ptr) {
			fclose(fp_ptr);
			fp_ptr=fopen(DEFAULT_FILE_PATH_EU,"r");
		}
		else {
			fp_ptr=fopen(DEFAULT_FILE_PATH,"r");
		}
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
		free(line);
	}
	else
	{
		//printf ("found MAGIC ID\n");
		init_share_ptr();
	}

	return 0;
}

void
nvram_clean(void)
{
	/*erase MAGIC_ID, current pointer offset, and hash table*/
	//int len=(strlen(MAGIC_ID)+1)+8+(sizeof(struct varinit *)*VTABSIZE+1);
	//memset(ptr_start,0,len);
	memset(pointer,0,SHARESIZE);

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
        /*printf ("before realloc...poniter [%x]...\n",(int)pointer);*/
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
        /* if memory size is not available,  we may try to allocate smaller size */
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
		if (len==0) {
			break;
		}
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

