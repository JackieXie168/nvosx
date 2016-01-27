#include <dni_shutils.h>
#include <dni_wan_info.h>
#include <dni_ioctl.h>
//#include "vdslwebapi.h"


/* ======================(function header)========================
Function Name: int findstr (const char *, const char *)
Description: find the sub-string from the source string.
Arguments: const char *, const char *
Return: Returns the array element number where 
      "sub-string" occurs in "original string", or -1
      if substr is not present in "original string".
written by jackyxie
Date   :  2007/07/10
================================================================*/
int findstr (char * sub_str, char * str)
{
   int i = -1, str_index;
   int sub_str_len, str_len;
   if (str == NULL || sub_str == NULL) return -1;
   sub_str_len = strlen(sub_str);
   str_len = strlen(str);

   /* sub_str has to be smaller than str */
   if (sub_str_len > str_len) return -1;

   /* look through str for sub_str, stopping at end of string or when the 
length
           of sub_str exceeds the remaining number of characters to be 
searched */ 
   while (str[++i] != '\0' && (str_len - i + 1) > sub_str_len) 
   {
      for (str_index = 0; str_index < sub_str_len; ++str_index)
      {
           /* definitely not at location i */
	   if (str[i + str_index] != sub_str[str_index])
		   break;
           /* if last letter matches, then we know the whole thing matched */
	   else if (str_index == sub_str_len - 1)
	     {
		   return i;
	     }
      }
   }
   /* sub_str not present in str */
   return -1;
}

int get_wan_rx_tx_status(struct net_device_stats *stats)
{
	int res;
	struct net_device_stats wan_stats;
	res = re865xIoctl("eth0", RTL865X_IOCTL_GET_WANSTATUS, (unsigned int)stats,0,0);

	return 1;
}

#if 0
/* get wan interface port status : it's enable as return 1 */
int get_wan_if_status()
{
	char *if_status=NULL;
	
	web_get_port_status(&if_status);
	if(findstr("Showtime", if_status) >=0 )
		return 1;
	else
		return 0;
}
#endif

