#include "GlobalDefine.h"

#ifdef USE_GBK

char * xe_strstr( const char* src,const char* szFind )
{
	if (src == NULL || szFind == NULL)
		return NULL;

	char * cp = (char*)src;

	//û���ַ�
	if (*szFind == '\0')
		return NULL;

	char * s1, *s2;

	//ѭ������
	while(*cp)
	{
		s1 = cp;
		s2 = (char *)szFind;

		//��ͬ
		while(*s1 && *s2 && !(*s1 - *s2))
			s1++,s2++;

		//ĩβ
		if (!*s2)
			return s1;

		//����
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

	//������
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


