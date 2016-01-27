//#include "dni_bcmnvram.h"
//#include "dni_shutils.h"

int main(void)
{
	nvram_show();

#if 1 
	if ( nvram_match("hello","123") )
		printf("hello is 123\n");
	else
		printf("hello is not 123\n");
#endif
	return 0;
}

