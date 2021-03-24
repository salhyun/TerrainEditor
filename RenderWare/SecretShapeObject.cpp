#include "SecretShapeObject.h"
#include "SecretTerrain.h"

CSecretShapeObject::CSecretShapeObject()
{
	m_nNumShape = 0 ;
	m_bRender = 0 ;
	m_ppcShape = NULL ;
}
CSecretShapeObject::~CSecretShapeObject()
{
	Release() ;
}
void CSecretShapeObject::Release()
{
	for(int i=0 ; i<m_nNumShape ; i++)
	{
		m_ppcShape[i]->Release() ;
		SAFE_DELETE(m_ppcShape[i]) ;
	}
	SAFE_DELETEARRAY(m_ppcShape) ;
}
bool CSecretShapeObject::Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice)
{
    if(pcASEData->m_lKind != CASEData::KIND_SHAPE)
		return false ;

	m_pd3dDevice = pd3dDevice ;

	D3DXMATRIX mat ;
	Vector3 pos ;
    SShapeObject *psShapeObject ;
	int i, n, count ;

	n=0 ;
	for(i=0 ; i<pcASEData->m_nNumShapeObject ; i++)
	{
		psShapeObject = pcASEData->m_apsShapeObject[i] ;
		m_nNumShape += psShapeObject->nNumLine ;
	}
	m_ppcShape = new CSecretShape*[m_nNumShape] ;

    count=0 ;
	for(i=0 ; i<pcASEData->m_nNumShapeObject ; i++)
	{
		psShapeObject = pcASEData->m_apsShapeObject[i] ;
		//memcpy(&mat, &psShapeObject->smatLocal, sizeof(D3DXMATRIX)) ;

		//Shape Line
		for(n=0 ; n<psShapeObject->nNumLine ; n++)
		{
			CSecretShapeLine *psShapeLine = new CSecretShapeLine(CSecretShape::KNID_LINE) ;
			////psShapeLine->SetLocalTM(&mat) ;
			psShapeLine->Initialize(&psShapeObject->psShapeLine[n], m_pd3dDevice) ;
			psShapeLine->SetName(psShapeObject->szName) ;
			m_ppcShape[count++] = (CSecretShape *)psShapeLine ;
		}
	}
	assert(count == m_nNumShape) ;
	return true ;
}
void CSecretShapeObject::Process(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj, bool bPressed, D3DXVECTOR3 *pvCurPos)
{
	if(!m_bRender)
		return ;

	CSecretShapeLine *psShapeLine ;

	for(int i=0 ; i<m_nNumShape ; i++)
	{
		if(m_ppcShape[i]->GetKind() == CSecretShape::KNID_LINE)
		{
			psShapeLine = (CSecretShapeLine *)m_ppcShape[i] ;
			psShapeLine->Process(pvCamera, pvUp, pmatView, pmatProj) ;
		}
	}
}
void CSecretShapeObject::Render()
{
	if(!m_bRender)
		return ;

	int i ;
	for(i=0 ; i<m_nNumShape ; i++)
	{
		if(m_ppcShape[i]->GetAttr() & CSecretShape::ATTR_RENDER)
			m_ppcShape[i]->Render() ;
	}
}
int CSecretShapeObject::GetNumShape()
{
	return m_nNumShape ;
}
CSecretShape *CSecretShapeObject::GetShape(int nNum)
{
	return m_ppcShape[nNum] ;
}
void CSecretShapeObject::OnTerrain(CSecretTerrain *pcTerrain)
{
	CSecretShapeLine *psShapeLine ;
    for(int i=0 ; i<m_nNumShape ; i++)
	{
		if(m_ppcShape[i]->GetKind() == CSecretShape::KNID_LINE)
		{
			psShapeLine = (CSecretShapeLine *)m_ppcShape[i] ;
			psShapeLine->OnTerrain(pcTerrain) ;
		}
	}
}
void CSecretShapeObject::SetRender(int bRender)
{
	m_bRender = bRender ;
}
bool CSecretShapeObject::IsRender()
{
	if(m_bRender)
		return true ;
	return false ;
}