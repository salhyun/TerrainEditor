#include "SecretEffectManager.h"

SEffectObject::SEffectObject()
{
	pcEffect = NULL ;
}
SEffectObject::~SEffectObject()
{
}
void SEffectObject::setEffect(CSecretEffect *effect, float timelimit, int operation, int attr)
{
	pcEffect = effect ;
	D3DXMatrixIdentity(&matWorld) ;
	fElapsedTime = 0.0f ;
	fTimeLimit = timelimit ;
	nOperation = operation ;
	nAttr = attr ;
}
void SEffectObject::setPosition(float x, float y, float z)
{
	matWorld._41 = x ;
	matWorld._42 = y ;
	matWorld._43 = z ;
}
void SEffectObject::process(float time)
{
	if(nOperation == ONCE)
	{
		fElapsedTime += time ;
		if(fElapsedTime > fTimeLimit)
			pcEffect = NULL ;
	}
	else if(nOperation == LOOP)
	{
		fElapsedTime += time ;
		if(fElapsedTime > fTimeLimit)
			fElapsedTime = (fElapsedTime - fTimeLimit) ;
	}
	else if(nOperation == SHUTTLE)
	{
		if(nAttr & ATTR_REVERSE)
		{
			fElapsedTime -= time ;
			if(fElapsedTime < 0.0f)
			{
				fElapsedTime = fElapsedTime + time ;
				nAttr &= (~ATTR_REVERSE) ;
			}
		}
		else
		{
			fElapsedTime += time ;
			if(fElapsedTime > fTimeLimit)
			{
				fElapsedTime = fElapsedTime - time ;
				nAttr |= ATTR_REVERSE ;
			}
		}
	}
}
void SEffectObject::deleteEffect()
{
	pcEffect = NULL ;
}
void SEffectObject::setAttr(int attr, bool enable)
{
	if(enable)
		nAttr |= attr ;
	else
		nAttr &= (~attr) ;
}

//###########################################################################################//
// CSecretEffectManager
//###########################################################################################//

CSecretEffectManager::CSecretEffectManager()
{
	m_nNumKind = 0 ;
	m_ppcEffectKinds = NULL ;
}
CSecretEffectManager::~CSecretEffectManager()
{
	release() ;
}
void CSecretEffectManager::release()
{
	m_sEffects.ReleaseAll() ;

	if(m_ppcEffectKinds)
	{
		for(int i=0 ; i<m_nNumKind ; i++)
			SAFE_DELETE(m_ppcEffectKinds[i]) ;

		SAFE_DELETEARRAY(m_ppcEffectKinds) ;
	}
}

void CSecretEffectManager::initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;
	m_sEffects.Initialize(MAXNUM_EFFECTOBJECT) ;
}
void CSecretEffectManager::setBillboardEffects(int nNum, char **ppszFileNames)
{
	//m_nNumKind = KIND_END ;
	m_nNumKind = nNum ;
	m_ppcEffectKinds = new CSecretEffect*[m_nNumKind] ;

	float afOffset_inPixel[] = {10.0f, 55.0f, 55.0f, 55.0f, 55.0f, 55.0f, 55.0f, 22.0f, 128.0f} ;

	int i ;

	for(i=0 ; i<nNum ; i++)
	{
		CSecretBillboardEffect *pcBillboardEffect = new CSecretBillboardEffect() ;

		pcBillboardEffect->setEffect(m_pcBillboardEffectEffect) ;
		pcBillboardEffect->initialize(m_pd3dDevice, ppszFileNames[i], afOffset_inPixel[i]) ;
		pcBillboardEffect->loadVolumeTexture(ppszFileNames[i]) ;
		m_ppcEffectKinds[i] = (CSecretEffect *)pcBillboardEffect ;
	}
}
void CSecretEffectManager::process(float time)
{
	int i ;
	SEffectObject *psEffectObject ;
	for(i=0 ; i<m_sEffects.nCurPos ; i++)
	{
		psEffectObject = m_sEffects.GetAt(i) ;
		if(psEffectObject->pcEffect)
			psEffectObject->process(time) ;
	}

	i=0 ;
	while(i < m_sEffects.nCurPos)
	{
		psEffectObject = m_sEffects.GetAt(i) ;
		if(!psEffectObject->pcEffect)
		{
			m_sEffects.Delete(i) ;//지우고나면 밀려지니까 한번더 한다
		}
		else
		{
			i++ ;
		}
	}
}
void CSecretEffectManager::render()
{
	if(!m_sEffects.nCurPos)
		return ;

	float fTime ;
	SEffectObject *psEffectObject ;
	for(int i=0 ; i<m_sEffects.nCurPos ; i++)
	{
		psEffectObject = m_sEffects.GetAt(i) ;
		if(psEffectObject->nAttr & SEffectObject::ATTR_INVISIBLE)
			continue ;

		if(psEffectObject->pcEffect)
		{
			fTime = (psEffectObject->fElapsedTime >= 0.9f) ? 0.9f : psEffectObject->fElapsedTime ;

			psEffectObject->pcEffect->render(fTime, &psEffectObject->matWorld) ;
		}
	}
}
void CSecretEffectManager::setBillboardEffectEffect(CSecretBillboardEffectEffect *pcBillboardEffectEffect)
{
	m_pcBillboardEffectEffect = pcBillboardEffectEffect ;
}
SEffectObject *CSecretEffectManager::addEffect(int nKind, D3DXVECTOR3 pos, float fTimeLimit, int nOperation, int nAttr)
{
	SEffectObject sEffectObject ;
	sEffectObject.setEffect(m_ppcEffectKinds[nKind], fTimeLimit, nOperation, nAttr) ;
	sEffectObject.setPosition(pos.x, pos.y, pos.z) ;
	m_sEffects.Insert(&sEffectObject) ;
	return (SEffectObject *)m_sEffects.GetAt(m_sEffects.nCurPos-1) ;
}
