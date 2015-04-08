#include "GlobalDefine.h"

#ifdef USE_GBK

char * xe_strstr( const char* src,const char* szFind )
{
	if (src == NULL || szFind == NULL)
		return NULL;

	char * cp = (char*)src;

	//没有字符
	if (*szFind == '\0')
		return NULL;

	char * s1, *s2;

	//循环查找
	while(*cp)
	{
		s1 = cp;
		s2 = (char *)szFind;

		//相同
		while(*s1 && *s2 && !(*s1 - *s2))
			s1++,s2++;

		//末尾
		if (!*s2)
			return s1;

		//否则
		if (*cp++ < 0)
			cp++;
	}

	return NULL;
}

char * xe_strchr(const char* src,int ch)
{
	if(src == NULL)
		return NULL;

	char * cp = (char*)src;

	//不等于
	while(*cp && (*cp != (char)ch))
	{
		if (*cp++ < 0)
			cp++;
	}

	if (*cp == (char)ch)
		return cp;
	
	return NULL;
}

#endif
