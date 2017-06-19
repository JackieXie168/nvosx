/*
	gcc -o test test.c -L../.libs -lnvram -I../include
*/
#include <nvram.h>
#define config_get(name)  ((name != NULL) ? (nvram_get(name) != NULL ? nvram_get(name) : "") : "")
#define cfg_get(name)	(name ? (nvram_get(name) ? : "") : "")

int main(void)
{
	nvram_show();
	printf("\n\n\n");
	nvram_set("Asia/Phnom Penh", "ICT-7");
	nvram_set("Asia/Kuala Lumpur", "MYT-8");
#if 1 
	if ( nvram_match("hello","123") )
		printf("hello is 123\n");
	else
		printf("hello is not 123\n");
#endif

	printf("nvram_get(\"Asia/Phnom Penh\") is %s\n", nvram_get("Asia/Phnom Penh"));
	printf("nvram_get(\"Asia/Kuala Lumpur\" is %s\n", nvram_get("Asia/Kuala Lumpur"));
	printf("config_get(NULL) is %s\n", config_get(NULL));
	printf("config_get(\"Asia/Taipei\") is %s\n", config_get("Asia/Taipei"));
	printf("cfg_get(\"Asia/Beijing\") is %s\n", cfg_get("Asia/Beijing"));
	printf("cfg_get(\"\") is %s\n", cfg_get(""));

	return 0;
}
