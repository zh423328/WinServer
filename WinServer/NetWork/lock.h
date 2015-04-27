//////////////////////////////////////////////////////////////////////////
//lock
//////////////////////////////////////////////////////////////////////////
#ifndef LOCK_H_
#define LOCK_H_
#include "GlobalDefine.h"

//ÁÙ½çÇø
class CLock
{
public:
	CLock()
	{
		InitializeCriticalSection(&mCs);
	}
	~CLock()
	{
		DeleteCriticalSection(&mCs);
	}

	//Ìí¼ÓËø
	bool AttemptLock()
	{
		return TryEnterCriticalSection(&mCs) == TRUE ? true : false;
	}

	void Lock()
	{
		EnterCriticalSection(&mCs);
	}

	void UnLock()
	{
		LeaveCriticalSection(&mCs);
	}
private:
	CRITICAL_SECTION mCs;
};


class CAutoLock
{
public:
	CAutoLock(CLock *pCs)
	{
		m_pCs = pCs;

		if (m_pCs)
			m_pCs->Lock();
	}

	~CAutoLock()
	{
		if (m_pCs)
			m_pCs->UnLock();
	}
private:
	CLock *m_pCs;
};

#endif