#include <assert.h>
#include "SecretMeshObject.h"
#include "SecretDummyMesh.h"
#include "SecretBoneMesh.h"
#include "SecretRigidMesh.h"
#include "SecretRigidMeshBump.h"
#include "SecretRigidMeshAmbO.h"
#include "SecretSkinnedMesh.h"
#include "SecretSkinnedMeshVS.h"
#include "SecretBillboardMesh.h"
#include "SecretMaterial.h"
#include "D3DMathLib.h"
#include "GeoLib.h"
#include "MathOrdinary.h"
#include "SecretMirror.h"
#include "SecretTerrain.h"
#include "SecretShadowMap.h"
#include "SecretWater.h"
#include "SecretUtility.h"
#include <time.h>

using namespace std ;

bool IdenticalNode(void *pvNode1, void *pvNode2)
{
	CSecretNode *pcNode1 = (CSecretNode *)pvNode1 ;
	CSecretNode *pcNode2 = (CSecretNode *)pvNode2 ;

	if(pcNode1->m_lEigenIndex == pcNode2->m_lEigenIndex)
		return true ;

	return false ;
}

void CSecretObjectScatteringData::CalculateShaderParameterObjectOptimized()
{
	//m_shaderParams.vSumBeta1Beta2 *= 0.1f ;
	m_shaderParams.vBetaD1 *= 0.1f ;
	m_shaderParams.vBetaD2 *= 0.1f ;
	//m_shaderParams.vRcpSumBeta1Beta2 *= 15.0f ;
	//m_shaderParams.vTermMultipliers.x *= 1.2f;
	//m_shaderParams.vTermMultipliers.x = 0.86f;

	//m_shaderParams.vTermMultipliers.x = 1.3911f;
	//20100320 수정
	m_shaderParams.vTermMultipliers.x = 1.6f ;

	m_shaderParams.vTermMultipliers.y *= 1.2f;

	m_shaderParams.vSoilReflectivity.z = m_shaderParams.vSoilReflectivity.y ;
}

bool SD3DMaterial::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SMaterial *psMaterial, char *pszPath, CSecretTextureContainer *pcTexContainer)
{
	D3DSURFACE_DESC desc ;
	char str[256] ;

	nAttr = psMaterial->nAttr ;

	//Decal Texture
	this->bUseTexDecal = psMaterial->bUseTexDecal ;
	//sprintf(this->szDecaleName, "%s", psMaterial->szDecaleName) ;
	if(this->bUseTexDecal)
	{
		//pTexDecal = pcTexContainer->FindTexture(psMaterial->szDecaleName) ;
		RemoveExt(str, psMaterial->szDecaleName) ;
		pTexDecal = pcTexContainer->FindTexture(str) ;
		assert(pTexDecal) ;

		pTexDecal->GetLevelDesc(0, &desc) ;
		//TRACE("texture format %d\r\n", desc.Format) ;
		
		if((desc.Format == D3DFMT_A8R8G8B8) || (desc.Format == D3DFMT_DXT5))
			nAttr |= MATERIAL_ATTR_ALPHABLENDING ;
	}

	//NormalMap Texture
	//this->bUseNormalMap = psMaterial->bUseNormalMap ;
	//sprintf(this->szBumpName, "%s", psMaterial->szBumpName) ;
	//if(this->bUseNormalMap)
	//{
	//	pTexBump = pcTexContainer->FindTexture(psMaterial->szBumpName) ;
	//	assert(pTexBump) ;
	//}

	//AmbientMap Texture
	this->bUseAmbientMap = psMaterial->bUseAmbientMap ;
	sprintf(this->szAmbientName, "%s", psMaterial->szAmbientName) ;
	if(this->bUseAmbientMap)
	{
		//write here about AmbientMap
	}

	this->sDiffuse = psMaterial->sDiffuse ;
	this->sAmbient = psMaterial->sAmbient ;
	this->sSpecular = psMaterial->sSpecular ;

	this->nNumSubMaterial = psMaterial->nNumSubMaterial ;
	if(this->nNumSubMaterial > 0)
	{
		this->psSubMaterial = new SD3DMaterial[this->nNumSubMaterial] ;

		int i ;
		for(i=0 ; i<this->nNumSubMaterial ; i++)
			this->psSubMaterial[i].Initialize(pd3dDevice, &psMaterial->psSubMaterial[i], pszPath, pcTexContainer) ;
	}

	return true ;
}
void SD3DMaterial::Release()
{
	int i ;
	for(i=0 ; i<nNumSubMaterial ; i++)
		psSubMaterial[i].Release() ;
	SAFE_DELETEARRAY(psSubMaterial) ;
	nNumSubMaterial = 0 ;

	//if(pTexDecal)
	//{
	//	pTexDecal->Release() ;
	//	pTexDecal = NULL ;
	//}
	//if(pTexBump)
	//{
	//	pTexBump->Release() ;
	//	pTexBump = NULL ;
	//}
}

void CSecretMeshObject::InitMemberVariable(LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex)
{
	m_lEigenIndex = lIndex ;
	m_lAttr = CSecretMeshObject::ATTR_SOLID ;
	m_nNumMesh = 0 ;
	m_pd3dDevice = pd3dDevice ;
	m_lIndexDistribute=0 ;
	m_nNumBone = 0 ;
	m_lRenderOption = MOBJ_RO_RIGID ;
    D3DXMatrixIdentity(&m_matWorld) ;
	m_psMaterial = NULL ;
	m_lSkinningUsage = SKINNINGUSAGE_SOFTWARE ;
	//m_lSkinningUsage = SKINNINGUSAGE_VERTEXSHADER ;
	m_cMaterialNode.resize(0) ;
	m_pcMirror = NULL ;
	m_fOffset = 0 ;
	m_pcAtm = NULL ;
	m_pmatSkin = NULL ;
	m_fObjectHeight = 0.0f ;
	m_fAlphaTestDiffuse = 0.0f ;
	m_fWaveLeafTimeScale = 2.0f ;
	m_fWaveLeafFrequency = 10.0f ;
	m_fWaveLeafAmplitude = 0.8f ;
}

CSecretMeshObject::CSecretMeshObject()
{
	m_lAttr = CSecretMeshObject::ATTR_SOLID ;
	m_nNumMesh = 0 ;
	m_lIndexDistribute=0 ;
	m_nNumBone = 0 ;
	m_lRenderOption = MOBJ_RO_RIGID ;
    D3DXMatrixIdentity(&m_matWorld) ;
	m_psMaterial = NULL ;
	m_lSkinningUsage = SKINNINGUSAGE_SOFTWARE ;
	//m_lSkinningUsage = SKINNINGUSAGE_VERTEXSHADER ;
	m_cMaterialNode.resize(0) ;
	m_pcMirror = NULL ;
	m_fOffset = 0 ;
	m_pcAtm = NULL ;
	m_pmatSkin = NULL ;
	sprintf(m_szObjName, "") ;
	sprintf(m_szCatalogName, "") ;
	m_fObjectHeight = 0.0f ;

	ZeroMemory(m_anTemporary, sizeof(int)*16) ;
}

CSecretMeshObject::CSecretMeshObject(LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex)
{
	InitMemberVariable(pd3dDevice, lIndex) ;
}
CSecretMeshObject::CSecretMeshObject(LPDIRECT3DDEVICE9 pd3dDevice, CASEData *pcASEData, char *pszPath, CSecretTextureContainer *pcTexContainer, UINT lIndex)
{
	InitMemberVariable(pd3dDevice, lIndex) ;
	InitObject(pcASEData, pszPath, pcTexContainer) ;
}

CSecretMeshObject::~CSecretMeshObject()
{
	Release() ;
}

bool CSecretMeshObject::InitObject(CASEData *pcASEData, char *pszPath, CSecretTextureContainer *pcTexContainer, UINT lAddNodeAttr)
{
	if(!pcASEData)
		return false ;

	int i ;
    int nMaterialID, nSubMaterialID ;
	CSecretMesh *pcMesh ;

	m_lAttr = pcASEData->m_lAttr ;
	m_lSubAttr = pcASEData->m_lSubAttr ;

	m_sAnimationInfo.fFirstFrame = pcASEData->m_sSceneInfo.fFirstFrame ;
	m_sAnimationInfo.fLastFrame = pcASEData->m_sSceneInfo.fLastFrame ;
	m_sAnimationInfo.fFrameSpeed = pcASEData->m_sSceneInfo.fFrameSpeed ;
	m_sAnimationInfo.fTicksPerFrame = pcASEData->m_sSceneInfo.fTicksPerFrame ;
	m_sAnimationInfo.fTotalFrame = m_sAnimationInfo.fLastFrame * m_sAnimationInfo.fTicksPerFrame ;

	m_sAnimationInfo.fFrameSpeed = 20.0f ;

	SetAddAlpha(pcASEData->m_szFileName) ;
	SetBillboardAttr(pcASEData->m_szFileName) ;

	m_vMin.set(pcASEData->m_vMin.x, pcASEData->m_vMin.y, pcASEData->m_vMin.z) ;
	m_vMax.set(pcASEData->m_vMax.x, pcASEData->m_vMax.y, pcASEData->m_vMax.z) ;

	m_fObjectHeight = float_abs(m_vMax.y - m_vMin.y) ;

	//give the attribute of the reflected
	if(m_fObjectHeight > 100.0f)//7m 이상 되는 오브젝트는 반사되어짐.
		m_lAttr |= ATTR_REFLECTED ;

	if(strstr(pcASEData->m_szFileName, "05_kko"))
		int wifwf=0 ;

	////////////////////////////////////////////////////////////////////////////////////
	//Material 재질도 참조리스트로 만들어 둔다.
	////////////////////////////////////////////////////////////////////////////////////
	m_nNumMaterial = pcASEData->m_nNumMaterial ;
	if(m_nNumMaterial)
	{
		m_psMaterial = new SD3DMaterial[m_nNumMaterial] ;

		for(i=0 ; i<m_nNumMaterial ; i++)
			m_psMaterial[i].Initialize(m_pd3dDevice, &pcASEData->m_psMaterial[i], pszPath, pcTexContainer) ;
	}

	CSecretNode *pcHead = new CSecretNode(m_lIndexDistribute++) ;
	CSecretNode *pcTail = new CSecretNode(m_lIndexDistribute++) ;
	//m_cMeshNode.AllocateArrangedBuffer(MAXNUM_MESH) ;
	m_cMeshNode.Initialize(pcHead, pcTail, IdenticalNode) ;

	m_pmatSkin = new D3DXMATRIX[pcASEData->m_nNumBone] ;

	for(i=0 ; i<pcASEData->m_nNumBone ; i++)
	{
		m_apszBoneName[i] = new char[256] ;
		sprintf(m_apszBoneName[i], "%s", pcASEData->m_apszBoneName[i]) ;
	}

	unsigned long lVertexCount=0, lTriangleCount=0 ;

	m_nNumMesh = pcASEData->m_nNumMesh ;
	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		//pMesh = new CSecretMesh(m_lIndexDistribute++) ;
		//if(pcASEData->m_apMesh[i]->nAttr & MESHDATA_ATTR_USETEX)// 속성값에 따라서 해당하는 메쉬를 할당하도록 해야 된다.

		//pMesh->Initialize(pcASEData->m_apMesh[i]) ;

		pcASEData->m_apMesh[i]->nAttr |= lAddNodeAttr ;//속성을 추가한다.

		if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_BONE)
		{
			CSecretBoneMesh *pBoneMesh = new CSecretBoneMesh(m_lIndexDistribute++, m_pd3dDevice) ;
			pBoneMesh->Initialize(pcASEData->m_apMesh[i]) ;

			lVertexCount += pBoneMesh->GetNumVertex() ;
			lTriangleCount += pBoneMesh->GetNumTriangle() ;

			pBoneMesh->SetWireFrame(true) ;

			if(pBoneMesh->m_bParent)//이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
			{
				CSecretNode *pcParentNode = (CSecretNode *)(m_cMeshNode.FindNode(pBoneMesh->GetParentID())->pvData) ;
				assert((pcParentNode != NULL) && "Cannot FindNode in CSecretMeshObject::InitObject BoneMesh") ;
				pBoneMesh->SetParentNode(pcParentNode) ;

				//TRACE("###SET BoneMesh parent mesh node=%s parent=%s\r\n", pBoneMesh->GetNodeName(), pBoneMesh->GetParentNode()->GetNodeName()) ;
			}

			pcMesh = (CSecretMesh *)pBoneMesh ;
			m_cMeshNode.InsertNode((void *)pBoneMesh) ;

			m_nNumBone++ ;
		}
		else if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_SKINNING)
		{
			if(m_lSkinningUsage == SKINNINGUSAGE_SOFTWARE)//소프트웨어로 스키닝
			{
				CSecretSkinnedMesh *pcSkinnedMesh = new CSecretSkinnedMesh(m_lIndexDistribute++, m_pd3dDevice) ;
				pcSkinnedMesh->Initialize(pcASEData->m_apMesh[i]) ;

				if(pcSkinnedMesh->m_bParent)////이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
				{
					CSecretNode *pcParentNode = (CSecretNode *)(m_cMeshNode.FindNode(pcSkinnedMesh->GetParentID())->pvData) ;
					assert((pcParentNode != NULL) && "Cannot FindNode in CSecretMeshObject::InitObject SkinnedMesh") ;
					pcSkinnedMesh->SetParentNode(pcParentNode) ;

					//TRACE("###SET SkinnedMesh parent mesh node=%s parent=%s\r\n", pcSkinnedMesh->GetNodeName(), pcSkinnedMesh->GetParentNode()->GetNodeName()) ;
				}

				//pRigidMesh->SetWireFrame(true) ;
				pcMesh = (CSecretMesh *)pcSkinnedMesh ;
				m_cMeshNode.InsertNode((void *)pcSkinnedMesh) ;
			}
			else if(m_lSkinningUsage == SKINNINGUSAGE_VERTEXSHADER)//버텍스쉐이더로 스키닝
			{
				CSecretSkinnedMeshVS *pcSkinnedMeshVS = new CSecretSkinnedMeshVS(m_lIndexDistribute++, m_pd3dDevice) ;
				pcSkinnedMeshVS->Initialize(pcASEData->m_apMesh[i]) ;

				if(pcSkinnedMeshVS->m_bParent)////이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
				{
					CSecretNode *pcParentNode = (CSecretNode *)(m_cMeshNode.FindNode(pcSkinnedMeshVS->GetParentID())->pvData) ;
					assert((pcParentNode != NULL) && "Cannot FindNode in CSecretMeshObject::InitObject SkinnedMesh") ;
					pcSkinnedMeshVS->SetParentNode(pcParentNode) ;

					//TRACE("###SET SkinnedMeshVS parent mesh node=%s parent=%s\r\n", pcSkinnedMeshVS->GetNodeName(), pcSkinnedMeshVS->GetParentNode()->GetNodeName()) ;
				}

				//pRigidMesh->SetWireFrame(true) ;
                pcMesh = (CSecretMesh *)pcSkinnedMeshVS ;
				m_cMeshNode.InsertNode((void *)pcSkinnedMeshVS) ;
			}
		}
		else if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_RIGID || pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_XBILLBOARD)//일반적인 오브젝트에 해당
		{
            CSecretRigidMesh *pRigidMesh ;

			if(pcASEData->m_apMesh[i]->nAttr & SMESHATTR_USENORMALMAP)
			{
				CSecretRigidMeshBump *pRigidMeshBump = new CSecretRigidMeshBump(m_lIndexDistribute++, m_pd3dDevice) ;
				pRigidMeshBump->Initialize(pcASEData->m_apMesh[i]) ;
				pRigidMesh = (CSecretRigidMesh *)pRigidMeshBump ;
			}
			else if (pcASEData->m_apMesh[i]->nAttr & SMESHATTR_USEAMBIENTDEGREE)
			{
				CSecretRigidMeshAmbO *pRigidMeshAmbO = new CSecretRigidMeshAmbO(m_lIndexDistribute++, m_pd3dDevice) ;
				pRigidMeshAmbO->Initialize(pcASEData->m_apMesh[i]) ;
				pRigidMesh = (CSecretRigidMesh *)pRigidMeshAmbO ;
			}
			else
			{
				pRigidMesh = new CSecretRigidMesh(m_lIndexDistribute++, m_pd3dDevice) ;
				pRigidMesh->Initialize(pcASEData->m_apMesh[i]) ;
			}

			if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_XBILLBOARD)//십자로된 빌보드는 라이팅 하지 않는다.
				pRigidMesh->AddAttr(SECRETNODEATTR_NOLIGHTING) ;

			if(m_lAttr & CSecretMeshObject::ATTR_REFLECTED)//반사속성은 메쉬클래스에도 적용.
				pRigidMesh->AddAttr(SECRETNODEATTR_REFLECTED) ;

			lVertexCount += pRigidMesh->GetNumVertex() ;
			lTriangleCount += pRigidMesh->GetNumTriangle() ;

			if(pRigidMesh->m_bParent)////이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
			{
				CSecretNode *pcParentNode = (CSecretNode *)(m_cMeshNode.FindNode(pRigidMesh->GetParentID())->pvData) ;
				assert((pcParentNode != NULL) && "Cannot FindNode in CSecretMeshObject::InitObject RigidMesh") ;
				pRigidMesh->SetParentNode(pcParentNode) ;

				//TRACE("###SET RigidMesh parent mesh node=%s parent=%s\r\n", pRigidMesh->GetNodeName(), pRigidMesh->GetParentNode()->GetNodeName()) ;
			}

			//pRigidMesh->SetWireFrame(true) ;
			pcMesh = (CSecretMesh *)pRigidMesh ;
			m_cMeshNode.InsertNode((void *)pRigidMesh) ;
		}
		/*
		else if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_RIGIDBUMP)
		{
			CSecretRigidMeshBump *pRigidMesh = new CSecretRigidMeshBump(m_lIndexDistribute++, m_pd3dDevice) ;
			pRigidMesh->Initialize(pcASEData->m_apMesh[i]) ;

			lVertexCount += pRigidMesh->GetNumVertex() ;
			lTriangleCount += pRigidMesh->GetNumTriangle() ;

			if(pRigidMesh->m_bParent)////이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
			{
				CSecretNode *pcParentNode = (CSecretNode *)(m_cMeshNode.FindNode(pRigidMesh->GetParentID())->pvData) ;
				assert((pcParentNode != NULL) && "Cannot FindNode in CSecretMeshObject::InitObject RigidMesh") ;
				pRigidMesh->SetParentNode(pcParentNode) ;

				TRACE("###SET RigidMesh parent mesh node=%s parent=%s\r\n", pRigidMesh->GetNodeName(), pRigidMesh->GetParentNode()->GetNodeName()) ;
			}

			//pRigidMesh->SetWireFrame(true) ;
			m_cMeshNode.InsertNode((void *)pRigidMesh) ;
		}
		*/
		else if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_DUMMY)//HelperObject 경우 해당, 에니메이션메쉬에서 사용된다.
		{
            CSecretDummyMesh *pDummyMesh = new CSecretDummyMesh(m_lIndexDistribute++, m_pd3dDevice) ;
			pDummyMesh->Initialize(pcASEData->m_apMesh[i]) ;

			m_pmatDummy = pDummyMesh->GetmatLocal() ;

			pcMesh = (CSecretMesh *)pDummyMesh ;
			m_cMeshNode.InsertNode((void *)pDummyMesh) ;
		}
		else if(pcASEData->m_apMesh[i]->MeshKind == SMESHKIND_BILLBOARD)
		{
			//TRACE("init Billboard Mesh name=%s\r\n", pcASEData->m_szFileName) ;

			CSecretBillboardMesh *pBillboardMesh = new CSecretBillboardMesh(m_lIndexDistribute++, m_pd3dDevice) ;

			pBillboardMesh->Initialize(pcASEData->m_apMesh[i]) ;
			//pBillboardMesh->SetAddAlpha(pcASEData->m_szFileName) ;
			pBillboardMesh->SetBillboardDiffuse(pcASEData->m_szFileName) ;

			pcMesh = (CSecretMesh *)pBillboardMesh ;
			m_cMeshNode.InsertNode((void *)pBillboardMesh) ;
		}

		//It will find mesh attribute whether using alphablending or not
		//마타리알 정보에서 알파블랜딩을 하는 메쉬인지 찾아낸다.

		nMaterialID = pcMesh->GetMaterialID() ;

		if((pcMesh->GetAttr() & SECRETNODEATTR_USESUBMATERIAL) && (pcMesh->GetNodeKind() == SECRETNODEKIND_RIGID))//RigidMesh 인 경우만 서브마타리알을 사용한다.
		{
			int bAlphaBlending=false, bNotUseSubMaterial=false, bTwosided=false ;
			//int nNumSubset = ((CSecretRigidMesh *)pcMesh)->GetNumSubSet() ;
			for(int n=0 ; n<((CSecretRigidMesh *)pcMesh)->GetNumSubSet() ; n++)
			{
				nSubMaterialID = ((CSecretRigidMesh *)pcMesh)->GetSubMaterialID(n) ;
				if(m_psMaterial[nMaterialID].nNumSubMaterial > 0)
				{
					if(m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].nAttr & MATERIAL_ATTR_ALPHABLENDING)
						bAlphaBlending = true ;

					if(m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].nAttr & MATERIAL_ATTR_TWOSIDED)
						bTwosided = true ;
				}
				else
					bNotUseSubMaterial = true ;//서브마타리알 속성인데 마타리알리스트에서 서브가 없다면 그 속성은 삭제한다.
			}

			if(bAlphaBlending)
				pcMesh->AddAttr(SECRETNODEATTR_ALPHABLENDING) ;
			if(bNotUseSubMaterial)
				pcMesh->RemoveAttr(SECRETNODEATTR_USESUBMATERIAL) ;

			if(bTwosided)
			{
				pcMesh->AddAttr(SECRETNODEATTR_TWOSIDED) ;

				float limited_angle=5.0f, speed=0.1f, initial_angle=0.0f ;

				static unsigned long lTime=(unsigned long)time(NULL) ;
				srand(lTime++) ;

				limited_angle += float_random(2.0f, true) ;
				speed += float_random(0.5f, false) ;
				initial_angle = float_random(8.0f, true) ;

				if(rand()%2)
					speed = -speed ;

				if(float_abs(initial_angle) >= 4.0f)
						initial_angle = initial_angle ;

				((CSecretRigidMesh *)pcMesh)->m_sLeafAni.Reset(limited_angle, speed, initial_angle) ;
			}
			else
			{
				//if(pcMesh->GetKind() == SECRETNODEKIND_RIGID)
				//	((CSecretRigidMesh *)pcMesh)->BuildBoundingModel() ;
			}

			//nSubMaterialID = ((CSecretRigidMesh *)pcMesh)->GetSubMaterialID(0) ;
			//if(m_psMaterial[nMaterialID].nNumSubMaterial > 0)
			//{
			//	if(m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].nAttr & MATERIAL_ATTR_ALPHABLENDING)
			//		pcMesh->AddAttr(SECRETNODEATTR_ALPHABLENDING) ;
			//}
			//else
			//	pcMesh->RemoveAttr(SECRETNODEATTR_USESUBMATERIAL) ;//서브마타리알 속성인데 마타리알리스트에서 서브가 없다면 그 속성은 삭제한다.
		}
		else
		{
			if(m_psMaterial[nMaterialID].nAttr & MATERIAL_ATTR_ALPHABLENDING)
			{
				pcMesh->AddAttr(SECRETNODEATTR_ALPHABLENDING) ;

				if(pcMesh->GetKind() == SECRETNODEKIND_RIGID)
				{
					float limited_angle=2.0f, speed=0.1f, initial_angle=0.0f ;

					static unsigned long lTime=(unsigned long)time(NULL) ;
					srand(lTime++) ;

					limited_angle += float_random(2.0f, true) ;
					speed += float_random(0.5f, false) ;
					initial_angle = float_random(8.0f, true) ;

					if(rand()%2)
						speed = -speed ;

                    if(float_abs(initial_angle) >= 4.0f)
						initial_angle = initial_angle ;

					((CSecretRigidMesh *)pcMesh)->m_sLeafAni.Reset(limited_angle, speed, initial_angle) ;
				}
			}
			else
			{
				//if(pcMesh->GetKind() == SECRETNODEKIND_RIGID)
				//	((CSecretRigidMesh *)pcMesh)->BuildBoundingModel() ;
			}
		}

		pcASEData->m_apMesh[i]->nAttr &= (~lAddNodeAttr) ;//속성다쉬 해제

		//노말맵을 일단 제외시킨다.
		DWORD dwAttr = pcMesh->GetAttr() ;
		dwAttr &= (~SECRETNODEATTR_USENORMALMAP) ;
		//dwAttr &= (~SECRETNODEATTR_ALPHABLENDING) ;
		pcMesh->SetAttr(dwAttr) ;
	}

	assert(m_nNumBone == pcASEData->m_nNumBone) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//계층구조를 구축한다. 에니메이션메쉬(Skinning Mesh)일경우 뼈대구조로만 계층을 구성해서 에니메이션에서 사용된다.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	pcHead = new CSecretNode(m_lIndexDistribute++) ;
	pcTail = new CSecretNode(m_lIndexDistribute++) ;
    
	//m_cHierarchyMeshList.AllocateArrangedBuffer(MAXNUM_MESH) ;
	m_cHierarchyMeshList.Initialize(pcHead, pcTail, IdenticalNode) ;

	if(m_lAttr & CSecretMeshObject::ATTR_ANI)
	{
		DNode *pTail = NULL ;
		DNode *pNode = NULL ;
		CSecretNode *pcNode = NULL ;
		int nCount=0 ;

		if(m_lSubAttr & CSecretMeshObject::SUBATTR_ANI_KEYFRAME)
		{
			pTail = m_cMeshNode.GetTail() ;
			pNode = m_cMeshNode.GetHead()->pNext ;

			while(pNode != pTail)
			{
				pcNode = (CSecretNode *)pNode->pvData ;

				m_cHierarchyMeshList.InsertNode(pNode->pvData) ;

				//TRACE("insert[%d] Node:%s ParentNode:%s NodeKind=%d\r\n",
				//	nCount++, ((CSecretNode *)pNode->pvData)->GetNodeName(), ((CSecretNode *)pNode->pvData)->GetParentNodeName(), pcNode->GetNodeKind()) ;

				pNode = pNode->pNext ;
			}
		}
		else if(m_lSubAttr & CSecretMeshObject::SUBATTR_ANI_SKINNED)
		{
			bool bHierarchy=false ;

			if(pcASEData->m_bUseDummyObject)//맥스에서 HELPEROBJECT를 사용하는 경우 Dummy
			{
				pTail = m_cMeshNode.GetTail() ;
				pNode = m_cMeshNode.GetHead()->pNext ;

				while(pNode != pTail)
				{
					pcNode = (CSecretNode *)pNode->pvData ;
					if(pcNode->GetNodeKind() == SECRETNODEKIND_DUMMY)
						bHierarchy = true ;

					if(bHierarchy)
					{
						m_cHierarchyMeshList.InsertNode(pNode->pvData) ;

						//TRACE("insert[%d] Node:%s ParentNode:%s\r\n",
						//	nCount++, ((CSecretNode *)pNode->pvData)->GetNodeName(), ((CSecretNode *)pNode->pvData)->GetParentNodeName()) ;
					}

					pNode = pNode->pNext ;
				}
			}
			else
			{
				pTail = m_cMeshNode.GetTail() ;
				pNode = m_cMeshNode.GetHead()->pNext ;

				while(pNode != pTail)
				{
					pcNode = (CSecretNode *)pNode->pvData ;
					if(pcNode->GetNodeKind() == SECRETNODEKIND_BONE || pcNode->GetNodeKind() == SECRETNODEKIND_DUMMY)
					{
						m_cHierarchyMeshList.InsertNode(pNode->pvData) ;

						//TRACE("insert[%d] Node:%s ParentNode:%s NodeKind=%d\r\n",
						//	nCount++, ((CSecretNode *)pNode->pvData)->GetNodeName(), ((CSecretNode *)pNode->pvData)->GetParentNodeName(), pcNode->GetNodeKind()) ;
					}

					pNode = pNode->pNext ;
				}
			}
		}
	}

	//디버깅을 위한 LocalSystem을 만들어둔다.
	m_cLocalSystem.Initialize(m_pd3dDevice) ;

	//PreComputedAmbientOcclusionPerVertex(pcASEData) ;

    //TRACE("character Object NumVertex=%d NumTriangle=%d\r\n", lVertexCount, lTriangleCount) ;

	return true ;
}

void CSecretMeshObject::SetElapsedTime(float ftime)
{
	m_fElapsedTime = ftime ;
}

float CSecretMeshObject::GetElapsedTime()
{
	return m_fElapsedTime ;
}

void CSecretMeshObject::_TimerAnimation(float fCurTime)
{
	m_fCurTime = (fCurTime * (m_sAnimationInfo.fFrameSpeed * m_sAnimationInfo.fTicksPerFrame)) ;
	m_fTimeGap = m_fCurTime-m_fOldTime ;

	m_fAniTime += m_fTimeGap ;

	float fLastTime = m_sAnimationInfo.fTicksPerFrame*m_sAnimationInfo.fLastFrame ;
	float fFirstTime = m_sAnimationInfo.fTicksPerFrame*m_sAnimationInfo.fFirstFrame ;

	if(m_fAniTime > fLastTime)
		m_fAniTime -= (fLastTime-fFirstTime) ;

	m_fOldTime = m_fCurTime ;
}

void CSecretMeshObject::Animation(float fFrame)
{
	DNode *pTail = m_cHierarchyMeshList.GetTail() ;
	DNode *pNode = m_cHierarchyMeshList.GetHead()->pNext ;
	CSecretMesh *pcMesh, *pcParentMesh ;
	D3DXMATRIX *pmatParentTM, matIdentity ;
	int nSkinCount=0 ;

	D3DXMatrixIdentity(&matIdentity) ;

	_TimerAnimation(fFrame) ;

	while(pNode != pTail)
	{
		pcMesh = (CSecretMesh *)pNode->pvData ;

		if(pcMesh->m_bParent)
		{
			pcParentMesh = (CSecretMesh *)pcMesh->GetParentNode() ;
			pmatParentTM = pcParentMesh->GetmatTM() ;
			pcMesh->Animation(m_fAniTime, pmatParentTM) ;
		}
		else
		{
			pcMesh->Animation(m_fAniTime, &matIdentity) ;
		}

		if(pcMesh->GetNodeKind() == SECRETNODEKIND_BONE)//스키닝행렬 빠레뜨를 만들고 있다
		{
			m_pmatSkin[nSkinCount] = *pcMesh->GetmatSkin() ;
			nSkinCount++ ;
		}

		pNode = pNode->pNext ;
	}

	assert(nSkinCount == m_nNumBone) ;
}

CSecretMesh *CSecretMeshObject::GetMesh(int nIndex)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pMeshNode ;
	int nCount=0 ;

	while(pNode != pTail)
	{
		pMeshNode = (CSecretNode *)pNode->pvData ;
        if(nCount == nIndex)
			return (CSecretMesh *)pMeshNode ;

		pNode = pNode->pNext ;
		nCount++ ;
	}

	return NULL ;
}

void CSecretMeshObject::_SetEigenIndexforIdShadow(CSecretNode *pcMeshNode, D3DXVECTOR4 &vId)
{
	UINT lr, lg, lb, la ;
	float fBias = 16.0f ;

	//Object EigenIndex
	la = (this->m_lEigenIndex & 0xff00)>>8 ;
	lr = (this->m_lEigenIndex & 0x00ff) ;

	//Mesh EigenIndex of Object
	lg = (pcMeshNode->m_lEigenIndex & 0xff00)>>8 ;
	lb = (pcMeshNode->m_lEigenIndex & 0x00ff) ;

	vId.x = (float)la/fBias ;
	vId.y = (float)lr/fBias ;
	vId.z = (float)lg/fBias ;
	vId.w = (float)lb/fBias ;

	//TRACE("EigenIndex=%03d lr=%03d vId.y=%3.3f\r\n", this->m_lEigenIndex, lr, vId.y) ;
}
/*
void CSecretMeshObject::RenderIdShadow(SD3DEffect *psEffect, CSecretIdShadow *pcIdShadow)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pcMeshNode ;
	D3DXMATRIX matWorld, matWL, matInv, matVP ;
	D3DXVECTOR4 vId ;

	while(pNode != pTail)
	{
		pcMeshNode = (CSecretNode *)pNode->pvData ;

		matWorld = (*((CSecretMesh *)pcMeshNode)->GetmatTM()) * m_matWorld ;

		psEffect->pEffect->SetMatrix(psEffect->hmatWIT, &matWorld) ;

		matWL = matWorld * (*pcIdShadow->GetmatLV()) ;
		psEffect->pEffect->SetMatrix(psEffect->hmatWL, &matWL) ;

        //_SetEigenIndexforIdShadow(pcMeshNode, vId) ;
		//psEffect->pEffect->SetVector(psEffect->hvID, &vId) ;
		int anObjectID[2] = {this->m_lEigenIndex, pcMeshNode->m_lEigenIndex} ;
		psEffect->pEffect->SetIntArray(psEffect->hanObjectID, anObjectID, 2) ;

//		if(pcMeshNode->GetNodeKind() == SECRETNODEKIND_RIGID)
//			((CSecretRigidMesh *)pcMeshNode)->RenderPerspectiveShadowMap(matWorld, psEffect->matView, pcIdShadow) ;

		pNode = pNode->pNext ;
	}

	m_lAttr |= ATTR_IDSHADOW ;
	m_pcShadowMap = pcIdShadow ;
}
*/
void CSecretMeshObject::Render(D3DXMATRIX *pmatWorld)//use FixedPipeLine
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pMeshNode ;
	D3DXMATRIX matWorld ;

	while(pNode != pTail)
	{
		pMeshNode = (CSecretNode *)pNode->pvData ;

		if(pMeshNode->GetMaterialID() == -1)
			m_pd3dDevice->SetTexture(0, NULL) ;

		//matWorld = (*((CSecretMesh *)pMeshNode)->GetmatTM()) * m_matWorld ;
		D3DXMATRIX *pmatTM = ((CSecretMesh *)pMeshNode)->GetmatLocal() ;//RigidMesh 이기 때문에 로칼행렬
		matWorld = (*pmatTM) * (*pmatWorld) ;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld) ;

		if((pMeshNode->GetNodeKind() == SECRETNODEKIND_RIGID) || (pMeshNode->GetNodeKind() == SECRETNODEKIND_SKINNING))
		{
			if(m_lRenderOption & MOBJ_RO_RIGID)
			{
				((CSecretMesh *)pMeshNode)->SetWireFrame(m_bWireFrame) ;
				pMeshNode->Render() ;
			}
		}
        if(pMeshNode->GetNodeKind() == SECRETNODEKIND_BONE)
		{
			if(m_lRenderOption & MOBJ_RO_BONE)
				pMeshNode->Render() ;
		}

		pNode = pNode->pNext ;
	}

	if(m_lRenderOption & MOBJ_RO_LOCALSYS)
	{
		D3DXMatrixIdentity(&matWorld) ;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld) ;
		m_cLocalSystem.Render() ;
	}
}

void CSecretMeshObject::RenderNormalVector(D3DXMATRIX *pmatWorld)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pMeshNode ;
	D3DXMATRIX matWorld, matIdentity ;

	while(pNode != pTail)
	{
		pMeshNode = (CSecretNode *)pNode->pvData ;

		matWorld = (*((CSecretMesh *)pMeshNode)->GetmatLocal()) * (*pmatWorld) ;

		D3DXMatrixIdentity(&matIdentity) ;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &matIdentity) ;

		if((pMeshNode->GetNodeKind() == SECRETNODEKIND_RIGID) || (pMeshNode->GetNodeKind() == SECRETNODEKIND_SKINNING))
		{
			((CSecretMesh *)pMeshNode)->RenderNormalVector(MatrixConvert(matWorld)) ;
		}

		pNode = pNode->pNext ;
	}
}

void CSecretMeshObject::RenderCollisionVolume(D3DXMATRIX *pmatWorld)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pMeshNode ;
	D3DXMATRIX matWorld, matIdentity ;

	while(pNode != pTail)
	{
		pMeshNode = (CSecretNode *)pNode->pvData ;

		matWorld = (*((CSecretMesh *)pMeshNode)->GetmatLocal()) * (*pmatWorld) ;

		D3DXMatrixIdentity(&matIdentity) ;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &matIdentity) ;

		if((pMeshNode->GetNodeKind() == SECRETNODEKIND_RIGID) || (pMeshNode->GetNodeKind() == SECRETNODEKIND_SKINNING))
		{
			((CSecretMesh *)pMeshNode)->RenderBoundingVolume(MatrixConvert(matWorld)) ;
		}

		pNode = pNode->pNext ;
	}
}

void CSecretMeshObject::Render(SD3DEffect *psEffect, D3DXMATRIX *pmatWorld)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pMeshNode ;
	D3DXMATRIX matWorld, matIdentity ;

	while(pNode != pTail)
	{
		pMeshNode = (CSecretNode *)pNode->pvData ;

        Render(psEffect, (CSecretMesh *)pMeshNode, pmatWorld) ;

		pNode = pNode->pNext ;
	}
}
void CSecretMeshObject::Render(SD3DEffect *psEffect, CSecretMesh *pcMeshNode, D3DXMATRIX *pmatWorld)
{
	HRESULT hr ;
	D3DXMATRIX mat, matWorld, *pmatTM ;
	D3DXVECTOR4 vLocalLightPos, vLocalLightDir, vLocalLookAt, vDiffuse, vLocalCameraPos ;
	SD3DMaterial *psMaterial ;
	int nMaterialID ;

	m_cLocalSystem.SetMatrix(m_matWorld) ;

	if(pcMeshNode->GetNodeKind() == SECRETNODEKIND_RIGID)
	{
		if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( ((CSecretMesh *)pcMeshNode)->m_pVertexElement, &psEffect->pDecl )))
			assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretMeshObject::Render(SD3DEffect *psEffect)") ;

		if(m_lAttr & ATTR_IDMAP)
		{
            psEffect->pEffect->BeginPass(CSecretMeshObject::PASS_IDMAP) ;
		}
		else if(m_lAttr & ATTR_IDSHADOW)
		{
			psEffect->pEffect->BeginPass(CSecretMeshObject::PASS_IDSHADOW) ;
		}
		else
		{
			unsigned long lAttr = pcMeshNode->GetAttr() ;
			if(lAttr & SECRETNODEATTR_SUNLIT)//Sunlit를 사용하는 것과
			{
				if(lAttr & SECRETNODEATTR_NOLIGHTING)
				{
					if(m_lAttr & ATTR_BILLBOARD_NOMIPMAPFILTER)
					{
						psEffect->pEffect->BeginPass(PASS_SIZZLING) ;
					}
					else
					{
						psEffect->pEffect->BeginPass(PASS_NOLIGHTINGSUNLIT) ;
					}
				}
				else if(lAttr & SECRETNODEATTR_TWOSIDED)
				{
					psEffect->pEffect->BeginPass(PASS_TWOSIDEDSUNLIT) ;
				}
				else if(lAttr & SECRETNODEATTR_NOTEXTURE)
				{
					psEffect->pEffect->BeginPass(PASS_NOTEXTURE) ;
				}
				else
				{
					psEffect->pEffect->BeginPass(PASS_SUNLIT) ;
				}

				psEffect->pEffect->SetValue(psEffect->hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;
			}
			else//그렇지 않는 것으로 나눈다.
			{
				if(lAttr & SECRETNODEATTR_USEAMBIENTDEGREE)
					psEffect->pEffect->BeginPass(PASS_USEAMBIENT) ;
				else if(lAttr & SECRETNODEATTR_USENORMALMAP)
					psEffect->pEffect->BeginPass(PASS_BUMP) ;
				else
					psEffect->pEffect->BeginPass(PASS_USETEX) ;
			}
		}

		//pmatTM = ((CSecretMesh *)pcMeshNode)->GetmatTM() ;
		pmatTM = ((CSecretMesh *)pcMeshNode)->GetmatLocal() ;//RigidMesh 이기 때문에 로칼행렬
		matWorld = (*pmatTM) * (*pmatWorld) ;

		D3DXMatrixInverse(&mat, NULL, &matWorld) ;

		D3DXVec4Transform(&vLocalLightPos, &psEffect->vLightPos, &mat) ;
		psEffect->pEffect->SetVector(psEffect->hvLightPos, &vLocalLightPos) ;

		psEffect->pEffect->SetVector(psEffect->hvLightColor, &psEffect->vLightColor) ;

		D3DXVec4Transform(&vLocalLightDir, &psEffect->vLightDir, &mat) ;
		D3DXVec4Normalize(&vLocalLightDir, &vLocalLightDir) ;
		//vLocalLightDir = psEffect->vLightDir ;
		psEffect->pEffect->SetVector(psEffect->hvLightDir, &vLocalLightDir) ;

		D3DXVec4Transform(&vLocalLookAt, &psEffect->vLookAt, &mat) ;
		D3DXVec4Normalize(&vLocalLookAt, &vLocalLookAt) ;
		psEffect->pEffect->SetVector(psEffect->hvLookAt, &vLocalLookAt) ;

		//카메라를 로칼좌표계안으로
		D3DXVec4Transform(&vLocalCameraPos, &psEffect->vCameraPos, &mat) ;
		psEffect->pEffect->SetVector(psEffect->hvCameraPos, &vLocalCameraPos) ;

		//mat = matWorld * psEffect->matView * psEffect->matProj ;
		mat = matWorld * psEffect->matWorld * psEffect->matView * psEffect->matProj ;
		psEffect->pEffect->SetMatrix(psEffect->hmatWVP, &mat) ;
		psEffect->pEffect->SetMatrix(psEffect->hmatWIT, &matWorld) ;

		psEffect->pEffect->SetMatrix(psEffect->hmatTexCoordAni, &((CSecretRigidMesh *)pcMeshNode)->m_sLeafAni.matAni) ;

		D3DXVECTOR3 vDistFromCamera(psEffect->vCameraPos.x-pmatWorld->_41, psEffect->vCameraPos.y-pmatWorld->_42, psEffect->vCameraPos.z-pmatWorld->_43) ;
		float fDistFromCamera = D3DXVec3Length(&vDistFromCamera) ;
		psEffect->pEffect->SetFloat(psEffect->hfDistFromCamera, fDistFromCamera) ;

		psEffect->pEffect->SetFloat(psEffect->hfAlphaBlendingByForce, pcMeshNode->m_fAlphaBlendingByForce) ;

		float fElapsedTime=0.0f ;
		if(m_lAttr & ATTR_WAVINGLEAF)
			fElapsedTime = psEffect->fElapsedTime ;
		psEffect->pEffect->SetFloat(psEffect->hfElapsedTime, fElapsedTime) ;

		psEffect->pEffect->SetFloat(psEffect->hfTimeScale, m_fWaveLeafTimeScale) ;
		psEffect->pEffect->SetFloat(psEffect->hfFrequency, m_fWaveLeafFrequency) ;
		psEffect->pEffect->SetFloat(psEffect->hfAmplitude, m_fWaveLeafAmplitude) ;

		//텍스쳐 애니메이션
		//m_fOffset += m_fElapsedTime/40.0f ;
		//if(m_fOffset > 1.0f)
		//{
		//	m_fOffset -= 1.0f ;
		//}

		//TRACE("texOffset=%g elapsedtime=%g\r\n", m_fOffset, m_fElapsedTime) ;

		//D3DXMATRIX matTexAni ;
		//D3DXMatrixIdentity(&matTexAni) ;
		//matTexAni._41 = m_fOffset ;
		//psEffect->pEffect->SetMatrix(psEffect->hmatTexCoordAni, &matTexAni) ;

		if(m_lAttr & CSecretMeshObject::ATTR_IDMAP)
		{
			psEffect->pEffect->SetInt(psEffect->hnEigenIndex, m_anTemporary[0]) ;//Set EigenIndex
		}

		if(m_lAttr & CSecretMeshObject::ATTR_IDSHADOW)
		{
			D3DXMATRIX matWLP ;
			matWLP = matWorld * (*m_pcShadowMap->GetmatLVP()) ;
			psEffect->pEffect->SetMatrix(psEffect->hmatWLP, &matWLP) ;

			int anObjectID[2] = {this->m_lEigenIndex, pcMeshNode->m_lEigenIndex} ;
			psEffect->pEffect->SetIntArray(psEffect->hanObjectID, anObjectID, 2) ;
		}

		if(m_lAttr & CSecretMeshObject::ATTR_SHADOWRECV)
		{
			float fOffsetX = 0.5f+(0.5f/(float)m_pcShadowMap->GetRenderTarget()->lWidth) ;
			float fOffsetY = 0.5f+(0.5f/(float)m_pcShadowMap->GetRenderTarget()->lHeight) ;

			//투영변환후에 텍스쳐공간으로 변환하기 위한 행렬, Offset값으로 오차를 조정한다.
			D3DXMATRIX matTexBias(0.5f,	    0.0f,	   0.0f,	   0.0f,
								  0.0f,    -0.5f,      0.0f,       0.0f,
								  0.0f,     0.0f,      0.0f,       0.0f,
								  fOffsetX, fOffsetY,  0.0f,       1.0f) ;

			//D3DXVECTOR4 vId ;
			//_SetEigenIndexforIdShadow(pcMeshNode, vId) ;
			//psEffect->pEffect->SetVector(psEffect->hvID, &vId) ;
			//int anObjectID[2] = {this->m_lEigenIndex, pcMeshNode->m_lEigenIndex} ;
			//psEffect->pEffect->SetIntArray(psEffect->hanObjectID, anObjectID, 2) ;

			D3DXMATRIX matWLP, matWLPT ;

			matWLP = matWorld * (*m_pcShadowMap->GetmatLVP()) ;
			psEffect->pEffect->SetMatrix(psEffect->hmatWLP, &matWLP) ;
			matWLPT = matWLP * matTexBias ;
			psEffect->pEffect->SetMatrix(psEffect->hmatWLPT, &matWLPT) ;
			psEffect->pEffect->SetFloat(psEffect->hfEpsilon, m_pcShadowMap->m_fShaderEpsilon) ;

			SRenderTarget *psRenderTarget = m_pcShadowMap->GetRenderTarget() ;
			psEffect->pEffect->SetTexture("ShadowMap", psRenderTarget->pTex) ;
			psEffect->pEffect->SetTexture("SrcShadowMap", m_pcShadowMap->GetTexSoft()) ;
		}
		else
		{
			psEffect->pEffect->SetFloat(psEffect->hfEpsilon, 1.0f) ;
		}

		//여기의 분기문은 Rigid_Mesh로 제한한다.
		//SubMaterial을 사용하는 경우 Material별로 소팅해서 랜더링을 한다
		if(pcMeshNode->GetAttr() & SECRETNODEATTR_USESUBMATERIAL)
		{
			int nSubMaterialID ;
			CSecretRigidMesh *pcRigidMesh = (CSecretRigidMesh *)pcMeshNode ;

			pcRigidMesh->PreRenderSubSet() ;

			nMaterialID = pcRigidMesh->GetMaterialID() ;

			for(int i=0 ; i<pcRigidMesh->GetNumSubSet() ; i++)
			{
				nSubMaterialID = pcRigidMesh->GetSubMaterialID(i) ;
				if((nSubMaterialID >= 0) && (nSubMaterialID < m_psMaterial[nMaterialID].nNumSubMaterial))
				{
					psMaterial = &m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID] ;

					vDiffuse.x = psMaterial->sDiffuse.r ;
					vDiffuse.y = psMaterial->sDiffuse.g ;
					vDiffuse.z = psMaterial->sDiffuse.b ;
					vDiffuse.w = psMaterial->sDiffuse.a ;

					psEffect->pEffect->SetVector(psEffect->hvDiffuse, &pcRigidMesh->m_vDiffuse) ;
					//psEffect->pEffect->SetVector(psEffect->hvDiffuse, &psEffect->vDiffuse) ;

					psEffect->pEffect->SetFloat(psEffect->hfAlphaTestDiffuse, m_fAlphaTestDiffuse) ;

					assert(m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].pTexDecal) ;

					if(pcMeshNode->GetAttr() & SECRETNODEATTR_NOLIGHTING)//xbillboard
					{
						if(m_lAttr & ATTR_IDSHADOW)//그림자 렌더링일 경우는 무조건 decalmap으로
							psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].pTexDecal) ;
						else
						{
							if(m_lAttr & ATTR_BILLBOARD_NOMIPMAPFILTER)
								psEffect->pEffect->SetTexture("NoMipmapFilterMap", m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].pTexDecal) ;
							else
								psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].pTexDecal) ;
						}
					}
					else
						psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].psSubMaterial[nSubMaterialID].pTexDecal) ;

					DWORD dwCullMode=0 ;
					if(psMaterial->nAttr & MATERIAL_ATTR_TWOSIDED)
					{
						m_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &dwCullMode) ;
						m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;
					}

					pcRigidMesh->SetWireFrame(m_bWireFrame) ;

					m_pd3dDevice->SetVertexDeclaration(psEffect->pDecl) ;
					psEffect->pEffect->CommitChanges() ;
					pcRigidMesh->RenderSubSet(i) ;

					if(psMaterial->nAttr & MATERIAL_ATTR_TWOSIDED)
						m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwCullMode) ;
				}
			}
			pcRigidMesh->PostRenderSubSet() ;

		}
		else//SubMaterial 을 사용하지 않고 하나의 Material을 사용하는 경우
		{
			DWORD dwCullMode=0 ;

			if(m_lAttr & ATTR_MIRRORMAPPING)//반사맵핑 사용
			{
				vDiffuse.x = vDiffuse.y = vDiffuse.z = vDiffuse.w = 1.0f ;
				psEffect->pEffect->SetVector(psEffect->hvDiffuse, &vDiffuse) ;

				psEffect->pEffect->SetTexture("DecalMap", m_pcMirror->GetTexReflect()) ;
				((CSecretMesh *)pcMeshNode)->SetWireFrame(m_bWireFrame) ;
			}
			else
			{
				nMaterialID = pcMeshNode->GetMaterialID() ;
				if(nMaterialID >= 0)
				{
					psMaterial = &m_psMaterial[nMaterialID] ;

					vDiffuse.x = 1 ;// psMaterial->sDiffuse.r ;
					vDiffuse.y = 1 ;// psMaterial->sDiffuse.g ;
					vDiffuse.z = 1 ;// psMaterial->sDiffuse.b ;
					vDiffuse.w = 1 ;// psMaterial->sDiffuse.a ;

					psEffect->pEffect->SetVector(psEffect->hvDiffuse, &pcMeshNode->m_vDiffuse) ;
					//psEffect->pEffect->SetVector(psEffect->hvDiffuse, &psEffect->vDiffuse) ;

					psEffect->pEffect->SetFloat(psEffect->hfAlphaTestDiffuse, m_fAlphaTestDiffuse) ;

					if(m_psMaterial[nMaterialID].bUseTexDecal)
					{
						if(pcMeshNode->GetAttr() & SECRETNODEATTR_NOLIGHTING)//xbillboard
						{
							if(m_lAttr & ATTR_BILLBOARD_NOMIPMAPFILTER)
								psEffect->pEffect->SetTexture("NoMipmapFilterMap", m_psMaterial[nMaterialID].pTexDecal) ;
							else
								psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].pTexDecal) ;
						}
						else
							psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].pTexDecal) ;
					}

					if(m_psMaterial[nMaterialID].bUseNormalMap)
						psEffect->pEffect->SetTexture("NormalMap", m_psMaterial[nMaterialID].pTexBump) ;

					if(psMaterial->nAttr & MATERIAL_ATTR_TWOSIDED)
					{
						m_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &dwCullMode) ;
						m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;
					}
				}

				((CSecretMesh *)pcMeshNode)->SetWireFrame(m_bWireFrame) ;
			}

			m_pd3dDevice->SetVertexDeclaration(psEffect->pDecl) ;
			psEffect->pEffect->CommitChanges() ;
			pcMeshNode->Render() ;

			if(psMaterial->nAttr & MATERIAL_ATTR_TWOSIDED)
				m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwCullMode) ;
		}

		psEffect->pDecl->Release() ;
		psEffect->pDecl = NULL ;

		psEffect->pEffect->EndPass() ;
	}
	else if(pcMeshNode->GetNodeKind() == SECRETNODEKIND_SKINNING)
	{
		//셰이더에서 사용하는 버텍스 설정

		if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( ((CSecretMesh *)pcMeshNode)->m_pVertexElement, &psEffect->pDecl )))
			assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretMeshObject::Render(SD3DEffect *psEffect)") ;

		if(m_lSkinningUsage == SKINNINGUSAGE_SOFTWARE)//소프트웨어 스키닝
		{
			//			mat = matWorld * psEffect->matView * psEffect->matProj ;
			//			psEffect->pEffect->SetMatrix(psEffect->hmatWVP, &mat) ;
			//			psEffect->pEffect->SetMatrix(psEffect->hmatWIT, &matWorld) ;

			psEffect->pEffect->BeginPass(PASS_USETEX) ;

			pmatTM = ((CSecretMesh *)pcMeshNode)->GetmatTM() ;
			matWorld = (*pmatTM) * (*pmatWorld) ;

			D3DXMatrixInverse(&mat, NULL, &matWorld) ;
			D3DXVec4Transform(&vLocalLightPos, &psEffect->vLightPos, &mat) ;
			psEffect->pEffect->SetVector(psEffect->hvLightPos, &vLocalLightPos) ;

			D3DXVec4Transform(&vLocalCameraPos, &psEffect->vCameraPos, &mat) ;
			psEffect->pEffect->SetVector(psEffect->hvCameraPos, &vLocalCameraPos) ;

			D3DXVec4Transform(&vLocalLightDir, &psEffect->vLightDir, &mat) ;
			D3DXVec4Normalize(&vLocalLightDir, &vLocalLightDir) ;
			psEffect->pEffect->SetVector(psEffect->hvLightDir, &vLocalLightDir) ;

			mat = matWorld * psEffect->matView * psEffect->matProj ;
			psEffect->pEffect->SetMatrix(psEffect->hmatWVP, &mat) ;
            //RigidMesh처럼 이것도 월드좌표 추가 되어야 됨.
			psEffect->pEffect->SetMatrix(psEffect->hmatWIT, &matWorld) ;

			nMaterialID = pcMeshNode->GetMaterialID() ;
			if(nMaterialID >= 0)
			{
				psMaterial = &m_psMaterial[nMaterialID] ;

				vDiffuse.x = psMaterial->sDiffuse.r ;
				vDiffuse.y = psMaterial->sDiffuse.g ;
				vDiffuse.z = psMaterial->sDiffuse.b ;
				vDiffuse.w = psMaterial->sDiffuse.a ;

				psEffect->pEffect->SetVector(psEffect->hvDiffuse, &pcMeshNode->m_vDiffuse) ;

				psEffect->pEffect->SetFloat(psEffect->hfAlphaTestDiffuse, m_fAlphaTestDiffuse) ;

				if(m_psMaterial[nMaterialID].bUseTexDecal)
					psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].pTexDecal) ;
			}

			((CSecretMesh *)pcMeshNode)->SetWireFrame(m_bWireFrame) ;

			((CSecretSkinnedMesh *)pcMeshNode)->SetMatrixPaletteSW(m_pmatSkin, m_pmatDummy, m_apszBoneName) ;
		}
		else if(m_lSkinningUsage == SKINNINGUSAGE_VERTEXSHADER)//셰이더 스키닝
		{
			psEffect->pEffect->BeginPass(PASS_SKINNING) ;

			pmatTM = ((CSecretMesh *)pcMeshNode)->GetmatTM() ;
			matWorld = (*pmatTM) * (*pmatWorld) ;

			D3DXMatrixInverse(&mat, NULL, &matWorld) ;
			D3DXVec4Transform(&vLocalLightPos, &psEffect->vLightPos, &mat) ;
			psEffect->pEffect->SetVector(psEffect->hvLightPos, &vLocalLightPos) ;

			D3DXVec4Transform(&vLocalCameraPos, &psEffect->vCameraPos, &mat) ;
			psEffect->pEffect->SetVector(psEffect->hvCameraPos, &vLocalCameraPos) ;

			D3DXVec4Transform(&vLocalLightDir, &psEffect->vLightDir, &mat) ;
			D3DXVec4Normalize(&vLocalLightDir, &vLocalLightDir) ;
			psEffect->pEffect->SetVector(psEffect->hvLightDir, &vLocalLightDir) ;

			CSecretSkinnedMesh *pcSkinnedMesh = (CSecretSkinnedMesh *)pcMeshNode ;

			mat = matWorld * psEffect->matView * psEffect->matProj ;
			psEffect->pEffect->SetMatrix(psEffect->hmatWVP, &mat) ;

            //RigidMesh처럼 이것도 월드좌표 추가 되어야 됨.
			psEffect->pEffect->SetMatrix(psEffect->hmatWIT, pcSkinnedMesh->GetmatLocal()) ;

			nMaterialID = pcMeshNode->GetMaterialID() ;
			if(nMaterialID >= 0)
			{
				psMaterial = &m_psMaterial[nMaterialID] ;

				vDiffuse.x = psMaterial->sDiffuse.r ;
				vDiffuse.y = psMaterial->sDiffuse.g ;
				vDiffuse.z = psMaterial->sDiffuse.b ;
				vDiffuse.w = psMaterial->sDiffuse.a ;

				psEffect->pEffect->SetVector(psEffect->hvDiffuse, &pcMeshNode->m_vDiffuse) ;

				psEffect->pEffect->SetFloat(psEffect->hfAlphaTestDiffuse, m_fAlphaTestDiffuse) ;

				if(m_psMaterial[nMaterialID].bUseTexDecal)
					psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].pTexDecal) ;
			}

			((CSecretMesh *)pcMeshNode)->SetWireFrame(m_bWireFrame) ;
		}

		m_pd3dDevice->SetVertexDeclaration(psEffect->pDecl) ;
		psEffect->pEffect->CommitChanges() ;
		pcMeshNode->Render() ;

		psEffect->pDecl->Release() ;
		psEffect->pDecl = NULL ;

		psEffect->pEffect->EndPass() ;
	}
	if(pcMeshNode->GetNodeKind() == SECRETNODEKIND_BILLBOARD)
	{
		if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( ((CSecretMesh *)pcMeshNode)->m_pVertexElement, &psEffect->pDecl )))
			assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretMeshObject::Render(SD3DEffect *psEffect)") ;

		D3DXMATRIX matS, matT, matR, matST ;//빌보드는 회전행렬을 빼고 해야 된다.
		D3DXMatrixDecomposeTranslation(&matT, pmatWorld) ;
		D3DXMatrixDecomposeRotation(&matR, pmatWorld) ;
		D3DXMatrixDecomposeScaling(&matS, pmatWorld) ;
		matST = matS*matT ;

		//psEffect->pEffect->BeginPass(PASS_BILLBOARD) ;
		//psEffect->pEffect->SetValue(psEffect->hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;

		if(m_lAttr & ATTR_IDSHADOW)
			psEffect->pEffect->BeginPass(CSecretMeshObject::PASS_BILLBOARDSHADOW) ;
		else
		{
			if(m_lAttr & ATTR_BILLBOARD_NOMIPMAPFILTER)
				psEffect->pEffect->BeginPass(PASS_BILLBOARDNOMIPMAPFILTER) ;
			else
				psEffect->pEffect->BeginPass(PASS_BILLBOARD) ;

			psEffect->pEffect->SetValue(psEffect->hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;
		}

		if(m_lAttr & CSecretMeshObject::ATTR_IDSHADOW)
		{
			D3DXMATRIX matWLP ;
			matWLP = matST * (*m_pcShadowMap->GetmatLVP()) ;
			psEffect->pEffect->SetMatrix(psEffect->hmatWLP, &matWLP) ;

			int anObjectID[2] = {this->m_lEigenIndex, pcMeshNode->m_lEigenIndex} ;
			psEffect->pEffect->SetIntArray(psEffect->hanObjectID, anObjectID, 2) ;
		}

		//pmatTM = ((CSecretMesh *)pcMeshNode)->GetmatLocal() ;//RigidMesh 이기 때문에 로칼행렬
		//matWorld = (*pmatTM) * (*pmatWorld) ;

		D3DXMatrixInverse(&mat, NULL, pmatWorld) ;

		D3DXVec4Transform(&vLocalLightPos, &psEffect->vLightPos, &mat) ;
		psEffect->pEffect->SetVector(psEffect->hvLightPos, &vLocalLightPos) ;

		psEffect->pEffect->SetVector(psEffect->hvLightColor, &psEffect->vLightColor) ;

		D3DXVec4Transform(&vLocalLightDir, &psEffect->vLightDir, &mat) ;
		D3DXVec4Normalize(&vLocalLightDir, &vLocalLightDir) ;
		//vLocalLightDir = psEffect->vLightDir ;
		psEffect->pEffect->SetVector(psEffect->hvLightDir, &vLocalLightDir) ;

		D3DXVec4Transform(&vLocalLookAt, &psEffect->vLookAt, &mat) ;
		D3DXVec4Normalize(&vLocalLookAt, &vLocalLookAt) ;
		psEffect->pEffect->SetVector(psEffect->hvLookAt, &vLocalLookAt) ;

		D3DXVec4Transform(&vLocalCameraPos, &psEffect->vCameraPos, &mat) ;//카메라위치 로칼좌표
		//vLocalCameraPos = psEffect->vCameraPos ;//카메라위치는 월드좌표
		psEffect->pEffect->SetVector(psEffect->hvCameraPos, &vLocalCameraPos) ;

		//BillboardDiffuse
		//psEffect->pEffect->SetFloat(psEffect->hfBillboardDiffuse, ((CSecretBillboardMesh *)pcMeshNode)->m_fBillboardDiffuse) ;

		D3DXVECTOR4 vec ;
		D3DXMatrixIdentity(&mat) ;

		//set veiw right
		vec = D3DXVECTOR4(psEffect->matView._11, psEffect->matView._21, psEffect->matView._31, 0.0f) ;
		D3DXVec4Transform(&vec, &vec, &matST) ;//로칼좌표안으로 들어감.
		D3DXVec4Normalize(&vec, &vec) ;
		mat._11 = vec.x ;
		mat._12 = vec.y ;
		mat._13 = vec.z ;
		mat._14 = vec.w ;
        
		//set object up
		//vec = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.0f) ;
		vec = D3DXVECTOR4(matR._21, matR._22, matR._23, 0.0f) ;
		D3DXVec4Transform(&vec, &vec, &matST) ;//로칼좌표안으로 들어감.
		D3DXVec4Normalize(&vec, &vec) ;
		mat._21 = vec.x ;
		mat._22 = vec.y ;
		mat._23 = vec.z ;
		mat._24 = vec.w ;

		//set inv veiw dir
		vec = D3DXVECTOR4(-psEffect->matView._13, -psEffect->matView._23, -psEffect->matView._33, 0.0f) ;
		D3DXVec4Transform(&vec, &vec, &matST) ;//로칼좌표안으로 들어감.
		D3DXVec4Normalize(&vec, &vec) ;
		mat._31 = vec.x ;
		mat._32 = vec.y ;
		mat._33 = vec.z ;
		mat._34 = vec.w ;

		psEffect->pEffect->SetMatrixArray(psEffect->hmatSkin, &mat, 1) ;

		//mat = (*pmatWorld) * psEffect->matView * psEffect->matProj ;
        mat = matST * psEffect->matView * psEffect->matProj ;
		psEffect->pEffect->SetMatrix(psEffect->hmatWVP, &mat) ;
		psEffect->pEffect->SetMatrix(psEffect->hmatWIT, pmatWorld) ;

		D3DXVECTOR3 vDistFromCamera(psEffect->vCameraPos.x-pmatWorld->_41, psEffect->vCameraPos.y-pmatWorld->_42, psEffect->vCameraPos.z-pmatWorld->_43) ;
		float fDistFromCamera = D3DXVec3Length(&vDistFromCamera) ;
		psEffect->pEffect->SetFloat(psEffect->hfDistFromCamera, fDistFromCamera) ;

		nMaterialID = pcMeshNode->GetMaterialID() ;
		if(nMaterialID >= 0)
		{
			psMaterial = &m_psMaterial[nMaterialID] ;

			vDiffuse.x = 1 ;// psMaterial->sDiffuse.r ;
			vDiffuse.y = 1 ;// psMaterial->sDiffuse.g ;
			vDiffuse.z = 1 ;// psMaterial->sDiffuse.b ;
			vDiffuse.w = 1 ;// psMaterial->sDiffuse.a ;

			psEffect->pEffect->SetVector(psEffect->hvDiffuse, &pcMeshNode->m_vDiffuse) ;

			psEffect->pEffect->SetFloat(psEffect->hfAlphaTestDiffuse, m_fAlphaTestDiffuse) ;

			if(m_psMaterial[nMaterialID].bUseTexDecal)
			{
				if((m_lAttr & ATTR_IDSHADOW) || (m_lAttr & ATTR_BILLBOARD_NOMIPMAPFILTER))
					psEffect->pEffect->SetTexture("NoMipmapFilterMap", m_psMaterial[nMaterialID].pTexDecal) ;
				else
					psEffect->pEffect->SetTexture("DecalMap", m_psMaterial[nMaterialID].pTexDecal) ;
			}
		}

		((CSecretMesh *)pcMeshNode)->SetWireFrame(m_bWireFrame) ;

		m_pd3dDevice->SetVertexDeclaration(psEffect->pDecl) ;
		psEffect->pEffect->CommitChanges() ;
		pcMeshNode->Render() ;

		psEffect->pDecl->Release() ;
		psEffect->pDecl = NULL ;

		psEffect->pEffect->EndPass() ;
	}
}

void CSecretMeshObject::RenderVertexDisk()
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretNode *pMeshNode ;
	D3DXMATRIX matWorld, matIdentity ;

	while(pNode != pTail)
	{
		pMeshNode = (CSecretNode *)pNode->pvData ;

		matWorld = (*((CSecretMesh *)pMeshNode)->GetmatTM()) * m_matWorld ;

//		D3DXMatrixIdentity(&matIdentity) ;
//		m_pd3dDevice->SetTransform(D3DTS_WORLD, &matIdentity) ;

		if((pMeshNode->GetNodeKind() == SECRETNODEKIND_RIGID) || (pMeshNode->GetNodeKind() == SECRETNODEKIND_SKINNING))
		{
			((CSecretMesh *)pMeshNode)->RenderVertexDisk(MatrixConvert(matWorld)) ;
		}

		pNode = pNode->pNext ;
	}
}

//메쉬순서대로 버텍스를 가져온다
void CSecretMeshObject::GetVertexDisks(std::vector<SVertexDisk *> *pVertexDisks)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pMesh ;
	SVertexDisk *psVertexDisk ;

	while(pNode != pTail)
	{
		pMesh = (CSecretMesh *)pNode->pvData ;
		psVertexDisk = pMesh->GetVertexDisks() ;
		for(UINT i=0 ; i<pMesh->GetNumVertex() ; i++)
		{
			pVertexDisks->push_back(&psVertexDisk[i]) ;
		}

		pNode = pNode->pNext ;
	}
}

void CSecretMeshObject::PreComputedAmbientOcclusionPerVertex()
{
	std::vector<SVertexDisk *> VertexDisks ;
	VertexDisks.resize(0) ;
    GetVertexDisks(&VertexDisks) ;

    int recv, emt ;
	Vector3 vDist ;
	float ftemp, fSolidAngle ;

	for(recv=0 ; recv<(int)VertexDisks.size() ; recv++)
	{
		fSolidAngle = 0 ;
        for(emt=0 ; emt<(int)VertexDisks.size() ; emt++)
		{
			vDist = VertexDisks[emt]->vPos - VertexDisks[recv]->vPos ;
			if(vDist.Magnitude() > 0)
			{
				if(VertexDisks[recv]->vNormal.dot(vDist.Normalize()) > 0)//다른 디스크가 현재 디스크의 노말방향의 앞쪽에 있다면
				{
					//F:\Study\3D 관련자료\Grobal Illumination\AmbientOcclusion\DynamicAmbientOcclusion_OPerez.pdf 참조
					//입체각 공식 2pi(1-cos(theta))
					//theta = atan(EmtterRadius/Distance)

					ftemp = 2.0f*3.141592f*(1.0f - cosf(atanf(VertexDisks[emt]->fRadius/vDist.Magnitude()))) ;
					fSolidAngle += ftemp ;
					//TRACE("[emt=%2d] SolidAngle=%3.3f vDist(%3.3f, %3.3f, %3.3f) Radius(emt)=%3.3f\r\n", emt, ftemp, enumVector(vDist), VertexDisks[emt]->fRadius) ;

					//ftemp = (VertexDisks[emt]->fRadius * (vDist.Normalize().dot(VertexDisks[emt]->vNormal))) / (vDist.Magnitude()*vDist.Magnitude()) ;
					//fRadiosity += ftemp ;
					//TRACE("Radiosity=%3.3f vDist(%3.3f, %3.3f, %3.3f) Radius(emt)=%3.3f\r\n", ftemp, enumVector(vDist), VertexDisks[emt]->fRadius) ;
				}
			}
		}

		VertexDisks[recv]->fAmbientDegree = 1.0f - (fSolidAngle/(2.0f*3.141592f)) ;
		//TRACE("[recv=%2d]Total TotalSolidAngle=%3.3f ratio=%3.3f\r\n", recv, fSolidAngle, VertexDisks[recv]->fAmbientDegree) ;
	}
}

void CSecretMeshObject::SetVertexDisks(std::vector<SVertexDisk *> *pVertexDisks)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pMesh ;
	SVertexDisk *psVertexDisk ;
	int nDiskCount=0 ;

	while(pNode != pTail)
	{
		pMesh = (CSecretMesh *)pNode->pvData ;
		psVertexDisk = pMesh->GetVertexDisks() ;
		pNode = pNode->pNext ;
	}
}

void CSecretMeshObject::Release()
{
	if(m_pmatSkin)
	{
		delete []m_pmatSkin ;
		m_pmatSkin = NULL ;
	}

    int i ;
	for(i=0 ; i<m_nNumBone ; i++)
	{
		if(m_apszBoneName[i])
		{
			delete []m_apszBoneName[i] ;
			m_apszBoneName[i] = NULL ;
		}
	}

	DNode *pNode, *pTail ;
	CSecretNode *pMeshNode ;
	//#########################//
    //Release HierarchyMeshList//
	//#########################//
	//if(!m_cHierarchyMeshList.m_bDeleteAll)
	if(m_cHierarchyMeshList.IsEnable())
	{
		SAFE_DELETE(m_cHierarchyMeshList.GetHead()->pvData) ;
		SAFE_DELETE(m_cHierarchyMeshList.GetTail()->pvData) ;
		m_cHierarchyMeshList.DeleteAll() ;
	}

    //Release MeshNodeList
	//if(!m_cMeshNode.m_bDeleteAll)
	if(m_cMeshNode.IsEnable())
	{
		pTail = m_cMeshNode.GetTail() ;
		pNode = m_cMeshNode.GetHead()->pNext ;

		while(pNode != pTail)
		{
			pMeshNode = (CSecretNode *)pNode->pvData ;
			delete pMeshNode ;
			pNode = pNode->pNext ;
		}
		SAFE_DELETE(m_cMeshNode.GetHead()->pvData) ;
		SAFE_DELETE(m_cMeshNode.GetTail()->pvData) ;
		m_cMeshNode.DeleteAll() ;
	}

	if(m_psMaterial)
	{
		int i ;
		for(i=0 ; i<m_nNumMaterial ; i++)
			m_psMaterial[i].Release() ;
		SAFE_DELETEARRAY(m_psMaterial) ;
	}

	//if(m_psMaterial)
	//{
	//	int i, n ;

	//	for(i=0 ; i<m_nNumMaterial ; i++)
	//	{
	//		for(n=0 ; n<m_psMaterial[i].nNumSubMaterial ; n++)
	//		{
	//			delete []m_psMaterial[i].psSubMaterial ;
	//			m_psMaterial[i].psSubMaterial = NULL ;
	//		}
	//	}

	//	delete []m_psMaterial ;
	//	m_psMaterial = NULL ;
	//}

	SAFE_DELETE(m_pcMirror) ;
}


void CSecretMeshObject::SetRenderOption(UINT lRenderOption)
{
	m_lRenderOption = lRenderOption ;
}

void CSecretMeshObject::SetmatWorld(D3DXMATRIX &matWorld)
{
	m_matWorld = matWorld ;
}
void CSecretMeshObject::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}

void CSecretMeshObject::AttachMirror(D3DXVECTOR3 &vMirrorPos, D3DXVECTOR3 &vNormal, float fMirrorSize)
{
	Matrix4 matRotation ;
	//matRotation.SetRotatebyAxisX(90) ;

	m_pcMirror = new CSecretMirror(VectorConvert(vMirrorPos), VectorConvert(vNormal), fMirrorSize, fMirrorSize, matRotation, m_pd3dDevice,  true, true) ;
	m_lAttr |= ATTR_MIRRORMAPPING ;
}

void CSecretMeshObject::ProcessMirror(D3DXVECTOR3 &vCameraPos, SD3DEffect *psEffect, CSecretTerrain *pcTerrain)
{
	if(!(m_lAttr & ATTR_MIRRORMAPPING))
		return ;

	m_pcMirror->Process(VectorConvert(vCameraPos)) ;
	m_pcMirror->RenderVirtualCamera(m_pcMirror->GetmatReflectView(), m_pcMirror->GetmatReflectProj(), m_pcMirror->GetTexReflect(), psEffect, pcTerrain) ;
}

void CSecretMeshObject::RenderMirrorPlane()
{
	if(m_pcMirror)
		m_pcMirror->Render() ;
}

//ReflectMapping은 RigidMesh이면서 SubMaterial을 사용하지 않는 것만 사용될수 있다.
//현재는 (0, 1, 0)에 대한 반사표면만 구현되고 있다.
void CSecretMeshObject::MirrorMapping()
{
    DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretRigidMesh *pcRigidMesh ;
	SRigidMesh_Vertex *pVertices ;
	D3DXMATRIX matWorld, matTransform, matInvMirrorTransform, *pmatTM ;
	CSecretMirror *pcMirror = m_pcMirror ;
	UINT i ;

	float fdaltaX, fdaltaZ, fdistX, fdistZ ;
	D3DXVECTOR4 v4Pos ;
	D3DXVECTOR3 vPos ;
	//미러맵은 좌우반전이 되어 있어서 right로 시작됨.
	D3DXVECTOR3 vPoslt = VectorConvert(pcMirror->GetPos(CSecretMirror::RIGHTTOP)) ;//VectorConvert(pcMirror->GetPos(CSecretMirror::LEFTTOP)) ;//월드변환후 좌표임.
	D3DXVECTOR3 vPosrb = VectorConvert(pcMirror->GetPos(CSecretMirror::RIGHTBOTTOM)) ;//월드변환후 좌표임.

	D3DXMatrixInverse(&matInvMirrorTransform, NULL, pcMirror->GetmatMirrorTransform()) ;

	Vector3 vStart, vEnd ;

	vStart = pcMirror->GetPos(CSecretMirror::RIGHTTOP) ;
	vEnd = pcMirror->GetPos(CSecretMirror::LEFTTOP) ;
	fdistX = (vStart-vEnd).Magnitude() ;

	vStart = pcMirror->GetPos(CSecretMirror::RIGHTBOTTOM) ;
	vEnd = pcMirror->GetPos(CSecretMirror::RIGHTTOP) ;
	fdistZ = (vStart-vEnd).Magnitude() ;

	//TRACE("### Reflection Mapping TextureCoordinate ###\r\n") ;

	while(pNode != pTail)
	{
		pcRigidMesh = (CSecretRigidMesh *)pNode->pvData ;
		if(pcRigidMesh->GetNodeKind() == SECRETNODEKIND_RIGID)
		{
			pmatTM = pcRigidMesh->GetmatTM() ;
			matWorld = (*pmatTM) * m_matWorld ;
            matTransform = matWorld * matInvMirrorTransform ;

			pVertices = pcRigidMesh->GetVertices() ;

			for(i=0 ; i<pcRigidMesh->GetNumVertex() ; i++)
			{
				vPos = pVertices->pos ;
				D3DXVec3Transform(&v4Pos, &vPos, &matTransform) ;

				fdaltaX = v4Pos.x - vPoslt.x ;
				fdaltaZ = v4Pos.z - vPoslt.z ;

				pVertices->t.x = -fdaltaX/fdistX ;
				pVertices->t.y = -fdaltaZ/fdistZ ;

				//static count=0 ;
				//TRACE("[%2d] tex(%3.3f %3.3f)\r\n", count++, pVertices->t.x, pVertices->t.y) ;

				pVertices++ ;
			}

			if(pcRigidMesh->GetAttr() & SECRETNODEATTR_USESUBMATERIAL)
			{
				UINT lAttr = pcRigidMesh->GetAttr() & (~SECRETNODEATTR_USESUBMATERIAL) ;
				pcRigidMesh->SetAttr(lAttr) ;
			}

			//pcRigidMesh->SetMaterialID(0) ;
			//SD3DMaterial *psMaterial = GetMaterial(pcRigidMesh->GetMaterialID()) ;
			//memset(psMaterial, 0, sizeof(SD3DMaterial)) ;
			//psMaterial->bUseTexDecal = true ;
			//psMaterial->pTexDecal = pcMirror->GetTexReflect() ;

			pcRigidMesh->RefreashVertexBuffer() ;
		}

		pNode = pNode->pNext ;
	}
}

//#include "SecretOctTree.h"
//
//void CSecretMeshObject::ApplyOctTree(CASEData *pcASEData)
//{
//	SRigidMesh_Vertex *psVertices ;
//	STrnPolygon *psPolygons ;
//
//	int i, n ;
//	int nNumVertex=0, nNumTriangle=0 ;
//	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
//	{
//		nNumVertex += pcASEData->m_apMesh[i]->nNumVertex ;
//		nNumTriangle += pcASEData->m_apMesh[i]->nNumTriangle ;
//	}
//
//	psVertices = new SRigidMesh_Vertex[nNumVertex] ;
//	psPolygons = new STrnPolygon[nNumTriangle] ;
//
//	Matrix4 matLocal ;
//	Vector3 vPos ;
//	int nVertexCount=0, nTriangleCount=0 ;
//	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
//	{
//		memcpy(&matLocal, &pcASEData->m_apMesh[i]->smatLocal, sizeof(Matrix4)) ;
//		for(n=0 ; n<pcASEData->m_apMesh[i]->nNumVertex ; n++)
//		{
//			vPos.set(pcASEData->m_apMesh[i]->psVertex[n].pos.x,	pcASEData->m_apMesh[i]->psVertex[n].pos.y, pcASEData->m_apMesh[i]->psVertex[n].pos.z) ;
//			vPos *= matLocal ;
//
//			psVertices[nVertexCount].pos = D3DXVECTOR3(vPos.x, vPos.y, vPos.z) ;
//			psVertices[nVertexCount].normal = D3DXVECTOR3(pcASEData->m_apMesh[i]->psVertex[n].normal.x, pcASEData->m_apMesh[i]->psVertex[n].normal.y, pcASEData->m_apMesh[i]->psVertex[n].normal.z) ;
//			psVertices[nVertexCount].t = D3DXVECTOR2(pcASEData->m_apMesh[i]->psVertex[n].tex.u, pcASEData->m_apMesh[i]->psVertex[n].tex.v) ;
//			nVertexCount++ ;
//		}
//
//		for(n=0 ; n<pcASEData->m_apMesh[i]->nNumTriangle ; n++)
//		{
//			psPolygons[nTriangleCount].alIndex[0] = pcASEData->m_apMesh[i]->psTriangle[n].anVertexIndex[0] ;
//			psPolygons[nTriangleCount].alIndex[1] = pcASEData->m_apMesh[i]->psTriangle[n].anVertexIndex[1] ;
//			psPolygons[nTriangleCount].alIndex[2] = pcASEData->m_apMesh[i]->psTriangle[n].anVertexIndex[2] ;
//			nTriangleCount++ ;
//		}
//	}
//
//	CSecretOctTree *pcOctTree = new CSecretOctTree(Vector3(0, 0, 0), Vector3(8, 8, 8)) ;
//	pcOctTree->BuildTree(psVertices, psPolygons, nNumTriangle) ;
//
//	SAFE_DELETE(pcOctTree) ;
//	SAFE_DELETEARRAY(psVertices) ;
//	SAFE_DELETEARRAY(psPolygons) ;
//
//	int end ;
//
//	end=0 ;
//
//}

DWORD ExportHeightMapThreadProc(STerrainInfo_forExportHeightMap *psTerrainInfo)
{
	int nCount=0, i, n, t ;
	float x, z ;
	CASEData *pcASEData=psTerrainInfo->pcASEData ;
	geo::STriangle sTriangle ;
	geo::SLine sLine ;
	STerrain_FragmentInfo *psFragmentInfo=psTerrainInfo->psFragmentInfo ;
	SMesh *pMesh ;
	Vector3 vBaryCenter ;
	int nMaterialID ;
	int nWidth = psTerrainInfo->nWidth, nHeight = psTerrainInfo->nHeight ;
	bool bIntersect=false ;

	for(z=psTerrainInfo->fZStart ; z>psTerrainInfo->fZEnd ; z-=psTerrainInfo->fStep)
	{
		for(x=(float)(-psTerrainInfo->nWidth/2.0f); x<=(float)psTerrainInfo->nWidth/2.0f; x+=psTerrainInfo->fStep, nCount++)
		{
			bIntersect = false ;

			sLine.set(Vector3(x, 1000.0f, z), Vector3(0, -1, 0), 2000.0f) ;

			for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
			{
				pMesh = pcASEData->m_apMesh[i] ;
				nMaterialID = pMesh->nMaterialIndex ;
				for(n=0 ; n<pMesh->nNumTriangle ; n++)
				{
					for(t=0 ; t<3 ; t++)
					{
						sTriangle.avVertex[t].vPos.x = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.x ;
						sTriangle.avVertex[t].vPos.y = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.y ;
						sTriangle.avVertex[t].vPos.z = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.z ;

						sTriangle.avVertex[t].vNormal.x = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].normal.x ;
						sTriangle.avVertex[t].vNormal.y = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].normal.y ;
						sTriangle.avVertex[t].vNormal.z = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].normal.z ;

						if( float_eq(sTriangle.avVertex[t].vPos.x, 2.0f) && float_eq(sTriangle.avVertex[t].vPos.z, 2.0f) )
							nCount = nCount ;

						sTriangle.avVertex[t].vTex.x = pMesh->psTriangle[n].asTexCoord[t].u ;
						sTriangle.avVertex[t].vTex.y = pMesh->psTriangle[n].asTexCoord[t].v ;
					}

					sTriangle.sPlane.a = pMesh->psTriangle[n].sFaceNormal.x ;
					sTriangle.sPlane.b = pMesh->psTriangle[n].sFaceNormal.y ;
					sTriangle.sPlane.c = pMesh->psTriangle[n].sFaceNormal.z ;

					sTriangle.sPlane.d = -(sTriangle.sPlane.a*sTriangle.avVertex[0].vPos.x
						+ sTriangle.sPlane.b*sTriangle.avVertex[0].vPos.y
						+ sTriangle.sPlane.c*sTriangle.avVertex[0].vPos.z) ;

					//평면이 만들어지지 않으면 계산에 넣지 않는다.
					//bEnable = sTriangle.sPlane.MakePlane(sTriangle.avVertex[0].vPos, sTriangle.avVertex[1].vPos, sTriangle.avVertex[2].vPos) ;

					//if(IsPointOnTriangle(Vector3(x, 0.0f, z), sTriangle, vBaryCenter))
					if(IntersectLinetoTriangle(sLine, sTriangle, vBaryCenter) == geo::INTERSECT_POINT)
					{
						//int sssss = IntersectLinetoTriangle(sLine, sTriangle, vBaryCenter) ;
						//TRACE("success intersect uv(%05.02f %05.02f)\r\n", u, v) ;

						//Height Value Interpolation
						//psFragmentInfo[nCount].fElevation = (1-u-v)*sTriangle.avPos[0].y + u*sTriangle.avPos[1].y + v*sTriangle.avPos[2].y ;
						psFragmentInfo[nCount].fElevation =
							(vBaryCenter.x)*sTriangle.avVertex[0].vPos.y + vBaryCenter.y*sTriangle.avVertex[1].vPos.y + vBaryCenter.z*sTriangle.avVertex[2].vPos.y ;

						//u of texture Coordinate Interpolation
						//psFragmentInfo[nCount].u = (1-u-v)*sTriangle.avTex[0].x + u*sTriangle.avTex[1].x + v*sTriangle.avTex[2].x ;
						psFragmentInfo[nCount].u =
							(vBaryCenter.x)*sTriangle.avVertex[0].vTex.x + vBaryCenter.y*sTriangle.avVertex[1].vTex.x + vBaryCenter.z*sTriangle.avVertex[2].vTex.x ;

						// (x + w/2)/w
						// x/w + 1/2
						psFragmentInfo[nCount].u = x/nWidth + 0.5f ;// fabs((x+nWidth/2.0f)) ;

						//v of texture Coordinate Interpolation
						//psFragmentInfo[nCount].v = (1-u-v)*sTriangle.avTex[0].y + u*sTriangle.avTex[1].y + v*sTriangle.avTex[2].y ;
						psFragmentInfo[nCount].v =
							(vBaryCenter.x)*sTriangle.avVertex[0].vTex.y + vBaryCenter.y*sTriangle.avVertex[1].vTex.y + vBaryCenter.z*sTriangle.avVertex[2].vTex.y ;

						// (h/2 - z)/h
						// 1/2 - z/h
						psFragmentInfo[nCount].v = 0.5f - z/nHeight ;// fabs(nHeight/2.0f-z) ;

						psFragmentInfo[nCount].nMaterialID = pMesh->psTriangle[n].nMaterialID ;

						//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n",
						//	nCount, x, psFragmentInfo[nCount].fElevation, z, psFragmentInfo[nCount].u, psFragmentInfo[nCount].v) ;

						bIntersect = true ;
						break ;
					}
				}

				if(bIntersect)
					break ;
			}

			if(!bIntersect)
			{
				psFragmentInfo[nCount].fElevation = 0 ;
				//psFragmentInfo[nCount].u = psFragmentInfo[nCount].v = 0 ;
				psFragmentInfo[nCount].nMaterialID = -1 ;

				// (x + w/2)/w
				// x/w + 1/2
				psFragmentInfo[nCount].u = x/nWidth + 0.5f ;// fabs((x+nWidth/2.0f)) ;

				// (h/2 - z)/h
				// 1/2 - z/h
				psFragmentInfo[nCount].v = 0.5f - z/nHeight ;// fabs(nHeight/2.0f-z) ;

				//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n",
				//			nCount, x, psFragmentInfo[nCount].fElevation, z, psFragmentInfo[nCount].u, psFragmentInfo[nCount].v) ;
			}
		}
	}

	CloseHandle(psTerrainInfo->hThread) ;
	DWORD id = psTerrainInfo->dwThreadID ;
	psTerrainInfo->bEnd = true ;
	TRACE("TerrainInfo Exit Thread ID=%d\r\n", id) ;
	return true ;
}

void CSecretMeshObject::SetObjName(char *pszObjName)
{
	sprintf(m_szObjName, "%s", pszObjName) ;
}
char *CSecretMeshObject::GetObjName()
{
	return m_szObjName ;
}
char *CSecretMeshObject::GetCatalogName()
{
	return m_szCatalogName ;
}
void CSecretMeshObject::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_lAttr |= nAttr ;
	else
		m_lAttr &= (~nAttr) ;
}
void CSecretMeshObject::SetShadowMap(CSecretShadowMap *pcShadowMap)
{
	m_pcShadowMap = pcShadowMap ;
}

int SubSetAddAlpha(float ratio)
{
	int nAlpha = 0x40 ;
	if(float_eq(ratio, 0.0f))
		return nAlpha ;

	DWORD *pdw ;	
	ratio *= 0.01f ;
	pdw = (DWORD *)&ratio ;
	if((*pdw) & 0x80000000)//minus -> plus
		(*pdw) &= ~((*pdw) & 0x80000000) ;
	else//plus -> minus
		(*pdw) |= 0x80000000 ;

	nAlpha = nAlpha+(int)(nAlpha*ratio) ;
	return nAlpha ;
}
void CSecretMeshObject::SetAddAlpha(char *pszObjectName)
{
	//원래 알파테스트값 0x40
	//nAlpha+(DWORD)(nAlpha*0.6f) 일경우 감소
	//nAlpha-(DWORD)(nAlpha*0.6f) 일경우 증가
	int nAlpha = 0x40 ;

	if(strstr(pszObjectName, "00_soo_w1"))
		nAlpha = SubSetAddAlpha(+40) ;
	else if(strstr(pszObjectName, "00_soo"))
		nAlpha = SubSetAddAlpha(-20) ;

	else if(strstr(pszObjectName, "01_nue"))
		nAlpha = SubSetAddAlpha(-60) ;

	
	else if(strstr(pszObjectName, "03_dan_y15_280c"))
		nAlpha = SubSetAddAlpha(-20) ;
	else if(strstr(pszObjectName, "03_dan_y14_6m"))
		nAlpha = SubSetAddAlpha(-20) ;
	else if(strstr(pszObjectName, "03_dan_y12_7m"))
		nAlpha = SubSetAddAlpha(-20) ;
	else if(strstr(pszObjectName, "03_dan_y11_8m"))
		nAlpha = SubSetAddAlpha(+10) ;
	else if(strstr(pszObjectName, "03_dan_y10_5m"))
		nAlpha = SubSetAddAlpha(+10) ;
	else if(strstr(pszObjectName, "03_dan_y9_8m"))
		nAlpha = SubSetAddAlpha(+10) ;
	else if(strstr(pszObjectName, "03_dan_y8_4m"))
		nAlpha = SubSetAddAlpha(5) ;
	else if(strstr(pszObjectName, "03_dan_y7_5m"))
		nAlpha = SubSetAddAlpha(5) ;
	else if(strstr(pszObjectName, "03_dan_y3_7m"))
		nAlpha = SubSetAddAlpha(5) ;
	else if(strstr(pszObjectName, "03_dan_y1_12m"))
		nAlpha = SubSetAddAlpha(-75) ;
	else if(strstr(pszObjectName, "03_dan_o1_6m"))
		nAlpha = SubSetAddAlpha(-55) ;
	else if(strstr(pszObjectName, "03_dan_o3_10m"))
		nAlpha = SubSetAddAlpha(-55) ;
	else if(strstr(pszObjectName, "03_dan_y2_4m"))
		nAlpha = SubSetAddAlpha(+10) ;
	else if(strstr(pszObjectName, "03_dan_y4_7m"))
		nAlpha = SubSetAddAlpha(-20) ;
	else if(strstr(pszObjectName, "03_dan_y5_4m"))
		nAlpha = SubSetAddAlpha(-5) ;
	else if(strstr(pszObjectName, "03_dan_y6_5m"))
		nAlpha = SubSetAddAlpha(-20) ;
	else if(strstr(pszObjectName, "03_dan"))
		nAlpha = SubSetAddAlpha(-70) ;

	else if(strstr(pszObjectName, "04_bud"))
		nAlpha = SubSetAddAlpha(+30) ;

	else if(strstr(pszObjectName, "05_kko"))
		nAlpha = SubSetAddAlpha(-10) ;

	else if(strstr(pszObjectName, "07_pla"))
		nAlpha = SubSetAddAlpha(-50) ;

	else if(strstr(pszObjectName, "08_gye_f3_15m"))
		nAlpha = SubSetAddAlpha(+30) ;
	else if(strstr(pszObjectName, "08_gye"))
		nAlpha = SubSetAddAlpha(+40) ;

	else if(strstr(pszObjectName, "09_mok"))
		nAlpha = SubSetAddAlpha(+50) ;

	else if(strstr(pszObjectName, "10_gaa_i") || strstr(pszObjectName, "10_gaa_o"))
		nAlpha = SubSetAddAlpha(-20) ;
	else if(strstr(pszObjectName, "10_gaa_t"))
		nAlpha = SubSetAddAlpha(-1) ;

	else if(strstr(pszObjectName, "13_dae"))
		nAlpha = SubSetAddAlpha(+5) ;

	else if(strstr(pszObjectName, "12_mee"))
		nAlpha = SubSetAddAlpha(-40) ;

	else if(strstr(pszObjectName, "15_jag"))
		nAlpha = SubSetAddAlpha(-20) ;

	else if(strstr(pszObjectName, "18_met"))
		nAlpha = SubSetAddAlpha(-50) ;

	else if(strstr(pszObjectName, "21_hym"))
		nAlpha = SubSetAddAlpha(-10) ;

	else if(strstr(pszObjectName, "22_jaj"))
		nAlpha = SubSetAddAlpha(+42) ;

	else if(strstr(pszObjectName, "23_chm"))
		nAlpha = SubSetAddAlpha(+30) ;

	else if(strstr(pszObjectName, "24_hit_i3_8m") || strstr(pszObjectName, "24_hit_i4_8m"))
		nAlpha = SubSetAddAlpha(+70) ;
	else if(strstr(pszObjectName, "24_hit_i9_4m") || strstr(pszObjectName, "24_hit_i10_3m") || strstr(pszObjectName, "24_hit_i11_2m") || strstr(pszObjectName, "24_hit_i12_1m"))
		nAlpha = SubSetAddAlpha(+70) ;
	else if(strstr(pszObjectName, "24_hit"))
		nAlpha = SubSetAddAlpha(+35) ;

	else if(strstr(pszObjectName, "69_top"))
		nAlpha = SubSetAddAlpha(-80) ;

	else if(strstr(pszObjectName, "61_kkw_o2_150c_02595") || strstr(pszObjectName, "61_kkw_o8_150c_02595"))
		nAlpha = SubSetAddAlpha(+40) ;

	else if(strstr(pszObjectName, "16_bae") || strstr(pszObjectName, "31_kae") || strstr(pszObjectName, "34_jop")
		|| strstr(pszObjectName, "51_jan") || strstr(pszObjectName, "51_hap") || strstr(pszObjectName, "53_rud")
		|| strstr(pszObjectName, "62_chl") || strstr(pszObjectName, "65_nng") || strstr(pszObjectName, "74_jar")
		 || strstr(pszObjectName, "75_okj"))
	{
		nAlpha = SubSetAddAlpha(+15) ;
		sprintf(m_szCatalogName, "flowers") ;
	}

	else if(strstr(pszObjectName, "100_yaj_e") || strstr(pszObjectName, "100_yaj_c") || strstr(pszObjectName, "100_yaj_d"))
		nAlpha = SubSetAddAlpha(-80) ;

	else if(strstr(pszObjectName, "100_yaj_g") || strstr(pszObjectName, "100_yaj_h"))
		nAlpha = SubSetAddAlpha(-40) ;

	else if(strstr(pszObjectName, "101_yak_a"))
		nAlpha = SubSetAddAlpha(-15) ;

    else if(strstr(pszObjectName, "14_buj"))
		nAlpha = SubSetAddAlpha(-10) ;

	else
		nAlpha = 0x00 ;

	if(nAlpha >= 0xff)
		nAlpha = 0xff ;
	else if(nAlpha <= 0)
		nAlpha = 0x00 ;

	m_dwAddAlpha = (DWORD)nAlpha ;
}
void CSecretMeshObject::SetBillboardAttr(char *pszObjectName)
{
	if(strstr(pszObjectName, "_pl"))
		m_lAttr |= ATTR_BILLBOARD_NOMIPMAPFILTER ;
	/*
	if(strstr(pszObjectName, "04_bud")
		|| strstr(pszObjectName, "05_kko")
		|| strstr(pszObjectName, "08_gye")
        || strstr(pszObjectName, "09_mok")
		|| strstr(pszObjectName, "22_jaj")
		|| strstr(pszObjectName, "23_chm")
		|| strstr(pszObjectName, "24_hit")

        || strstr(pszObjectName, "13_dae_o1_1m_005")
		|| strstr(pszObjectName, "13_dae_o2_1m_008")
		|| strstr(pszObjectName, "15_jag_s1_5m_09478")
		|| strstr(pszObjectName, "19_chi")

		|| strstr(pszObjectName, "16_bae")
		|| strstr(pszObjectName, "31_kae")
		|| strstr(pszObjectName, "32_moo")
		|| strstr(pszObjectName, "34_jop")
		|| strstr(pszObjectName, "51_jan")
		|| strstr(pszObjectName, "52_hap")
		|| strstr(pszObjectName, "53_rud")
		|| strstr(pszObjectName, "62_chl")
		|| strstr(pszObjectName, "65_nng")
		|| strstr(pszObjectName, "71_gal")
		|| strstr(pszObjectName, "74_jar")
		|| strstr(pszObjectName, "75_okj")
		|| strstr(pszObjectName, "77_eok")
		|| strstr(pszObjectName, "79_mul")
		|| strstr(pszObjectName, "91_boo")

		//add
		|| strstr(pszObjectName, "01_nue")
		|| strstr(pszObjectName, "01_dan")
		|| strstr(pszObjectName, "06_chu")
		|| strstr(pszObjectName, "12_mee")
		|| strstr(pszObjectName, "15_jag")
		|| strstr(pszObjectName, "12_mee")
		|| strstr(pszObjectName, "101_yak")
		|| strstr(pszObjectName, "104_yan")
		|| strstr(pszObjectName, "104_yaj_b")
		|| strstr(pszObjectName, "104_yaj_d")
		|| strstr(pszObjectName, "96_cha")



		)
	{
		m_lAttr |= ATTR_BILLBOARD_NOMIPMAPFILTER ;
	}
	*/
}

int CSecretMeshObject::GetBoundingPosfromAllMesh(Vector3 *pvRecvPos)
{
	int i, nCount=0 ;
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pcMeshNode ;

	while(pNode != pTail)
	{
		pcMeshNode = (CSecretMesh *)pNode->pvData ;

		for(i=0 ; i<8; i++)
		{
			pvRecvPos[nCount++] = pcMeshNode->m_sAACube.GetPos(i) ;
			//TRACE("Position(%07.03f %07.03f %07.03f)\r\n", enumVector(pvRecvPos[nCount-1])) ;
		}

		pNode = pNode->pNext ;
	}
	return nCount ;
}
void CSecretMeshObject::buildCollisionVolume()
{
	int i ;
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pcMeshNode ;

	TRACE("buildCollisionVolume %s\r\n", m_szObjName) ;

	while(pNode != pTail)
	{
		pcMeshNode = (CSecretMesh *)pNode->pvData ;

		if((pcMeshNode->GetKind() == SECRETNODEKIND_RIGID) && !(pcMeshNode->GetAttr() & SECRETNODEATTR_ALPHABLENDING))
			((CSecretRigidMesh *)pcMeshNode)->BuildCollisionModel() ;

		pNode = pNode->pNext ;
	}
}
void CSecretMeshObject::releaseCollisionVolume()
{
	int i ;
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pcMeshNode ;

	TRACE("releaseCollisionVolume %s\r\n", m_szObjName) ;

	while(pNode != pTail)
	{
		pcMeshNode = (CSecretMesh *)pNode->pvData ;

		if((pcMeshNode->GetKind() == SECRETNODEKIND_RIGID) && !(pcMeshNode->GetAttr() & SECRETNODEATTR_ALPHABLENDING))
			((CSecretRigidMesh *)pcMeshNode)->ReleaseCollisionModel() ;

		pNode = pNode->pNext ;
	}
}
void CSecretMeshObject::setAlphaBlendingbyForce(float alpha)
{
	int i ;
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pcMeshNode ;

	while(pNode != pTail)
	{
		pcMeshNode = (CSecretMesh *)pNode->pvData ;
		pcMeshNode->m_fAlphaBlendingByForce = alpha ;
		pNode = pNode->pNext ;
	}
}
void CSecretMeshObject::setWavingLeaf()
{
	DWORD dwAttr ;
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretMesh *pcMeshNode ;

	while(pNode != pTail)
	{
		pcMeshNode = (CSecretMesh *)pNode->pvData ;

		dwAttr = pcMeshNode->GetAttr() ;
		if((dwAttr & SECRETNODEATTR_ALPHABLENDING) || (dwAttr & SECRETNODEATTR_TWOSIDED))
		{
			m_lAttr |= ATTR_WAVINGLEAF ;
			return ;
		}
		pNode = pNode->pNext ;
	}
}

bool CSecretMeshObject::ExportHeightMap(CASEData *pcASEData, char *pszFileName)
{
	int i, n, t ;
	float fMaxX, fMinX, fMaxY, fMinY, fMaxZ, fMinZ ;
	SMesh *pMesh; 

	fMaxX = fMinX = pcASEData->m_apMesh[0]->psVertex->pos.x ;
	fMaxY = fMinY = pcASEData->m_apMesh[0]->psVertex->pos.y ;
	fMaxZ = fMinZ = pcASEData->m_apMesh[0]->psVertex->pos.z ;

	//Get Number of Material
	STerrain_MaterialInfo asMaterialInfo[256] ;
	int nNumMaterial = 0 ;
	for(i=0 ; i<pcASEData->m_nNumMaterial ; i++)
	{
		if(pcASEData->m_psMaterial[i].bUseTexDecal)
		{
			memcpy(asMaterialInfo[nNumMaterial].szFileName, pcASEData->m_psMaterial[i].szDecaleName, strlen(pcASEData->m_psMaterial[i].szDecaleName)) ;

			asMaterialInfo[nNumMaterial].afAmbient[0] = pcASEData->m_psMaterial[i].sAmbient.r ;
			asMaterialInfo[nNumMaterial].afAmbient[1] = pcASEData->m_psMaterial[i].sAmbient.g ;
			asMaterialInfo[nNumMaterial].afAmbient[2] = pcASEData->m_psMaterial[i].sAmbient.b ;

			asMaterialInfo[nNumMaterial].afDiffuse[0] = pcASEData->m_psMaterial[i].sDiffuse.r ;
			asMaterialInfo[nNumMaterial].afDiffuse[1] = pcASEData->m_psMaterial[i].sDiffuse.g ;
			asMaterialInfo[nNumMaterial].afDiffuse[2] = pcASEData->m_psMaterial[i].sDiffuse.b ;

			asMaterialInfo[nNumMaterial].afSpecular[0] = pcASEData->m_psMaterial[i].sSpecular.r ;
			asMaterialInfo[nNumMaterial].afSpecular[1] = pcASEData->m_psMaterial[i].sSpecular.g ;
			asMaterialInfo[nNumMaterial].afSpecular[2] = pcASEData->m_psMaterial[i].sSpecular.b ;

			nNumMaterial++ ;
		}

		if(pcASEData->m_psMaterial[i].nNumSubMaterial)
		{
			for(n=0 ; n<pcASEData->m_psMaterial[i].nNumSubMaterial ; n++)
			{
				if(pcASEData->m_psMaterial[i].psSubMaterial[n].bUseTexDecal)
				{
					memcpy(asMaterialInfo[nNumMaterial].szFileName,
						pcASEData->m_psMaterial[i].psSubMaterial[n].szDecaleName,
						strlen(pcASEData->m_psMaterial[i].psSubMaterial[n].szDecaleName)) ;

					asMaterialInfo[nNumMaterial].afAmbient[0] = pcASEData->m_psMaterial[i].psSubMaterial[n].sAmbient.r ;
					asMaterialInfo[nNumMaterial].afAmbient[1] = pcASEData->m_psMaterial[i].psSubMaterial[n].sAmbient.g ;
					asMaterialInfo[nNumMaterial].afAmbient[2] = pcASEData->m_psMaterial[i].psSubMaterial[n].sAmbient.b ;

					asMaterialInfo[nNumMaterial].afDiffuse[0] = pcASEData->m_psMaterial[i].psSubMaterial[n].sDiffuse.r ;
					asMaterialInfo[nNumMaterial].afDiffuse[1] = pcASEData->m_psMaterial[i].psSubMaterial[n].sDiffuse.g ;
					asMaterialInfo[nNumMaterial].afDiffuse[2] = pcASEData->m_psMaterial[i].psSubMaterial[n].sDiffuse.b ;

					asMaterialInfo[nNumMaterial].afSpecular[0] = pcASEData->m_psMaterial[i].psSubMaterial[n].sSpecular.r ;
					asMaterialInfo[nNumMaterial].afSpecular[1] = pcASEData->m_psMaterial[i].psSubMaterial[n].sSpecular.g ;
					asMaterialInfo[nNumMaterial].afSpecular[2] = pcASEData->m_psMaterial[i].psSubMaterial[n].sSpecular.b ;

					nNumMaterial++ ;
				}
			}
		}
	}


	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		pMesh = pcASEData->m_apMesh[i] ;
		for(n=0 ; n<pMesh->nNumVertex ; n++)
		{
			if(fMaxX < pMesh->psVertex[n].pos.x)
				fMaxX = pMesh->psVertex[n].pos.x ;
			if(fMinX > pMesh->psVertex[n].pos.x)
				fMinX = pMesh->psVertex[n].pos.x ;

			if(fMaxY < pMesh->psVertex[n].pos.y)
				fMaxY = pMesh->psVertex[n].pos.y ;
			if(fMinY > pMesh->psVertex[n].pos.y)
				fMinY = pMesh->psVertex[n].pos.y ;

			if(fMaxZ < pMesh->psVertex[n].pos.z)
				fMaxZ = pMesh->psVertex[n].pos.z ;
			if(fMinZ > pMesh->psVertex[n].pos.z)
				fMinZ = pMesh->psVertex[n].pos.z ;
		}
	}

	int nWidth = CloseToExp((fMaxX-fMinX), 2) ;
	int nHeight = CloseToExp((fMaxZ-fMinZ), 2) ;
	int nCount=0 ;
	bool bIntersect=false ;
	//int nWidth = CloseToExp((fMaxX-fMinX)+1, 2)+1 ;
	//int nHeight = CloseToExp((fMaxZ-fMinZ)+1, 2)+1 ;
	//int nCount=0 ;
	//bool bIntersect=false ;

    if(nWidth > nHeight)
		nHeight = nWidth ;
	else
		nWidth = nHeight ;

	float fnumerator   = 1 ;
	float fdenominator = 1 ;
	float fStep = fnumerator/fdenominator ;
	geo::STriangle sTriangle ;
	geo::SLine sLine ;
	STerrain_FragmentInfo *psFragmentInfo = NULL ;
	int nMaterialID ;
	float x, z ;
	Vector3 vBaryCenter ;
	int nNumFragments = ((int)(nWidth/fStep)+1) * ((int)(nHeight/fStep)+1) ;

	psFragmentInfo = new STerrain_FragmentInfo[nNumFragments] ;
/*
	DWORD dwThreadID=0 ;
	nCount=0 ;
	int nDivider=8 ;
	int nNumThread=(nWidth/nDivider) ;

    STerrainInfo_forExportHeightMap *psTerrainInfo = new STerrainInfo_forExportHeightMap[nNumThread] ;
	STerrainInfo_forExportHeightMap *p = psTerrainInfo ;

    for(z=(float)nHeight/2.0f; z>(float)(-nHeight/2.0f); z-=(float)nDivider, dwThreadID++, p++, nCount++)
	{
		p->pcASEData = pcASEData ;
		int nSeek = (nCount*nDivider) * ((int)(nWidth/fStep)+1) ;
		p->psFragmentInfo = &psFragmentInfo[(nCount*nDivider) * ((int)(nWidth/fStep)+1)] ;
		p->nWidth = nWidth ;
		p->nHeight = nHeight ;
		p->fZStart = z ;
		p->fZEnd = p->fZStart-(float)nDivider ;
		p->fStep = fStep ;
		p->dwThreadID = dwThreadID ;
		p->bEnd = false ;

		p->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ExportHeightMapThreadProc, p, 0, &dwThreadID) ;
	}

	bool bAllEnded ;

	MSG msg ;
	DWORD dwStart = GetTickCount() ;
	DWORD dwTime = (1000*60)*5 ;

	while((GetTickCount()-dwStart) < dwTime)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
		}

		bAllEnded = true ;
        for(i=0 ; i<nNumThread ; i++)
		{
			if(psTerrainInfo[i].bEnd == false)
				bAllEnded = false ;
		}
		if(bAllEnded)
			break ;
	}
*/
/*
	for(z=(float)nHeight/2.0f; z>=(float)(-nHeight/2.0f); z-=fStep, dwThreadID++, nCount++)
	{
		STerrainInfo_forExportHeightMap *psTerrainInfo = new STerrainInfo_forExportHeightMap ;
		
		psTerrainInfo->pcASEData = pcASEData ;
		int nSeek = nCount * ((int)(nWidth/fStep)+1) ;
		psTerrainInfo->psFragmentInfo = &psFragmentInfo[nCount * ((int)(nWidth/fStep)+1)] ;
		psTerrainInfo->nWidth = nWidth ;
		psTerrainInfo->nHeight = nHeight ;
		psTerrainInfo->z = z ;
		psTerrainInfo->fStep = fStep ;
		psTerrainInfo->dwThreadID = dwThreadID ;

		TRACE("thread start\r\n") ;
		psTerrainInfo->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ExportHeightMapThreadProc, psTerrainInfo, 0, &dwThreadID) ;
	}
*/

	//for(z=fMinZ ; z<=(fMinZ+nHeight-1) ; z += fStep)
	for(z=(float)nHeight/2.0f; z>=(float)(-nHeight/2.0f); z-=fStep)
	{
		//TRACE("line %g/%d \r\n", z, nHeight) ;
		//for(x=fMinX ; x<=(fMinX+nWidth-1) ; x += fStep, nCount++)
		for(x=(float)(-nWidth/2.0f); x<=(float)nWidth/2.0f; x+=fStep, nCount++)
		{
			bIntersect = false ;

			sLine.set(Vector3(x, 100, z), Vector3(0, -1, 0), 200) ;

			for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
			{
				pMesh = pcASEData->m_apMesh[i] ;
				nMaterialID = pMesh->nMaterialIndex ;
				for(n=0 ; n<pMesh->nNumTriangle ; n++)
				{
					for(t=0 ; t<3 ; t++)
					{
						//sTriangle.avPos[t].x = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.x ;
						//sTriangle.avPos[t].y = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.y ;
						//sTriangle.avPos[t].z = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.z ;
						sTriangle.avVertex[t].vPos.x = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.x ;
						sTriangle.avVertex[t].vPos.y = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.y ;
						sTriangle.avVertex[t].vPos.z = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].pos.z ;

						sTriangle.avVertex[t].vNormal.x = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].normal.x ;
						sTriangle.avVertex[t].vNormal.y = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].normal.y ;
						sTriangle.avVertex[t].vNormal.z = pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[t]].normal.z ;

						if( float_eq(sTriangle.avVertex[t].vPos.x, 2.0f) && float_eq(sTriangle.avVertex[t].vPos.z, 2.0f) )
							nCount = nCount ;

						sTriangle.avVertex[t].vTex.x = pMesh->psTriangle[n].asTexCoord[t].u ;
						sTriangle.avVertex[t].vTex.y = pMesh->psTriangle[n].asTexCoord[t].v ;
					}

					sTriangle.sPlane.a = pMesh->psTriangle[n].sFaceNormal.x ;
					sTriangle.sPlane.b = pMesh->psTriangle[n].sFaceNormal.y ;
					sTriangle.sPlane.c = pMesh->psTriangle[n].sFaceNormal.z ;

					sTriangle.sPlane.d = -(sTriangle.sPlane.a*sTriangle.avVertex[0].vPos.x
											+ sTriangle.sPlane.b*sTriangle.avVertex[0].vPos.y
											+ sTriangle.sPlane.c*sTriangle.avVertex[0].vPos.z) ;

					//if(float_eq(z, 1) && float_eq(x, 0) && n == 18)
					//if(float_eq(z, 2) && float_eq(x, -1) && n == 18)
					if(float_eq(z, 3) && float_eq(x, -3) && n == 24)
					{
						TRACE("Triangle 0(%07.03f %07.03f %07.03f) 1(%07.03f %07.03f %07.03f) 2(%07.03f %07.03f %07.03f)\r\n",
							enumVector(sTriangle.avVertex[0].vPos), enumVector(sTriangle.avVertex[1].vPos), enumVector(sTriangle.avVertex[2].vPos)) ;
					}
					//평면이 만들어지지 않으면 계산에 넣지 않는다.
					//bEnable = sTriangle.sPlane.MakePlane(sTriangle.avVertex[0].vPos, sTriangle.avVertex[1].vPos, sTriangle.avVertex[2].vPos) ;


					//if(IsPointOnTriangle(Vector3(x, 0.0f, z), sTriangle, vBaryCenter))
					if(IntersectLinetoTriangle(sLine, sTriangle, vBaryCenter) == geo::INTERSECT_POINT)
					{
						//int sssss = IntersectLinetoTriangle(sLine, sTriangle, vBaryCenter) ;
						//TRACE("success intersect uv(%05.02f %05.02f)\r\n", u, v) ;

						//Height Value Interpolation
						//psFragmentInfo[nCount].fElevation = (1-u-v)*sTriangle.avPos[0].y + u*sTriangle.avPos[1].y + v*sTriangle.avPos[2].y ;
						psFragmentInfo[nCount].fElevation =
							(vBaryCenter.x)*sTriangle.avVertex[0].vPos.y + vBaryCenter.y*sTriangle.avVertex[1].vPos.y + vBaryCenter.z*sTriangle.avVertex[2].vPos.y ;

						//u of texture Coordinate Interpolation
						//psFragmentInfo[nCount].u = (1-u-v)*sTriangle.avTex[0].x + u*sTriangle.avTex[1].x + v*sTriangle.avTex[2].x ;
						psFragmentInfo[nCount].u =
							(vBaryCenter.x)*sTriangle.avVertex[0].vTex.x + vBaryCenter.y*sTriangle.avVertex[1].vTex.x + vBaryCenter.z*sTriangle.avVertex[2].vTex.x ;

						// (x + w/2)/w
						// x/w + 1/2
						psFragmentInfo[nCount].u = x/nWidth + 0.5f ;// fabs((x+nWidth/2.0f)) ;

						//v of texture Coordinate Interpolation
						//psFragmentInfo[nCount].v = (1-u-v)*sTriangle.avTex[0].y + u*sTriangle.avTex[1].y + v*sTriangle.avTex[2].y ;
						psFragmentInfo[nCount].v =
							(vBaryCenter.x)*sTriangle.avVertex[0].vTex.y + vBaryCenter.y*sTriangle.avVertex[1].vTex.y + vBaryCenter.z*sTriangle.avVertex[2].vTex.y ;

						// (h/2 - z)/h
						// 1/2 - z/h
						psFragmentInfo[nCount].v = 0.5f - z/nHeight ;// fabs(nHeight/2.0f-z) ;

						psFragmentInfo[nCount].nMaterialID = pMesh->psTriangle[n].nMaterialID ;

						//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n",
						//	nCount, x, psFragmentInfo[nCount].fElevation, z, psFragmentInfo[nCount].u, psFragmentInfo[nCount].v) ;

						bIntersect = true ;
                        break ;
					}
				}

				if(bIntersect)
					break ;
			}

			if(!bIntersect)
			{
				psFragmentInfo[nCount].fElevation = 0 ;
				//psFragmentInfo[nCount].u = psFragmentInfo[nCount].v = 0 ;
				psFragmentInfo[nCount].nMaterialID = -1 ;

				// (x + w/2)/w
				// x/w + 1/2
				psFragmentInfo[nCount].u = x/nWidth + 0.5f ;// fabs((x+nWidth/2.0f)) ;

				// (h/2 - z)/h
				// 1/2 - z/h
				psFragmentInfo[nCount].v = 0.5f - z/nHeight ;// fabs(nHeight/2.0f-z) ;

				//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n",
				//			nCount, x, psFragmentInfo[nCount].fElevation, z, psFragmentInfo[nCount].u, psFragmentInfo[nCount].v) ;
			}
		}
	}

	//for(i=0; i<nNumFragments; i++)
	//{
	//	TRACE("[%02d] Elevation=%+07.03f tex(%+07.03f %+07.03f)\r\n",
	//						i, psFragmentInfo[i].fElevation, psFragmentInfo[i].u, psFragmentInfo[i].v) ;
	//}

	STerrain_FileHeader *psFileHeader=NULL ;
	psFileHeader = new STerrain_FileHeader ;
	
	psFileHeader->dwType = 't' + 'r' + 'n' ;
	psFileHeader->lNumMaterial = nNumMaterial ;
	psFileHeader->lWidth = nWidth ;
	psFileHeader->lHeight = nHeight ;
	psFileHeader->nNumerator = (int)fnumerator ;
	psFileHeader->nDenominator = (int)fdenominator ;

	STerrain_File sTerrainFile ;
	sTerrainFile.psFileHeader = psFileHeader ;
	sTerrainFile.psMaterialInfo = asMaterialInfo ;
	sTerrainFile.psFragmentInfo = psFragmentInfo ;

    //SaveTerrainFile(psFileHeader, asMaterialInfo, psFragmentInfo, pszFileName) ;
	SaveTerrainFile(&sTerrainFile, pszFileName) ;

	SAFE_DELETE(psFileHeader) ;

	return true ;
}
/*
bool CSecretMeshObject::ExportHeightMap(CASEData *pcASEData)
{
	DNode *pTail = m_cMeshNode.GetTail() ;
	DNode *pNode = m_cMeshNode.GetHead()->pNext ;
	CSecretRigidMesh *pcRigidMesh ;

	pcASEData->m_apMesh[0]

	int i ;
	D3DXVECTOR4 v4Pos ;
	D3DXVECTOR2 vTex ;
	SRigidMesh_Vertex *psRigidVertices ;
	float fMaxX, fMinX, fMaxY, fMinY, fMaxZ, fMinZ ;

	fMaxX = fMinX = ((CSecretRigidMesh *)pNode->pvData)->GetVertices()->pos.x ;
	fMaxY = fMinY = ((CSecretRigidMesh *)pNode->pvData)->GetVertices()->pos.y ;
	fMaxZ = fMinZ = ((CSecretRigidMesh *)pNode->pvData)->GetVertices()->pos.z ;

	while(pNode != pTail)
	{
		pcRigidMesh = (CSecretRigidMesh *)pNode->pvData ;

		psRigidVertices = pcRigidMesh->GetVertices() ;

		for(i=0 ; i<(int)pcRigidMesh->GetNumVertex() ; i++)
		{
			if(fMaxX < psRigidVertices[i].pos.x)
				fMaxX = psRigidVertices[i].pos.x ;
			if(fMinX > psRigidVertices[i].pos.x)
				fMinX = psRigidVertices[i].pos.x ;

			if(fMaxY < psRigidVertices[i].pos.y)
				fMaxY = psRigidVertices[i].pos.y ;
			if(fMinY > psRigidVertices[i].pos.y)
				fMinY = psRigidVertices[i].pos.y ;

			if(fMaxZ < psRigidVertices[i].pos.z)
				fMaxZ = psRigidVertices[i].pos.z ;
			if(fMinZ > psRigidVertices[i].pos.z)
				fMinZ = psRigidVertices[i].pos.z ;
		}

		pNode = pNode->pNext ;
	}

    int nWidth = CloseToExp(fMaxX-fMinX, 2) ;
	int nHeight = CloseToExp(fMaxZ-fMinZ, 2) ;

	STerrain_FragmentInfo *psFragmentInfo = new STerrain_FragmentInfo[nWidth*nHeight] ;

	pNode = m_cMeshNode.GetHead()->pNext ;
	while(pNode != pTail)
	{
		pcRigidMesh = (CSecretRigidMesh *)pNode->pvData ;

		pcRigidMesh->ExportFragmentInfo(fMinX, fMinZ, (float)nWidth, (float)nHeight, psFragmentInfo) ;

		pNode = pNode->pNext ;
	}

	return true ;
}
*/