#ifndef SESSIONPOOL_H_
#define SESSIONPOOL_H_

#include "GlobalDefine.h"


//连接池
class CSessionList;
class CSession;

class CSessionPool 
{
public:
	CSessionPool(xe_uint32 dwSize);
	~CSessionPool();


	//预先创建存储
	void Create(xe_uint32 dwSize);

	void CreateStepSize();

	//分配一个session
	CSession* Alloc();

	//释放
	void	Free(CSession *pSession);

	//剩余多少session可分配
	xe_uint32 GetLength();

private:
	CSessionList * m_pList;			//连接链

	xe_uint32	m_nSize;			//最大大小
	xe_uint32	m_nStepSize;		//1/10
};

#endif