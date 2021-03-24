#include "SecretRenderingGroup.h"
#include "SecretSceneManager.h"
#include "SecretRigidMesh.h"

UINT lRenderingNodeIndexCount = 0 ;

bool CompareRenderingNode(void *pv1, void *pv2)
{
	SRenderingNode *psNode1 = (SRenderingNode *)pv1 ;
	SRenderingNode *psNode2 = (SRenderingNode *)pv2 ;

	if(psNode1->nIndex == psNode2->nIndex)
		return true ;
    return false ;
}

int compare_DistfromCamera_greater(const void *pv1, const void *pv2)//decreasing order 내림차순 (적은숫자 -> 큰숫자)
{
	SRenderingNode *pNode1 = (SRenderingNode *)pv1 ;
	SRenderingNode *pNode2 = (SRenderingNode *)pv2 ;

	float dist = pNode1->fDistfromCamera - pNode2->fDistfromCamera ;
    if(float_less(dist, 0.0f))
		return 1 ;
	else if(float_greater(dist, 0.0f))
		return -1 ;
	else
		return 0 ;
}
int compare_DistfromCamera_less(const void *pv1, const void *pv2)//increasing order 오름차순 (큰숫자 -> 적은숫자)
{
	SRenderingNode *pNode1 = (SRenderingNode *)pv1 ;
	SRenderingNode *pNode2 = (SRenderingNode *)pv2 ;

	float dist = pNode1->fDistfromCamera - pNode2->fDistfromCamera ;
    if(float_less(dist, 0.0f))
		return -1 ;
	else if(float_greater(dist, 0.0f))
		return 1 ;
	else
		return 0 ;
}

//#############################################################
//CSecretRenderingGroup
//#############################################################
CSecretRenderingGroup::CSecretRenderingGroup()
{
	m_psObjects = NULL ;
	m_psRolls = NULL ;
	m_pcShadowMap = NULL ;
	m_pcWaterManager = NULL ;
	D3DXMatrixIdentity(&m_matMirror) ;
}

CSecretRenderingGroup::~CSecretRenderingGroup()
{
	Release() ;
}

void CSecretRenderingGroup::Release()
{
	SAFE_DELETEARRAY(m_psRolls) ;
	SAFE_DELETEARRAY(m_psObjects) ;
}

bool CSecretRenderingGroup::Initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;
	m_nObjectCount = 0 ;
	m_psObjects = new SWaitingforRenderingObject[MAXNUM_WAITINGFORRENDERINGOBJECT] ;

	m_psRolls = new CSecretRenderingRoll[MAXNUM_RENDERINGROLL] ;

	m_psRolls[0].Initialize(CSecretRenderingRoll::RROLL_ATTR_NEARTOFAR) ;//알파블렌딩 안하는 메쉬
	m_psRolls[1].Initialize(CSecretRenderingRoll::RROLL_ATTR_FARTONEAR) ;//알파블렌딩 하는 메쉬

	return true ;
}

void CSecretRenderingGroup::Process(float fFrame)
{
	CSecretRenderingRoll *psRolls = m_psRolls ;
	for(int i=0 ; i<MAXNUM_RENDERINGROLL ; i++, psRolls++)
	{
		psRolls->Sorting() ;
		//psRolls->Process(fFrame) ;
	}
}

void CSecretRenderingGroup::Render(SD3DEffect *psEffect, bool bWireFrame, int nInstantAttr, int nDistNear, int nDistFar)
{
    CSecretRenderingRoll *psRolls = m_psRolls ;
	for(int i=0 ; i<MAXNUM_RENDERINGROLL ; i++, psRolls++)
	{
//		if(nInstantAttr & CSecretRenderingRoll::RROLL_ATTR_INSTANTBLENDTOTEST)//임시로 강제로 알파블랜딩메쉬를 알파테스트로 랜더링
//			psRolls->AddAttr(CSecretRenderingRoll::RROLL_ATTR_INSTANTBLENDTOTEST) ;

		if(nInstantAttr & CSecretRenderingRoll::RROLL_ATTR_BEHINDMIRRORRENDER)
			psRolls->AddAttr(CSecretRenderingRoll::RROLL_ATTR_BEHINDMIRRORRENDER) ;

		if(nInstantAttr & CSecretRenderingRoll::RROLL_ATTR_MIRRORRENDER)
		{
			psRolls->AddAttr(CSecretRenderingRoll::RROLL_ATTR_MIRRORRENDER) ;
			psRolls->m_pmatMirror = &m_matMirror ;
			psRolls->m_pcWaterManager = m_pcWaterManager ;
		}

		if(nInstantAttr & CSecretRenderingRoll::RROLL_ATTR_SHADOWRENDERING)
			psRolls->RenderShadow(m_pd3dDevice, psEffect, m_pcShadowMap) ;
		else
			psRolls->Render(m_pd3dDevice, psEffect, bWireFrame, nDistNear, nDistFar) ;

//		if(psRolls->IsAttr(CSecretRenderingRoll::RROLL_ATTR_INSTANTBLENDTOTEST))
//			psRolls->AddAttr(CSecretRenderingRoll::RROLL_ATTR_INSTANTBLENDTOTEST, false) ;//제거

		if(nInstantAttr & CSecretRenderingRoll::RROLL_ATTR_BEHINDMIRRORRENDER)
			psRolls->AddAttr(CSecretRenderingRoll::RROLL_ATTR_BEHINDMIRRORRENDER, false) ;//제거

		if(psRolls->IsAttr(CSecretRenderingRoll::RROLL_ATTR_MIRRORRENDER))
			psRolls->AddAttr(CSecretRenderingRoll::RROLL_ATTR_MIRRORRENDER, false) ;//제거
	}
}

void CSecretRenderingGroup::_Sorting()
{
	CSecretRenderingRoll *psRolls = m_psRolls ;
	for(int i=0 ; i<MAXNUM_RENDERINGROLL ; i++, psRolls++)
		psRolls->Sorting() ;
}

bool CSecretRenderingGroup::InsertObject(CSecretMeshObject *pcMeshObject, D3DXMATRIX *pmatWorld, Vector3 vCameraPos, int nAttr)
{
	int i ;
	CSecretMesh *pcMesh ;
	SWaitingforRenderingObject *psObject ;

	m_psObjects[m_nObjectCount].set(pcMeshObject, pmatWorld) ;
	psObject = &m_psObjects[m_nObjectCount++] ;
	assert(m_nObjectCount < MAXNUM_WAITINGFORRENDERINGOBJECT) ;

	for(i=0 ; i<pcMeshObject->GetNumMesh() ; i++)
	{
		pcMesh = pcMeshObject->GetMesh(i) ;
		if(!(nAttr & SRenderingNode::ATTR_ALPHATESTBYLOD))
		{
			if( (pcMesh->GetAttr() & SECRETNODEATTR_ALPHABLENDING) || (nAttr & SRenderingNode::ATTR_ALPHABLENDINGBYFORCE) )
				m_psRolls[ROLLKIND_ALPHABLENDED].InsertMesh(pcMesh, pcMeshObject, pmatWorld, &vCameraPos, nAttr) ;
			else
				m_psRolls[ROLLKIND_NOALPHABLENDED].InsertMesh(pcMesh, pcMeshObject, pmatWorld, &vCameraPos, nAttr) ;
		}
		else
			m_psRolls[ROLLKIND_NOALPHABLENDED].InsertMesh(pcMesh, pcMeshObject, pmatWorld, &vCameraPos, nAttr) ;

		//if(!(nAttr & SRenderingNode::ATTR_ALPHATESTBYLOD) && (pcMesh->GetAttr() & SECRETNODEATTR_ALPHABLENDING))
		//	m_psRolls[ROLLKIND_ALPHABLENDED].InsertMesh(pcMesh, pcMeshObject, pmatWorld, &vCameraPos, nAttr) ;
		//else
		//	m_psRolls[ROLLKIND_NOALPHABLENDED].InsertMesh(pcMesh, pcMeshObject, pmatWorld, &vCameraPos, nAttr) ;
	}

	return true ;
}

void CSecretRenderingGroup::ResetRolls()
{
	m_nObjectCount = 0 ;
	CSecretRenderingRoll *psRolls = m_psRolls ;
	for(int i=0 ; i<MAXNUM_RENDERINGROLL ; i++, psRolls++)
		psRolls->Reset() ;
}
void CSecretRenderingGroup::SetShadowMap(CSecretShadowMap *pcShadowMap)
{
	m_pcShadowMap = pcShadowMap ;
}
void CSecretRenderingGroup::SetWaterManager(CSecretWaterManager *pcWaterManager)
{
	m_pcWaterManager = pcWaterManager ;
}
void CSecretRenderingGroup::SetSpecificRenderingArea(Vector3 vPos, float fRadius)
{
	CSecretRenderingRoll *psRolls = m_psRolls ;
	for(int i=0 ; i<MAXNUM_RENDERINGROLL ; i++, psRolls++)
	{
		psRolls->m_sSpecificRenderingArea.vPos = vPos ;
		psRolls->m_sSpecificRenderingArea.fRadius = fRadius ;
	}
}
CSecretRenderingRoll *CSecretRenderingGroup::GetRenderingRolls()
{
	return m_psRolls ;
}

//#############################################################
//SRenderingNode
//#############################################################
SRenderingNode::SRenderingNode()
{
	bEnable=false; nAttr=0; pcMesh=NULL; pcParentObject=NULL; fDistfromCamera=0.0f; nIndex=0;
}
SRenderingNode::SRenderingNode(int index)
{
	bEnable=false; nAttr=0; pcMesh=NULL; pcParentObject=NULL; fDistfromCamera=0.0f; nIndex=index;
}
void SRenderingNode::set(CSecretMesh *mesh, CSecretMeshObject *object, int index, int attr)
{
	pcMesh=mesh; pcParentObject=object; nIndex=index; bEnable=true; fDistfromCamera=0.0f; nAttr=attr;
}

//#############################################################
//CSecretRenderingRoll
//#############################################################
CSecretRenderingRoll::CSecretRenderingRoll()
{
	m_nAttr = 0 ;
	m_nNodeCount = 0 ;
	m_psNodes = NULL ;
	m_pmatMirror = NULL ;
	m_pcWaterManager = NULL ;
	m_fAlphaTestDiffuse = 0.0466914f ;
}

CSecretRenderingRoll::~CSecretRenderingRoll()
{
	Release() ;
}

void CSecretRenderingRoll::Release()
{
	SAFE_DELETEARRAY(m_psNodes) ;
}

bool CSecretRenderingRoll::Initialize(int nAttr)
{
	m_psNodes = new SRenderingNode[MAXNUM_RENDERINGNODE] ;

	m_nAttr = nAttr ;
	if(m_nAttr & RROLL_ATTR_FARTONEAR)//알파블랜딩 하는것.
	{
		m_pfCompare = compare_DistfromCamera_greater ;
		m_dwZEnable = TRUE ;
		m_dwZWriteEnable = FALSE ;
		m_dwZFunc = D3DCMP_LESS ;
	}
	else if(m_nAttr & RROLL_ATTR_NEARTOFAR)//알파블랜딩 하지 않는것.
	{
		m_pfCompare = compare_DistfromCamera_less ;
		m_dwZEnable = TRUE ;
		m_dwZWriteEnable = TRUE ;
		m_dwZFunc = D3DCMP_LESSEQUAL ;
	}

	return true ;
}

void CSecretRenderingRoll::Process(float fFrame)
{
	int i ;
	SRenderingNode *psNodes = m_psNodes ;
	for(i=0 ; i<m_nNodeCount ; i++, psNodes++)
	{
		if(psNodes->pcMesh->GetKind() == SECRETNODEKIND_RIGID)
		{
			//if(psNodes->pcMesh->GetAttr() & SECRETNODEATTR_TWOSIDED)
			{
				CSecretRigidMesh *pcRigid = (CSecretRigidMesh *)psNodes->pcMesh ;
				if(pcRigid->m_sLeafAni.bEnable)
					pcRigid->m_sLeafAni.Process(fFrame) ;
			}
		}
	}
}
bool CSecretRenderingRoll::_RenderTestbySpecificRenderingArea(SRenderingNode *psNode)
{
	Vector3 p(psNode->pmatWorld->_41, 0, psNode->pmatWorld->_43) ;

	if((m_sSpecificRenderingArea.vPos-p).Magnitude() <= m_sSpecificRenderingArea.fRadius)
		return true ;

	return false ;
}
void CSecretRenderingRoll::Render(LPDIRECT3DDEVICE9 pd3dDevice, SD3DEffect *psEffect, bool bWireFrame, int nDistNear, int nDistFar)
{
	if(!m_nNodeCount)
		return ;

	DWORD dwZEnable, dwZWriteEnable, dwZFunc, dwAlphaTest, dwAlphaBlend ;

	pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
	pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend) ;

	pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWriteEnable) ;
	pd3dDevice->GetRenderState(D3DRS_ZFUNC, &dwZFunc) ;

	int i ;
	SRenderingNode *psNodes = m_psNodes ;

	if(m_nAttr & RROLL_ATTR_NEARTOFAR)//알파블랜딩 하지 않는것
	{
		pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
		pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE) ;

		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) ;

		D3DXMATRIX mat ;
		for(i=0 ; i<m_nNodeCount ; i++, psNodes++)
		{
			if( (nDistNear != nDistFar) && ((psNodes->fDistfromCamera < (float)nDistNear) || (psNodes->fDistfromCamera > (float)nDistFar)) )
				continue ;

            if(m_nAttr & RROLL_ATTR_MIRRORRENDER)
			{
				if(!_RenderTestbySpecificRenderingArea(psNodes) || !(psNodes->pcMesh->GetAttr() & SECRETNODEATTR_REFLECTED))
					continue ;
			}

			if((psNodes->nAttr & SRenderingNode::ATTR_ALPHATESTBYLOD) && (psNodes->pcMesh->GetAttr() & SECRETNODEATTR_ALPHABLENDING))
			{
				pd3dDevice->SetRenderState(D3DRS_ALPHAREF, psNodes->pcParentObject->m_dwAddAlpha) ;
				pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
				pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

				char *pszObjName = psNodes->pcParentObject->GetObjName() ;

				if(!strcmp(pszObjName, "00_soo_f1_20m_15727")
					|| !strcmp(pszObjName, "00_soo_f1_20mc_15010")
					|| !strcmp(pszObjName, "00_soo_f1_20mc_16754")
					|| !strcmp(pszObjName, "00_soo_f1_24m_16539")
					|| !strcmp(pszObjName, "00_soo_f1_25m_15199")
					|| !strcmp(pszObjName, "00_soo_f1_29m_17159")
					|| !strcmp(pszObjName, "00_soo_f1_29mc_16758")

					|| !strcmp(pszObjName, "00_soo_i1_13m_13411")

					|| !strcmp(pszObjName, "00_soo_s1_3m_9052")
					|| !strcmp(pszObjName, "00_soo_s1_4m_9464")
					|| !strcmp(pszObjName, "00_soo_s1_5m_9833")
					|| !strcmp(pszObjName, "00_soo_s1_7m_10412")
					|| !strcmp(pszObjName, "00_soo_s1_10m_9844")

					|| !strcmp(pszObjName, "00_soo_s2_2m_8114")
					|| !strcmp(pszObjName, "00_soo_s2_7m_18693")
					|| !strcmp(pszObjName, "00_soo_s2_8m_10073")
					|| !strcmp(pszObjName, "00_soo_s2_9m_18696")
					|| !strcmp(pszObjName, "00_soo_s2_9m_18777")
					|| !strcmp(pszObjName, "00_soo_s2_11m_18763")
					|| !strcmp(pszObjName, "00_soo_s2_12m_18696")

					|| !strcmp(pszObjName, "00_soo_s3_13m_16989")

					|| !strcmp(pszObjName, "00_soo_y1_7m_12125")
					|| !strcmp(pszObjName, "00_soo_y1_14m_12125"))
				{
					psNodes->pcParentObject->m_fAlphaTestDiffuse = m_fAlphaTestDiffuse ;
				}
			}
			else
				pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE) ;

			psNodes->pcParentObject->SetWireFrame(bWireFrame) ;

			if(m_nAttr & RROLL_ATTR_MIRRORRENDER)
			{
				mat = (*psNodes->pmatWorld) * (*m_pmatMirror) ;
				psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, &mat) ;
			}
			else
				psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, psNodes->pmatWorld) ;

			psNodes->pcParentObject->m_fAlphaTestDiffuse = 0.0f ;

			//psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, psNodes->pmatWorld) ;
		}
	}
	else if(m_nAttr &RROLL_ATTR_FARTONEAR)//알파블랜딩 하는것.
	{
		//one pass rendering
		if((m_nAttr & RROLL_ATTR_BEHINDMIRRORRENDER) || (m_nAttr & RROLL_ATTR_MIRRORRENDER))
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHAREF, psNodes->pcParentObject->m_dwAddAlpha) ;
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
			pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;
			
			D3DXMATRIX mat ;
			for(i=0 ; i<m_nNodeCount ; i++, psNodes++)
			{
				if( (nDistNear != nDistFar) && ((psNodes->fDistfromCamera < (float)nDistNear) || (psNodes->fDistfromCamera > (float)nDistFar)) )
					continue ;

				if(!_RenderTestbySpecificRenderingArea(psNodes))
					continue ;

				psNodes->pcParentObject->SetWireFrame(bWireFrame) ;

				if(m_nAttr & RROLL_ATTR_MIRRORRENDER)
				{
					mat = (*psNodes->pmatWorld) * (*m_pmatMirror) ;
					psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, &mat) ;
				}
				else
					psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, psNodes->pmatWorld) ;
			}
		}
		else//two pass rendering
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
			pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x000000bf) ;//cut off greater that 0.75
			pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE) ;
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO) ;

			for(i=0 ; i<m_nNodeCount ; i++, psNodes++)
			{
				if( (nDistNear != nDistFar) && ((psNodes->fDistfromCamera < (float)nDistNear) || (psNodes->fDistfromCamera > (float)nDistFar)) )
					continue ;

				psNodes->pcParentObject->SetWireFrame(bWireFrame) ;
				psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, psNodes->pmatWorld) ;
			}

			pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000) ;
			pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;

			pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) ;
			pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS) ;

			psNodes = m_psNodes ;
			for(i=0 ; i<m_nNodeCount ; i++, psNodes++)
			{
				if( (nDistNear != nDistFar) && ((psNodes->fDistfromCamera < (float)nDistNear) || (psNodes->fDistfromCamera > (float)nDistFar)) )
					continue ;

				psNodes->pcParentObject->SetWireFrame(bWireFrame) ;
				psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, psNodes->pmatWorld) ;
			}
		}
	}

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable) ;
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, dwZFunc) ;

	pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend) ;
}
void CSecretRenderingRoll::RenderShadow(LPDIRECT3DDEVICE9 pd3dDevice, SD3DEffect *psEffect, CSecretShadowMap *pcShadowMap)
{
	if(!m_nNodeCount)
		return ;

	DWORD dwZEnable, dwZWriteEnable, dwZFunc, dwAlphaTest, dwAlphaBlend, dwCullMode ;

	pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
	pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend) ;

	pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWriteEnable) ;
	pd3dDevice->GetRenderState(D3DRS_ZFUNC, &dwZFunc) ;

	pd3dDevice->GetRenderState(D3DRS_CULLMODE, &dwCullMode) ;

    //Set Render State
	pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000001) ;
	pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
    pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	int i ;
	SRenderingNode *psNodes = m_psNodes ;
	for(i=0 ; i<m_nNodeCount ; i++, psNodes++)
	{
		if(psNodes->nAttr & SRenderingNode::ATTR_SHADOW)
		{
			if(psNodes->pcParentObject->GetKind() == CSecretMeshObject::KIND_BILLBOARD)
				pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;
			else
				pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW) ;

			psNodes->pcParentObject->SetWireFrame(false) ;
			psNodes->pcParentObject->AddAttr(CSecretMeshObject::ATTR_IDSHADOW) ;
			psNodes->pcParentObject->SetShadowMap(pcShadowMap) ;
			psNodes->pcParentObject->Render(psEffect, psNodes->pcMesh, psNodes->pmatWorld) ;
			psNodes->pcParentObject->AddAttr(CSecretMeshObject::ATTR_IDSHADOW, false) ;

			if(strcmp(psNodes->pcParentObject->GetObjName(), "indicator_common"))
				psNodes->pcParentObject->AddAttr(CSecretMeshObject::ATTR_SHADOWRECV) ;
		}
	}

	pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwCullMode) ;

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable) ;
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, dwZFunc) ;

	pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend) ;

}
bool CSecretRenderingRoll::InsertMesh(CSecretMesh *pcMesh, CSecretMeshObject *pcParentObject, D3DXMATRIX *pmatWorld, Vector3 *pvCameraPos, int nAttr)
{
    SRenderingNode *psNode = &m_psNodes[m_nNodeCount++] ;
	assert(m_nNodeCount < MAXNUM_RENDERINGNODE) ;

	psNode->pmatWorld = pmatWorld ;
	psNode->pcMesh = pcMesh ;
	psNode->pcParentObject = pcParentObject ;
	psNode->fDistfromCamera = CalculateDistfromCamera(psNode, pmatWorld, pvCameraPos) ;
	psNode->bEnable = true ;
	psNode->nAttr = nAttr ;

	return true ;
}

void CSecretRenderingRoll::Sorting()
{
    qsort(m_psNodes, m_nNodeCount, sizeof(SRenderingNode), m_pfCompare) ;
}

void CSecretRenderingRoll::Reset()
{
	m_nNodeCount = 0 ;
}

float CSecretRenderingRoll::CalculateDistfromCamera(SRenderingNode *psNode, D3DXMATRIX *pmatWorld, Vector3 *pvCamera)
{
	Vector3 vPos ;
	D3DXMATRIX matTransform ;

	if(psNode->pcParentObject->GetAttr() & CSecretMeshObject::ATTR_ANI)
		matTransform = (*psNode->pcMesh->GetmatTM()) * (*pmatWorld) ;
	else
		matTransform = (*psNode->pcMesh->GetmatLocal()) * (*pmatWorld) ;
	vPos.set(matTransform._41, matTransform._42, matTransform._43) ;

	return (vPos - (*pvCamera)).Magnitude() ;
}
void CSecretRenderingRoll::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
bool CSecretRenderingRoll::IsAttr(int nAttr)
{
	if(m_nAttr & nAttr)
		return true ;
	return false ;
}