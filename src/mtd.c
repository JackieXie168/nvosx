#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <nvram.h>

#if __linux__
#include <sys/ioctl.h>
//#define TARGET_DEVICE
#endif

#ifdef TARGET_DEVICE
struct mtd_info_user
{
	uint8	type;
	uint32	flags;
	uint32	size;			/* Total size of the MTD */
	uint32	erasesize;
	uint32	oobblock;	/* Size of OOB blocks (e.g. 512) */
	uint32	oobsize;		/* Amount of OOB data per block (e.g. 16) */
	uint32	ecctype;
	uint32	eccsize;
};

struct erase_info_user 
{
	uint32	start;
	uint32	length;
};

#define MEMGETINFO 	_IOR('M', 1, struct mtd_info_user)
#define MEMERASE		_IOW('M', 2, struct erase_info_user)
#else
#define NVRAM_MTD_FILE
#endif

/* the mtd is the full path name */
int mtd_write(const char *mtd, char *buf, int len)
{
#ifdef NVRAM_MTD_FILE
	FILE *fp;

	fp = fopen(mtd, "wb");
	if (fp == NULL)
		return -1;

	fwrite(buf, 1, len, fp);
	fclose(fp);
	
	return 0;
#else

	int fd, ret = -1;
	size_t r, w, e, offset;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdEraseInfo;
	
	if ((fd = open(mtd, O_RDWR |O_SYNC)) < 0)
		return ret;

	if (ioctl(fd, MEMGETINFO, &mtdInfo))
		goto fin;
		
	offset = w = e = 0;

	for (; len > 0; len -= mtdInfo.erasesize) {
		r = (mtdInfo.erasesize < len ? mtdInfo.erasesize : len);
		w += r;
		
		while (w > e) {
			mtdEraseInfo.start = e;
			mtdEraseInfo.length = mtdInfo.erasesize;

			if (ioctl(fd, MEMERASE, &mtdEraseInfo) < 0)
				goto fin;
			
			e += mtdInfo.erasesize;
		}

		if (write(fd, &buf[offset], r) != r)
			goto fin;

		offset += r;
	}

	ret = 0;
fin:
	close(fd);
	return ret;
#endif
}

int mtd_read(const char *mtd, char *buf, int len)
{
#ifdef NVRAM_MTD_FILE
	int rlen;
	FILE *fp;
	
	//fp = fopen(mtd, "rb");
	fp = fopen(mtd, "r+");
	if (fp == NULL)		
		return -1;;

	rlen = fread(buf, 1, len, fp);
	fclose(fp);

	return rlen;
#else
	int fd, rlen;
		
	if ((fd = open(mtd, O_RDONLY | O_SYNC)) < 0)
		return -1;

	rlen = read(fd, buf, len);
	close(fd);

	return rlen;
#endif
}
