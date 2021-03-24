#include "SecretAssistObject.h"
#include "MathOrdinary.h"
#include "D3DMathLib.h"
#include "ASEData.h"
#include "SecretTextureContainer.h"
#include "SecretGizmoSystem.h"
#include "SecretToolBrush.h"

//##########################################
//SAssistObjectKind
//##########################################
SAssistObjectKind::SAssistObjectKind()
{
	sprintf(szName, "") ;
	nNumVertex = nNumIndex = 0 ;
	pVB = NULL ;
	pIB = NULL ;
	pTex = NULL ;
}
SAssistObjectKind::~SAssistObjectKind()
{
	Release() ;
}
void SAssistObjectKind::Release()
{
    SAFE_RELEASE(pVB) ;
	SAFE_RELEASE(pIB) ;
}
HRESULT SAssistObjectKind::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszName, SAssistObjectVertex *psSrcVertices, int nNumSrcVertex, SAssistObjectIndex *psSrcIndices, int nNumSrcIndex, LPDIRECT3DTEXTURE9 pTexture)
{
	int i=0 ;

	assert(strlen(pszName) < 128) ;
	sprintf(szName, "%s", pszName) ;

	//Create VertexBuffer
	nNumVertex = nNumSrcVertex ;
	if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(SAssistObjectVertex), D3DUSAGE_WRITEONLY, D3DFVF_ASSISTOBJECT, D3DPOOL_MANAGED, &pVB, NULL)))
		return E_FAIL ;

	SAssistObjectVertex *psDestVertices=NULL ;
	if(FAILED(pVB->Lock(0, nNumVertex*sizeof(SAssistObjectVertex), (void **)&psDestVertices, 0)))
		return E_FAIL ;

	for(i=0 ; i<nNumVertex ; i++, psSrcVertices++, psDestVertices++)
		*psDestVertices = *psSrcVertices ;

	pVB->Unlock() ;

	//Create IndexBuffer
	nNumIndex = nNumSrcIndex ;
	if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(SAssistObjectIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
		return E_FAIL ;

	SAssistObjectIndex *psDestIndices=NULL ;
	if(FAILED(pIB->Lock(0, nNumIndex*sizeof(SAssistObjectIndex), (void **)&psDestIndices, 0)))
		return E_FAIL ;

	for(i=0 ; i<nNumIndex ; i++, psSrcIndices++, psDestIndices++)
		*psDestIndices = *psSrcIndices ;

	pIB->Unlock() ;

	pTex = pTexture ;

	return S_OK ;
}

//##########################################
//CSecretAssistObject
//##########################################

CSecretAssistObject::CSecretAssistObject()
{
	m_pd3dDevice = NULL ;
	m_psObjectKind = NULL ;
	m_pcGizmo = NULL ;
	m_pcToolBrush = NULL ;
	m_nStatus = CSecretAssistObject::UNSELECTED ;//  STATUS::UNSELECTED ;
}
CSecretAssistObject::~CSecretAssistObject()
{
}
void CSecretAssistObject::Release()
{
	m_pcGizmo->SetEnable(false) ;
}
bool CSecretAssistObject::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SAssistObjectKind *psObjectKind, CSecretGizmoSystem *pcGizmo, CSecretToolBrush *pcToolBrush)
{
	m_pd3dDevice = pd3dDevice ;
	m_psObjectKind = psObjectKind ;
	m_pcGizmo = pcGizmo ;
	m_pcToolBrush = pcToolBrush ;
	D3DXMatrixIdentity(&m_matTransform) ;
	D3DXMatrixScaling(&m_matTransform, 0.1f, 0.1f, 0.1f) ;
	m_nStatus = CSecretAssistObject::UNSELECTED ;
	return true ;
}
void CSecretAssistObject::Process(Vector3 *pvCameraPos, bool bPressedLButton)
{
	if(m_nStatus == CSecretAssistObject::PICKING)
	{
		CSecretPicking *pcPicking = m_pcToolBrush->m_pcPicking ;

		m_matTransform._41 = pcPicking->GetCurVertex()->pos.x ;
		m_matTransform._42 = pcPicking->GetCurVertex()->pos.y ;
		m_matTransform._43 = pcPicking->GetCurVertex()->pos.z ;

		if(bPressedLButton)
			SetStatus(CSecretAssistObject::SELECTED, pvCameraPos) ;
	}
	else if(m_nStatus == CSecretAssistObject::SELECTED)
	{
		D3DXMATRIX matView = MatrixConvert(*m_pcToolBrush->m_pmatView) ;
		m_pcGizmo->Process(pvCameraPos, &m_pcToolBrush->m_pcPicking->m_sLine, &matView, m_pcToolBrush->m_pmatProj, bPressedLButton) ;

		if(m_pcGizmo->m_sScrape.GetStatus() == SGizmoScrape::SCRAPE_START)
		{
			D3DXMatrixDecomposeTranslation(&m_matOriginTrans, &m_matTransform) ;
			D3DXMatrixDecomposeRotation(&m_matOriginRot, &m_matTransform) ;
			D3DXMatrixDecomposeScaling(&m_matOriginScale, &m_matTransform) ;
		}
		else if(m_pcGizmo->m_sScrape.GetStatus() == SGizmoScrape::SCRAPE_SCRAPING)
		{
			if(m_pcGizmo->m_sGizmoMoveGeometry.dwSelectedSubset == GIZMOSUBSET_AXISX)
			{
				m_matTransform._41 += m_pcGizmo->m_sScrape.vAmount.x ;
			}
			else if(m_pcGizmo->m_sGizmoMoveGeometry.dwSelectedSubset == GIZMOSUBSET_AXISY)
			{
				m_matTransform._42 += m_pcGizmo->m_sScrape.vAmount.y ;
			}
			else if(m_pcGizmo->m_sGizmoMoveGeometry.dwSelectedSubset == GIZMOSUBSET_AXISZ)
			{
				m_matTransform._43 += m_pcGizmo->m_sScrape.vAmount.z ;
			}
			else if(m_pcGizmo->m_sGizmoRotateGeometry.dwSelectedSubset == GIZMOSUBSET_ROTATEBYAXISX
				|| m_pcGizmo->m_sGizmoRotateGeometry.dwSelectedSubset == GIZMOSUBSET_ROTATEBYAXISY
				|| m_pcGizmo->m_sGizmoRotateGeometry.dwSelectedSubset == GIZMOSUBSET_ROTATEBYAXISZ)
			{
				m_matTransform = m_matOriginScale * m_matOriginRot * m_pcGizmo->m_sGizmoRotateGeometry.matRotate * m_matOriginTrans ;
			}
		}
	}
}
void CSecretAssistObject::SetStatus(int nStatus, Vector3 *pvCameraPos)
{
	m_nStatus = nStatus ;

	if(m_nStatus == CSecretAssistObject::SELECTED)
	{
		m_pcGizmo->SetEnable(true) ;
		//m_pcGizmo->SetType(CSecretGizmoSystem::GIZMOTYPE::MOVE) ;

        D3DXMATRIX mat, matView ;
		D3DXMatrixIdentity(&mat) ;
		mat._41 = m_matTransform._41 ;
		mat._42 = m_matTransform._42 ;
		mat._43 = m_matTransform._43 ;
        m_pcGizmo->SetmatWorld(&mat) ;

		matView = MatrixConvert(*m_pcToolBrush->m_pmatView) ;
		m_pcGizmo->Process(pvCameraPos, &m_pcToolBrush->m_pcPicking->m_sLine, &matView, m_pcToolBrush->m_pmatProj, false) ;
	}
	else if(m_nStatus == CSecretAssistObject::UNSELECTED)
	{
		m_pcGizmo->SetEnable(false) ;
	}
}
void CSecretAssistObject::Render()
{
	D3DXMATRIX matOldWorld ;
	m_pd3dDevice->GetTransform(D3DTS_WORLD, &matOldWorld) ;

	DWORD dwTssColorArg1 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwTssColorArg1) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE) ;
	
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_matTransform) ;
	m_pd3dDevice->SetFVF(D3DFVF_ASSISTOBJECT) ;
	m_pd3dDevice->SetStreamSource(0, m_psObjectKind->pVB, 0, sizeof(SAssistObjectVertex)) ;
	m_pd3dDevice->SetIndices(m_psObjectKind->pIB) ;
	m_pd3dDevice->SetTexture(0, m_psObjectKind->pTex) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_psObjectKind->nNumVertex, 0, m_psObjectKind->nNumIndex) ;

    m_pd3dDevice->SetTransform(D3DTS_WORLD, &matOldWorld) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwTssColorArg1) ;
}
void CSecretAssistObject::SetmatTransform(D3DXMATRIX *pmat)
{
	m_matTransform = *pmat ;
}
SAssistObjectKind *CSecretAssistObject::GetObjectKind()
{
	return m_psObjectKind ;
}
D3DXMATRIX *CSecretAssistObject::GetTransform()
{
	return &m_matTransform ;
}

//##########################################
//CSecretAssistObjectManager
//##########################################

CSecretAssistObjectManager::CSecretAssistObjectManager() : MAXNUM_OBJECT(12)
{
	m_pd3dDevice = NULL ;
	m_nNumObjectKind = 0 ;
	m_psObjectKind = NULL ;
	m_pcCurrentObject = NULL ;
	m_pcTexContainer = NULL ;
	m_pcGizmo = NULL ;
	m_nAttr = 0 ;
	m_nStatus = CSecretAssistObjectManager::IDLING ;
}
CSecretAssistObjectManager::~CSecretAssistObjectManager()
{
	Release() ;
}
void CSecretAssistObjectManager::Release()
{
	for(int i=0 ; i<m_cObjects.nCurPos ; i++)
		m_cObjects.GetAt(i)->Release() ;
	m_cObjects.Reset() ;

	SAFE_DELETE(m_pcTexContainer) ;
	SAFE_DELETE(m_pcGizmo) ;
	SAFE_DELETEARRAY(m_psObjectKind) ;
}
void CSecretAssistObjectManager::_InitObjectKind()
{
    int i, n ;
	char str[256] ;
	char **ppszFileList = new char*[16] ;
	for(i=0 ; i<16; i++)
		ppszFileList[i] = new char[256] ;

	//int nNumItem = GetAllFileNames(&ppszFileList, "../../Media/assist_objects/", "RMD") ;
	//assert(nNumItem < 16) ;

	sprintf(ppszFileList[0], "../../Media/assist_objects/black_tee.RMD") ;
	sprintf(ppszFileList[1], "../../Media/assist_objects/white_tee.RMD") ;
	sprintf(ppszFileList[2], "../../Media/assist_objects/red_tee.RMD") ;
	sprintf(ppszFileList[3], "../../Media/assist_objects/second.RMD") ;
	sprintf(ppszFileList[4], "../../Media/assist_objects/third.RMD") ;
	sprintf(ppszFileList[5], "../../Media/assist_objects/holecup.RMD") ;
	int nNumItem = 6 ;

	if(nNumItem)
	{
		m_nNumObjectKind = nNumItem ;
		m_psObjectKind = new SAssistObjectKind[m_nNumObjectKind] ;

		CASEData data ;
		for(i=0 ; i<m_nNumObjectKind ; i++)
		{
			if(data.ImportRMD(ppszFileList[i]))
			{
				assert(data.m_nNumMesh == 1) ;
				assert(data.m_nNumMaterial == 1) ;

				RemoveExt(str, data.m_psMaterial[0].szDecaleName) ;
				LPDIRECT3DTEXTURE9 pTexture = m_pcTexContainer->FindTexture(str) ;

				SMesh *psMesh = data.m_apMesh[0] ;

				int nNumVertex = psMesh->nNumVertex ;
				int nNumIndex = psMesh->nNumTriangle ;

				SAssistObjectVertex *psVertices = new SAssistObjectVertex[nNumVertex] ;
				SAssistObjectIndex *psIndices = new SAssistObjectIndex[nNumIndex] ;

				for(n=0 ; n<nNumVertex ; n++)
				{
					psVertices[n].pos = D3DXVECTOR3(psMesh->psVertex[n].pos.x, psMesh->psVertex[n].pos.y, psMesh->psVertex[n].pos.z) ;
					psVertices[n].tex = D3DXVECTOR2(psMesh->psVertex[n].tex.u, psMesh->psVertex[n].tex.v) ;
				}

				for(n=0 ; n<nNumIndex ; n++)
				{
					psIndices[n].awIndex[0] = psMesh->psTriangle[n].anVertexIndex[0] ;
					psIndices[n].awIndex[1] = psMesh->psTriangle[n].anVertexIndex[1] ;
					psIndices[n].awIndex[2] = psMesh->psTriangle[n].anVertexIndex[2] ;
				}

				GetFileNameFromPath(str, ppszFileList[i]) ;
				if(FAILED(m_psObjectKind[i].Initialize(m_pd3dDevice, str, psVertices, nNumVertex, psIndices, nNumIndex, pTexture)))
				{
                    sprintf(str, "item count=%d E_FAIL", i) ;
					MessageBox(NULL, str, "vertex/index buffer", MB_ICONERROR) ;
				}

				delete []psVertices ;
				delete []psIndices ;

				data.Release() ;
			}
		}
	}

	for(i=0 ; i<16; i++)
		delete []ppszFileList[i] ;
	delete []ppszFileList ;
}
bool CSecretAssistObjectManager::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretToolBrush *pcToolBrush)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcToolBrush = pcToolBrush ;

	m_cObjects.Initialize(MAXNUM_OBJECT) ;
	m_pcCurrentObject = NULL ;

	m_pcTexContainer = new CSecretTextureContainer() ;
	m_pcTexContainer->Initialize("../../Media/assist_objects/map/assist_objects.txr", m_pd3dDevice) ;

	_InitObjectKind() ;

	m_pcGizmo = new CSecretGizmoSystem() ;
	m_pcGizmo->Initialize(m_pd3dDevice) ;
	m_pcGizmo->SetEnable(false) ;
	AddAttr(CSecretAssistObjectManager::VISIBILITY) ;

	return true ;
}
void CSecretAssistObjectManager::Process(Vector3 vCameraPos, bool bPressedLButton)
{
	if(m_nStatus != CSecretAssistObjectManager::PROCESSING)
		return ;

    if(m_pcCurrentObject)
	{
		m_pcCurrentObject->Process(&vCameraPos, bPressedLButton) ;
	}

}
void CSecretAssistObjectManager::Render()
{
	if(!(m_nAttr & CSecretAssistObjectManager::VISIBILITY))
		return ;

	for(int i=0 ; i<m_cObjects.nCurPos ; i++)
	{
		m_cObjects.GetAt(i)->Render() ;
	}

    if(m_nStatus == CSecretAssistObjectManager::PROCESSING)
		m_pcGizmo->Render() ;
}

void CSecretAssistObjectManager::CreateAssistObject(char *pszName)
{
	SAssistObjectKind *psObjKind = FindObjectKind(pszName) ;

    CSecretAssistObject cObject ;
	cObject.Initialize(m_pd3dDevice, psObjKind, m_pcGizmo, m_pcToolBrush) ;
	m_cObjects.Insert(&cObject) ;
}
CSecretAssistObject *CSecretAssistObjectManager::SelectAssistObject(int nIndex)
{
	m_pcCurrentObject = m_cObjects.GetAt(nIndex) ;
	return m_pcCurrentObject ;
}
void CSecretAssistObjectManager::DeleteAssistObject(int nIndex)
{
	CSecretAssistObject *pcObject = m_cObjects.Delete(nIndex) ;
	if(pcObject)
	{
		pcObject->Release() ;
		if(pcObject == m_pcCurrentObject)
			m_pcCurrentObject = NULL ;
	}
}
void CSecretAssistObjectManager::ImportAssistObject(char *pszName, D3DXMATRIX *pmat)
{
	CreateAssistObject(pszName) ;
	m_cObjects.GetAt(m_cObjects.nCurPos-1)->SetmatTransform(pmat) ;
}
void CSecretAssistObjectManager::ResetObjects()
{
	for(int i=0 ; i<m_cObjects.nCurPos ; i++)
		m_cObjects.GetAt(i)->Release() ;
	m_cObjects.Reset() ;
	m_pcCurrentObject = NULL ;
}
CSecretTextureContainer *CSecretAssistObjectManager::GetTexContainer()
{
	return m_pcTexContainer ;
}
SAssistObjectKind *CSecretAssistObjectManager::FindObjectKind(char *pszName)
{
	for(int i=0 ; i<m_nNumObjectKind ; i++)
	{
		if(!strcmp(pszName, m_psObjectKind[i].szName))
			return &m_psObjectKind[i] ;
	}
	return NULL ;
}
void CSecretAssistObjectManager::SetEnable(bool bEnable)
{
	if(bEnable)
	{
		m_pcToolBrush->AddAttr(CSecretToolBrush::DRAWBRUSH, false) ;
		SetStatus(CSecretAssistObjectManager::PROCESSING) ;
	}
	else
	{
		m_pcToolBrush->AddAttr(CSecretToolBrush::DRAWBRUSH) ;
		SetStatus(CSecretAssistObjectManager::IDLING) ;
	}

	m_bEnable = bEnable ;
}
bool CSecretAssistObjectManager::GetEnable()
{
	return m_bEnable ;
}
void CSecretAssistObjectManager::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
bool CSecretAssistObjectManager::IsAttr(int nAttr)
{
	if(m_nAttr & nAttr)
		return true ;
	return false ;
}
void CSecretAssistObjectManager::SetStatus(int nStatus)
{
	m_nStatus = nStatus ;
}
int CSecretAssistObjectManager::GetStatus()
{
	return m_nStatus ;
}
D3DXMATRIX *CSecretAssistObjectManager::GetObjectMatrix(char *pszName)
{
	for(int i=0 ; i<m_cObjects.nCurPos ; i++)
	{
		if(!strcmp(m_cObjects[i].GetObjectKind()->szName, pszName))
			return m_cObjects[i].GetTransform() ;
	}
	return NULL ;
}