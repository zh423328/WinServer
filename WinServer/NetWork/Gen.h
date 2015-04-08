#ifndef GEN_H_
#define GEN_H_

//////////////////////////////////////////////////////////////////////////
//id生成器
//////////////////////////////////////////////////////////////////////////
#include "GlobalDefine.h"

class CGen
{
public:
	CGen(xe_uint32 dwStart = 0)
	{
		dwStart = dwStart;
	}

	~CGen()
	{

	}

	xe_uint32 GenIdx()
	{
		++dwStart;

		if (dwStart == 0xFFFFFFFF)
			dwStart = 1;

		return dwStart;
	}
private:
	xe_uint32 dwStart;
};


//gen管理器
class CGenManager
{
public:
	CGenManager();
	~CGenManager();

	static CGenManager * shareGenManager();

	xe_uint32  GenSessionIdx();

private:
	CGen*	m_pSessionGen;
};

#endif