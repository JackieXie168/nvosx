/*
	gcc -o test test.c -L../.libs -linvram -I../include
*/
#include <invram.h>

int main(void)
{
	invram_getall();
	invram_set("Asia/Phnom Penh", "ICT-7");
	invram_set("Asia/Kuala Lumpur", "MYT-8");
#if 1 
	if ( invram_match("hello","123") )
		printf("hello is 123\n");
	else
		printf("hello is not 123\n");
#endif

	invram_get("Asia/Phnom Penh");
	invram_get("Asia/Kuala Lumpur");
	return 0;
}

