/*
	gcc -o test test.c -L../.libs -lnvram -I../include
*/
#include <nvram.h>

int main(void)
{
	nvram_show();
	nvram_set("Asia/Phnom Penh", "ICT-7");
	nvram_set("Asia/Kuala Lumpur", "MYT-8");
#if 1 
	if ( nvram_match("hello","123") )
		printf("hello is 123\n");
	else
		printf("hello is not 123\n");
#endif

	nvram_get("Asia/Phnom Penh");
	nvram_get("Asia/Kuala Lumpur");
	return 0;
}

