/*
	gcc -o test test.c -L../.libs -ldniutils -I../include
*/
#include <nvram.h>

int main(void)
{
	{
		int len;
		char *name, *value;
		value = nvram_getall();	
		for (name = value; *name; name += strlen(name) + 1){
			len= strlen(name);
			if (len && (name[len-1] == '\r')) len--;	
			printf("%.*s\n", len, name);
		}
	}
	printf("\n");
	nvram_getall();
	nvram_set("Asia/Phnom Penh", "ICT-7");
	nvram_set("Asia/Kuala Lumpur", "MYT-8");
	nvram_set("hello", "123");
#if 1 
	if ( nvram_match("hello","123") )
		printf("hello is 123\n");
	else
		printf("hello is not 123\n");
#endif

	printf("nvram get \"Asia/Phnom Penh\" = %s\n", nvram_get("Asia/Phnom Penh"));
	printf("nvram get \"Asia/Kuala Lumpur\" = %s\n", nvram_get("Asia/Kuala Lumpur"));
	return 0;
}
