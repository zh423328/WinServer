#include "Gen.h"


static CGenManager * m_pShareManager = NULL;

CGenManager::CGenManager()
{
	m_pSessionGen = new CGen();
}

CGenManager::~CGenManager()
{
	SAFE_DELETE(m_pSessionGen);
}

CGenManager * CGenManager::shareGenManager()
{
	if(m_pShareManager == NULL)
	{
		m_pShareManager = new CGenManager();
	}

	return m_pShareManager;
}

xe_uint32 CGenManager::GenSessionIdx()
{
	return m_pSessionGen->GenIdx();
}
