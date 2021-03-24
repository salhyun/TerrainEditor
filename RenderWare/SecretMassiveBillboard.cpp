#include "SecretMassiveBillboard.h"
#include "SecretSceneManager.h"
#include "SecretMeshObject.h"
#include "SecretBillboardMesh.h"
#include "SecretShadowMap.h"

D3DVERTEXELEMENT9 decl_MassiveBillboard[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	{0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//빌보드의 크기정보
	{0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},//vDiffuse
	D3DDECL_END()
} ;

CSecretMassiveBillboard::CSecretMassiveBillboard() :
MAXNUMVERTEX(12000),
MAXNUMINDEX(12000/4*2),
MAXNUMBILLBOARD(3000),
TEXTURESIZE(2048)
{
	m_pd3dDevice = NULL ;
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_nNumBillboardIndex = 0 ;
	m_psBillboardIndex = NULL ;
	m_psUnitBillboards = NULL ;
	m_nUnitBillboardCount = 0 ;
	m_pcEffect = NULL ;
	m_apTex[0] = m_apTex[1] = m_apTex[2] = m_apTex[3] = NULL ;
	m_pcAtm = NULL ;
	m_pcShadowMap = NULL ;
	m_nAttr = 0 ;
	m_bWireFrame = false ;
}
CSecretMassiveBillboard::~CSecretMassiveBillboard()
{
	Release() ;
}
void CSecretMassiveBillboard::Release()
{
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
	SAFE_DELETEARRAY(m_psBillboardIndex) ;
	SAFE_DELETEARRAY(m_psUnitBillboards) ;
	SAFE_DELETE(m_pcEffect) ;
}
bool CSecretMassiveBillboard::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, CSecretObjectScatteringData *pcAtm)
{
	//_BuildBillboardIndex() ;
	m_pd3dDevice = pd3dDevice ;
	m_pcAtm = pcAtm ;

	m_pcEffect = new CSecretMassiveBillboardEffect() ;
	m_pcEffect->Initialize(pd3dDevice, "shader/MassiveBillboard.fx") ;
	m_pcEffect->m_psEssentialElements = psEssentialElements ;

	m_psVertices = new SMassiveBillboardVertex[MAXNUMVERTEX] ;
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(MAXNUMVERTEX*sizeof(SMassiveBillboardVertex), D3DUSAGE_WRITEONLY, D3DFVF_MASSIVEBILLBOARDVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return false ;

	m_psIndices = new SMassiveBillboardIndex[MAXNUMINDEX] ;
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(MAXNUMINDEX*sizeof(SMassiveBillboardIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL)))
		return false ;

	m_psUnitBillboards = new SUnitBillboard[MAXNUMBILLBOARD] ;
	m_nUnitBillboardCount = 0 ;

	_ImportBillboardIndex() ;

	return true ;
}
bool CSecretMassiveBillboard::_FindTextureCoord(SUnitBillboard *psUnitBillboard, STrueRenderingObject *psTRObject)
{
	for(int i=0 ; i<m_nNumBillboardIndex ; i++)
	{
		if(!strcmp(m_psBillboardIndex[i].szName, psTRObject->apcObject[0]->GetObjName()))
		{
			psUnitBillboard->psBillboardIndex = &m_psBillboardIndex[i] ;
			psUnitBillboard->psTRObject = psTRObject ;
			return true ;
		}
	}
	return false ;
}
void CSecretMassiveBillboard::_SetBillboardDiffuse(SUnitBillboard *psUnitBillboard, STrueRenderingObject *psTRObject)
{
	//if(strstr(psTRObject->apcObject[0]->GetObjName(), "104_yan"))
	//	psUnitBillboard->vDiffuse.x = psUnitBillboard->vDiffuse.y = psUnitBillboard->vDiffuse.z = 0.7f ;
	//else if(strstr(psTRObject->apcObject[0]->GetObjName(), "10_gaa_o"))
	//	psUnitBillboard->fDiffuse = 1.1f ;
	//else if(strstr(psTRObject->apcObject[0]->GetObjName(), "10_gaa_t"))
	//	psUnitBillboard->fDiffuse = 1.1f ;
	//else if(strstr(psTRObject->apcObject[0]->GetObjName(), "66_soo"))
	//	psUnitBillboard->fDiffuse = 1.0f ;
	//else
	//	psUnitBillboard->fDiffuse = 1.4f ;
}
bool CSecretMassiveBillboard::AddBillboard(STrueRenderingObject *psTRObject)
{
	if(m_nUnitBillboardCount >= MAXNUMBILLBOARD)
		return false ;

	char *pszObjName = psTRObject->apcObject[0]->GetObjName() ;

	//exception objects
	if(!strcmp(pszObjName, "100_yaj_h1_30m_057")
		|| !strcmp(pszObjName, "100_yaj_h2_25m_050")
		|| !strcmp(pszObjName, "100_yaj_h5_30m_071")
		|| !strcmp(pszObjName, "15_jag_y1_12m_19192")
		|| !strcmp(pszObjName, "15_jag_y2_16m_192")
		|| !strcmp(pszObjName, "00_soo_s3_13m_16989")

		|| !strcmp(pszObjName, "00_soo_s1_3m_9052")
		|| !strcmp(pszObjName, "00_soo_s1_4m_9464")
		|| !strcmp(pszObjName, "00_soo_s1_5m_9833")
		|| !strcmp(pszObjName, "00_soo_s1_7m_10412")
		|| !strcmp(pszObjName, "00_soo_s1_10m_9844")

		|| !strcmp(pszObjName, "00_soo_s2_11m_18763")
		|| !strcmp(pszObjName, "00_soo_s2_9m_18696")
		|| !strcmp(pszObjName, "00_soo_s2_2m_8114")
		|| !strcmp(pszObjName, "00_soo_s2_7m_18693")
		|| !strcmp(pszObjName, "00_soo_s2_8m_10073")
		|| !strcmp(pszObjName, "00_soo_s2_12m_18696")
		|| !strcmp(pszObjName, "00_soo_s2_9m_18777")
		|| !strcmp(pszObjName, "00_soo_s3_13m_16989")

		|| !strcmp(pszObjName, "00_soo_y1_14m_12125")
		|| !strcmp(pszObjName, "00_soo_y1_7m_12125"))

		return false ;

	if(strstr(pszObjName, "00_soo_f1_"))
		return false ;

	bool bFlipHorizontal=false ;
	if(!strcmp(pszObjName, "04_bud_s1_20m_13920")
		|| !strcmp(pszObjName, "04_bud_s2_20m_13920")
		|| !strcmp(pszObjName, "04_bud_y1_17m_12134")
		|| !strcmp(pszObjName, "04_bud_y2_25m_13280")
		|| !strcmp(pszObjName, "04_bud_y3_12m_06494"))
	{
		Vector3 v(psTRObject->matWorld._41, 0.0f, psTRObject->matWorld._43) ;
		//Vector3 vAxis(psTRObject->matWorld._13, psTRObject->matWorld._23, psTRObject->matWorld._33) ;
		Vector3 vAxis(psTRObject->matWorld._11, psTRObject->matWorld._21, psTRObject->matWorld._31) ;
		Vector3 vCameraPos(m_pcEffect->m_psEssentialElements->vCameraPos.x, 0.0f, m_pcEffect->m_psEssentialElements->vCameraPos.z) ;

		//오브젝트가 기본적으로 45도 돌아간 상태
		vAxis = vAxis.Normalize() ;
		v = (vCameraPos-v).Normalize() ;

		if(vAxis.dot(v) < 0.0f)
			bFlipHorizontal = true ;
		else
			bFlipHorizontal = false ;
	}

	SUnitBillboard *psUnitBillboard = &m_psUnitBillboards[m_nUnitBillboardCount] ;
	for(int i=0 ; i<m_nNumBillboardIndex ; i++)
	{
		if(!strcmp(m_psBillboardIndex[i].szName, psTRObject->apcObject[0]->GetObjName()))
		{
			D3DXVECTOR4 vRet ;
			D3DXVECTOR3 vPos(psTRObject->matWorld._41, psTRObject->matWorld._42, psTRObject->matWorld._43) ;
			D3DXVec3Transform(&vRet, &vPos, &m_pcEffect->m_psEssentialElements->matView) ;
			psUnitBillboard->fDistFromCameraInViewMatrix = vRet.z ;
			psUnitBillboard->psBillboardIndex = &m_psBillboardIndex[i] ;
			psUnitBillboard->psTRObject = psTRObject ;

			psUnitBillboard->vDiffuse.x = psTRObject->apcObject[1]->GetMesh(0)->m_vDiffuse.x ;
			psUnitBillboard->vDiffuse.y = psTRObject->apcObject[1]->GetMesh(0)->m_vDiffuse.y ;
			psUnitBillboard->vDiffuse.z = psTRObject->apcObject[1]->GetMesh(0)->m_vDiffuse.z ;

			if(bFlipHorizontal)
				psUnitBillboard->nAttr |= SUnitBillboard::ATTR_FLIPHORIZONTAL ;
			else
				psUnitBillboard->nAttr &= (~SUnitBillboard::ATTR_FLIPHORIZONTAL) ;

			_SetBillboardDiffuse(psUnitBillboard, psTRObject) ;
			m_nUnitBillboardCount++ ;
			return true ;
		}
	}
	return false ;

	//SUnitBillboard *psUnitBillboard = &m_psUnitBillboards[m_nUnitBillboardCount++] ;
	//CSecretBillboardMesh *pcBillboardMesh = (CSecretBillboardMesh *)psTRObject->apcObject[psTRObject->nCurLevel]->GetMesh(0) ;
	//psUnitBillboard->vPos.x = psTRObject->matWorld._41 ;
	//psUnitBillboard->vPos.y = psTRObject->matWorld._42 ;
	//psUnitBillboard->vPos.z = psTRObject->matWorld._43 ;
	//psUnitBillboard->fDistfromCamera = psTRObject->fDistfromCamera ;
	//psUnitBillboard->fWidth = pcBillboardMesh->m_fWidth ;
	//psUnitBillboard->fHeight = pcBillboardMesh->m_fHeight ;
	//psUnitBillboard->fTexWidth = pcBillboardMesh->m_fTexWidth ;
	//psUnitBillboard->fTexHeight = pcBillboardMesh->m_fTexHeight ;

	//오브젝트이름으로 텍스쳐인덱스를 정한다. 텍스쳐인덱스가 정해지면서 텍스쳐좌표가 정해진다.
}

int sorting_unitbillboard(const void *pv1, const void *pv2)//increasing order 오름차순 (큰숫자 -> 적은숫자)
{
	SUnitBillboard *pNode1 = (SUnitBillboard *)pv1 ;
	SUnitBillboard *pNode2 = (SUnitBillboard *)pv2 ;

	float dist = pNode1->fDistFromCameraInViewMatrix - pNode2->fDistFromCameraInViewMatrix ;
	if(dist < 0.0f)
		return 1 ;
	else if(dist > 0.0f)
		return -1 ;
	else
		return 0 ;
}
void CSecretMassiveBillboard::Process()
{
	if(!m_nUnitBillboardCount)
		return ;

	//sorting...
	qsort(m_psUnitBillboards, m_nUnitBillboardCount, sizeof(SUnitBillboard), sorting_unitbillboard) ;

	//calculate vertices and indices
	float a, b ;
	float fTxRatio, fTyRatio, fXRatio, fYRatio, div_upfactor= 0.0f ;//0.125f ;
	D3DXVECTOR3 vPos, vOffset ;
	CSecretBillboardMesh *pcBillboardMesh ;
	m_nNumVertex = m_nNumIndex = 0 ;
	SUnitBillboard *psUnitBillboards = m_psUnitBillboards ;
	for(int i=0 ; i<m_nUnitBillboardCount ; i++, psUnitBillboards++)
	{
		//getting billboard mesh
		pcBillboardMesh = (CSecretBillboardMesh *)psUnitBillboards->psTRObject->apcObject[psUnitBillboards->psTRObject->nCurLevel]->GetMesh(0) ;

		vPos = D3DXVECTOR3(psUnitBillboards->psTRObject->matWorld._41, psUnitBillboards->psTRObject->matWorld._42, psUnitBillboards->psTRObject->matWorld._43) ;

		vOffset.x = pcBillboardMesh->m_vBillboardOffset.x * 0.1f ;
		vOffset.y = pcBillboardMesh->m_vBillboardOffset.y * 0.1f ;
		vOffset.z = pcBillboardMesh->m_vBillboardOffset.z * 0.1f ;
		vPos += vOffset ;

		fTxRatio = (float)psUnitBillboards->psBillboardIndex->width/(float)TEXTURESIZE ;
		fTyRatio = (float)psUnitBillboards->psBillboardIndex->height/(float)TEXTURESIZE ;
		fXRatio = pcBillboardMesh->m_fWidth*0.05f ;//0.1f*0.5f ;
		fYRatio = pcBillboardMesh->m_fHeight*0.05f ;//0.1f*0.5f ;

		//left-top
		m_psVertices[m_nNumVertex].pos = vPos ;

		//texture coordinate
		a = (float)psUnitBillboards->psBillboardIndex->x/(float)TEXTURESIZE ;
		b = pcBillboardMesh->m_vTexMin.x*fTxRatio ;
		//(float)psUnitBillboards->psBillboardIndex->x/(float)TEXTURESIZE + pcBillboardMesh->m_vTexMin.x*(float)psUnitBillboards->psBillboardIndex->width/(float)TEXTURESIZE ;
		m_psVertices[m_nNumVertex].tex.x = a+b ;

		a = (float)psUnitBillboards->psBillboardIndex->y/(float)TEXTURESIZE ;
		b = pcBillboardMesh->m_vTexMin.y*fTyRatio ;
		m_psVertices[m_nNumVertex].tex.y = a+b ;

		//factor
		m_psVertices[m_nNumVertex].factor.x = -fXRatio ;
		m_psVertices[m_nNumVertex].factor.y = fYRatio*2.0f - div_upfactor ;
		m_psVertices[m_nNumVertex].factor.z = psUnitBillboards->psBillboardIndex->index/32.0f ;
		//m_psVertices[m_nNumVertex].factor.w = psUnitBillboards->fDiffuse ;
		m_psVertices[m_nNumVertex].diffuse = psUnitBillboards->vDiffuse ;
		m_nNumVertex++ ;

		//right-top
		m_psVertices[m_nNumVertex].pos = vPos ;

		//texture coordinate
		m_psVertices[m_nNumVertex].tex.x = m_psVertices[m_nNumVertex-1].tex.x + (pcBillboardMesh->m_vTexMax.x-pcBillboardMesh->m_vTexMin.x)*fTxRatio ;
		m_psVertices[m_nNumVertex].tex.y = m_psVertices[m_nNumVertex-1].tex.y ;

		//factor
		m_psVertices[m_nNumVertex].factor.x = fXRatio ;
		m_psVertices[m_nNumVertex].factor.y = fYRatio*2.0f - div_upfactor ;
		m_psVertices[m_nNumVertex].factor.z = psUnitBillboards->psBillboardIndex->index/32.0f ;
		//m_psVertices[m_nNumVertex].factor.w = psUnitBillboards->fDiffuse ;
		m_psVertices[m_nNumVertex].diffuse = psUnitBillboards->vDiffuse ;
		m_nNumVertex++ ;

		//left-bottom
		m_psVertices[m_nNumVertex].pos = vPos ;

		//texture coordinate
		m_psVertices[m_nNumVertex].tex.x = m_psVertices[m_nNumVertex-2].tex.x ;
		m_psVertices[m_nNumVertex].tex.y = m_psVertices[m_nNumVertex-2].tex.y + (pcBillboardMesh->m_vTexMax.y-pcBillboardMesh->m_vTexMin.y)*fTyRatio ;

		//factor
		m_psVertices[m_nNumVertex].factor.x = -fXRatio ;
		m_psVertices[m_nNumVertex].factor.y = -div_upfactor ;
		m_psVertices[m_nNumVertex].factor.z = psUnitBillboards->psBillboardIndex->index/32.0f ;
		//m_psVertices[m_nNumVertex].factor.w = psUnitBillboards->fDiffuse ;
		m_psVertices[m_nNumVertex].diffuse = psUnitBillboards->vDiffuse ;
		m_nNumVertex++ ;

		//right-bottom
		m_psVertices[m_nNumVertex].pos = vPos ;

		//texture coordinate
		m_psVertices[m_nNumVertex].tex.x = m_psVertices[m_nNumVertex-2].tex.x ;
		m_psVertices[m_nNumVertex].tex.y = m_psVertices[m_nNumVertex-2].tex.y + (pcBillboardMesh->m_vTexMax.y-pcBillboardMesh->m_vTexMin.y)*fTyRatio ;

		//factor
		m_psVertices[m_nNumVertex].factor.x = fXRatio ;
		m_psVertices[m_nNumVertex].factor.y = -div_upfactor ;
		m_psVertices[m_nNumVertex].factor.z = psUnitBillboards->psBillboardIndex->index/32.0f ;
		//m_psVertices[m_nNumVertex].factor.w = psUnitBillboards->fDiffuse ;
		m_psVertices[m_nNumVertex].diffuse = psUnitBillboards->vDiffuse ;
		m_nNumVertex++ ;

		if(psUnitBillboards->nAttr & SUnitBillboard::ATTR_FLIPHORIZONTAL)
		{
			float x ;
			x = m_psVertices[m_nNumVertex-4].tex.x ;
			m_psVertices[m_nNumVertex-4].tex.x = m_psVertices[m_nNumVertex-3].tex.x ;
			m_psVertices[m_nNumVertex-3].tex.x = x ;

			x = m_psVertices[m_nNumVertex-2].tex.x ;
			m_psVertices[m_nNumVertex-2].tex.x = m_psVertices[m_nNumVertex-1].tex.x ;
			m_psVertices[m_nNumVertex-1].tex.x = x ;
		}


		//index
		m_psIndices[m_nNumIndex].wIndex[0] = m_nNumVertex-4 ;
		m_psIndices[m_nNumIndex].wIndex[1] = m_nNumVertex-3 ;
		m_psIndices[m_nNumIndex].wIndex[2] = m_nNumVertex-2 ;
		m_nNumIndex++ ;

		m_psIndices[m_nNumIndex].wIndex[0] = m_nNumVertex-2 ;
		m_psIndices[m_nNumIndex].wIndex[1] = m_nNumVertex-3 ;
		m_psIndices[m_nNumIndex].wIndex[2] = m_nNumVertex-1 ;
		m_nNumIndex++ ;
	}

	void *pvoid ;
	if(FAILED(m_pVB->Lock(0, sizeof(SMassiveBillboardVertex)*m_nNumVertex, &pvoid, NULL)))
		return ;
	memcpy(pvoid, m_psVertices, sizeof(SMassiveBillboardVertex)*m_nNumVertex) ;
	m_pVB->Unlock() ;

	if(FAILED(m_pIB->Lock(0, sizeof(SMassiveBillboardIndex)*m_nNumIndex, &pvoid, NULL)))
		return ;
	memcpy(pvoid, m_psIndices, sizeof(SMassiveBillboardIndex)*m_nNumIndex) ;
	m_pIB->Unlock() ;
}

void CSecretMassiveBillboard::Render()
{
	if(!m_nUnitBillboardCount)
		return ;

	m_pd3dDevice->SetFVF(D3DFVF_MASSIVEBILLBOARDVERTEX) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pcEffect->m_pEffect->SetTechnique(m_pcEffect->m_hTechnique) ;
	m_pcEffect->m_pEffect->Begin(NULL, 0) ;

	if(m_nAttr & ATTR_IDSHADOW)
		m_pcEffect->m_pEffect->BeginPass(1) ;
	else
		m_pcEffect->m_pEffect->BeginPass(0) ;

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SMassiveBillboardVertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	DWORD dwCullmode ;
	m_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &dwCullmode) ;

	//DWORD dwAlphaBlend, dwZWriteEnable, dwZEnable, dwAlphaTestEnable ;
	//m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend) ;
	//m_pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWriteEnable) ;
	//m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	//m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTestEnable) ;

	//if(m_nAttr & ATTR_FIRSTPASS)
	//{
	//	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;

	//	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE) ;
	//}
	//else if(m_nAttr & ATTR_SECONDPASS)
	//{
	//	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x0000000) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	//	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR) ;

	//	m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS) ;
	//}
	//else
	//{
	//	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
	//	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;
	//}

	if(m_nAttr & ATTR_IDSHADOW)
	{
		m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;
		m_pcEffect->m_pEffect->SetMatrix(m_pcEffect->m_hmatWLP, m_pcShadowMap->GetmatLVP()) ;
	}
	else
	{
		D3DXMATRIX matWVP = m_pcEffect->m_psEssentialElements->matView * m_pcEffect->m_psEssentialElements->matProj ;
		m_pcEffect->m_pEffect->SetMatrix(m_pcEffect->m_hmatWVP, &matWVP) ;
	}

	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvCameraPos, &m_pcEffect->m_psEssentialElements->vCameraPos) ;

	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvLightDir, &m_pcEffect->m_psEssentialElements->vSunDir) ;
	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvLightColor, &m_pcEffect->m_psEssentialElements->vSunColor) ;

	m_pcEffect->m_pEffect->SetValue(m_pcEffect->m_hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;

	D3DXVECTOR4 vLookat(-m_pcEffect->m_psEssentialElements->matView._13, -m_pcEffect->m_psEssentialElements->matView._23, -m_pcEffect->m_psEssentialElements->matView._33, 0) ;
	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvLookat, &vLookat) ;

	m_pcEffect->m_pEffect->SetTexture("DecalMap01", m_apTex[0]) ;
	m_pcEffect->m_pEffect->SetTexture("DecalMap02", m_apTex[1]) ;
	m_pcEffect->m_pEffect->SetTexture("DecalMap03", m_apTex[2]) ;
	m_pcEffect->m_pEffect->SetTexture("DecalMap04", m_apTex[3]) ;

	m_pcEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nNumVertex, 0, m_nNumIndex) ;

	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwCullmode) ;

	m_pcEffect->m_pEffect->EndPass() ;
	m_pcEffect->m_pEffect->End() ;

	//m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTestEnable) ;
}

void CSecretMassiveBillboard::SetTexture(int nIndex, LPDIRECT3DTEXTURE9 pTex)
{
	m_apTex[nIndex] = pTex ;
}

void CSecretMassiveBillboard::Reset()
{
	m_nUnitBillboardCount = 0 ;
}

void CSecretMassiveBillboard::_ImportBillboardIndex()
{
	FILE *pfile = fopen("../../Media/etc/Plants.bbi", "r+b") ;
	if(pfile)
	{
		fread(&m_nNumBillboardIndex, 4, 1, pfile) ;
		m_psBillboardIndex = new SBillboardIndex[m_nNumBillboardIndex] ;
		fread(m_psBillboardIndex, sizeof(SBillboardIndex), m_nNumBillboardIndex, pfile) ;
		fclose(pfile) ;
	}
}
void CSecretMassiveBillboard::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
bool CSecretMassiveBillboard::IsAttr(int nAttr)
{
	if(m_nAttr & nAttr)
		return true ;
	return false ;
}
void CSecretMassiveBillboard::SetShadowMap(CSecretShadowMap *pcShadowMap)
{
	m_pcShadowMap = pcShadowMap ;
}
void CSecretMassiveBillboard::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}
void CSecretMassiveBillboard::_BuildBillboardIndex()
{
	int c=0, nIndex=0, line_count=0 ;
	SBillboardIndex *psBillboardIndex = new SBillboardIndex[512] ;

	int xpos=0, ypos=0 ;
	int w=256, h=256 ;

	////MassiveBillboard01.dds
	psBillboardIndex[c++].set("00_soo_f1_17m_19824",    nIndex,    xpos,        ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_18m_19851",    nIndex,    xpos+=w,     ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_19m_20590",    nIndex,    xpos+=w,     ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_20mc_15010",   nIndex,    xpos+=w,     ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_20mc_16754",   nIndex,    xpos+=w,     ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_20m_15727",    nIndex,    xpos+=w,     ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_24m_16539",    nIndex,    xpos+=w,     ypos,     w,   h) ;
	psBillboardIndex[c++].set("00_soo_f1_25m_15199",    nIndex,    xpos+=w,     ypos,     w,   h) ;

	//line 2
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_f1_29mc_16758",   nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_f1_29m_17159",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_10m_15817",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_11m_14048",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_11m_14049",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_12m_15091",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_13m_13411",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_14m_19869",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//line 3
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_i1_15m_15817",    nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_16m_15697",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_4m_9921",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_5m_13989",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_6m_12823",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_7m_16260",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_8m_14049",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_i1_9m_15697",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//line 4
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_p1_7m_16144",     nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_p1_8m_16214",     nIndex,    xpos+=w,		ypos+37,   w,	   h-37) ;//256x219, -37
	psBillboardIndex[c++].set("00_soo_p1_8m_18612",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_p1_9m_16103",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_p1_9m_17775",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_p1_10m_16202",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_p1_10m_17726",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_p1_11m_15825",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//line 5
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_p1_12m_23209",    nIndex,    xpos,		ypos+37,   w,	   h-37) ;//256x219, -37
	psBillboardIndex[c++].set("00_soo_p1_13m_16437",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_s1_10m_9844",     nIndex,    xpos+=w,		ypos+76,   w,	   h-76) ;//-76, 256x180
	psBillboardIndex[c++].set("00_soo_s1_3m_9052",      nIndex,    xpos+=w,		ypos+181,  w,	   h-181) ;//-181, 256x75
	psBillboardIndex[c++].set("00_soo_s1_4m_9464",      nIndex,    xpos+=w,		ypos+116,  w,	   h-116) ;//-116, 256x116
	psBillboardIndex[c++].set("00_soo_s1_5m_9833",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_s1_7m_10412",     nIndex,    xpos+=w,		ypos+106,  w,	   h-106) ;//-106, 256x160

	psBillboardIndex[c++].set("00_soo_p1_5m_17726",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	//line 6
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_s2_11m_18763",    nIndex,    xpos,		ypos+121,  w,	   h-121) ;//-121
	psBillboardIndex[c++].set("00_soo_s2_9m_18696",    nIndex,    xpos+=w,		ypos+60,   w,	   h-60) ;//-60
	psBillboardIndex[c++].set("00_soo_s2_2m_8114",      nIndex,    xpos+=w,		ypos+161,  w,	   h-161) ;//-161
	psBillboardIndex[c++].set("00_soo_s2_7m_18693",     nIndex,    xpos+=w,		ypos+106,  w,	   h-106) ;//-106
	psBillboardIndex[c++].set("00_soo_s2_8m_10073",     nIndex,    xpos+=w,		ypos+66,   w,	   h-66) ;//-66
	psBillboardIndex[c++].set("00_soo_s2_12m_18696",     nIndex,    xpos+=w,		ypos+56,   w,	   h-56) ;//-56
	psBillboardIndex[c++].set("00_soo_s2_9m_18777",     nIndex,    xpos+=w,		ypos+101,  w,	   h-101) ;//-101
	psBillboardIndex[c++].set("00_soo_s3_13m_16989",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	//line 7
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_t1_12m_16527",    nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_t1_16m_25017",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_t1_17m_12177",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_t1_19m_15931",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_t1_20m_21256",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("00_soo_w1_120cm_6685",   nIndex,    xpos+=w, 	ypos+162,  w/2,	   h-162) ;
	psBillboardIndex[c++].set("00_soo_w1_1m50_5836",    nIndex,    xpos+=w/2,	ypos+146,  w/2,	   h-146) ;
	psBillboardIndex[c++].set("00_soo_w1_1m80_5668",    nIndex,    xpos+=w/2,	ypos+146,  w/2,	   h-146) ;
	psBillboardIndex[c++].set("00_soo_w1_1m_6685",      nIndex,    xpos+=w/2,	ypos+161,  w/2,	   h-161) ;

	psBillboardIndex[c++].set("00_soo_p1_6m_16437",    nIndex,    xpos+=w/2,	ypos,	   w,	   h) ;

	//line 8
	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("00_soo_y1_14m_12125",    nIndex,    xpos,		ypos+71,   w,	   h-71) ;
	psBillboardIndex[c++].set("00_soo_y1_7m_12125",     nIndex,    xpos+=w,		ypos+96,   w,	   h-96) ;
	psBillboardIndex[c++].set("00_soo_y2_10m_17300",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_y2_15m_21428",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_y2_17m_21477",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_y2_19m_23097",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_y2_20m_21461",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("00_soo_y2_9m_21380",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	nIndex++ ;//MassiveBillboard02.dds

	line_count++ ;
	xpos=0 ;
	ypos+=h ;	
	psBillboardIndex[c++].set("01_nue_o1_18m_11648_s",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_o2_18m_11648_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_o3_18m_11648_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_t1_3m_094",		   nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_t2_350c_094",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_t3_550c_094",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_t4_6m_094",		   nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_w1_15m_090",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;	
	psBillboardIndex[c++].set("01_nue_w2_12m_090",		   nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_w3_18m_090",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_w4_8m_090",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_w5_13m_090",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_w6_19m_090",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_y1_10m_09882_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_y2_10m_09365_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_y3_10m_09822_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;	
	psBillboardIndex[c++].set("01_nue_y6_10m_09365_f",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_y7_7m_098_s",		   nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("01_nue_y8_7m_093_f",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_o1_6m_151",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_o2_8m_151",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_o3_11m_151",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_o4_13m_151",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_t1_9m_190",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("10_gaa_t2_12m_190",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_t3_15m_190",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_t4_18m_190",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("14_buj_y3_12m_099",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("14_buj_y4_15m_099",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("05_kko_i1_15m_114",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("05_kko_i2_10m_114",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("05_kko_i3_13m_103",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	//line_count++ ;
	//xpos=1280 ;
	//ypos+=h ;	
	//psBillboardIndex[c++].set("05_kko_i1_15m_114",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("05_kko_i2_10m_114",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("05_kko_i3_13m_103",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("05_kko_i4_7m_064",      nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("06_chu_i1_15m_127",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("06_chu_i2_15m_126",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("06_chu_i3_15m_127",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("06_chu_i4_10m_127",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("06_chu_i5_7m_126",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("07_pla_y1_17m_13887",   nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_f1_15m_052",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("08_gye_f2_15m_075",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_f3_15m_081",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_f4_10m_075",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_f5_7m_052",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_f6_8m_081",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_i1_15m_111",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_i2_17m_103",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("08_gye_i3_10m_137",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("08_gye_i4_7m_137",		   nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t1_20m_06918_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t2_30m_11154_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t3_30m_11154_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t4_20m_06918_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t5_30m_11154_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t6_20m_06918_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("12_mee_t7_25m_11154_s",     nIndex,    512,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t8_25m_11154_f",     nIndex,    768,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_t9_25m_11154_s",     nIndex,    1280, 		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("12_mee_i1_20m_06918_s",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("12_mee_i2_30m_11154_s",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_i3_30m_11154_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_i4_20m_06918_f",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_i5_30m_11154_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_i6_20m_06918_s",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("12_mee_i7_25m_11154_s",     nIndex,    0,    		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_i8_25m_11154_f",     nIndex,    256,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("12_mee_i9_25m_11154_s",     nIndex,    768,  		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("18_met_i1_10m_079",		   nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_i1_12m_07906",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_i2_15m_07938",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	nIndex++ ;//MassiveBillboard03.dds

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("18_met_i3_20m_07978",     nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_i4_25m_15744",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_i5_30m_21318",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_t1_10m_079",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_t1_12m_07906",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_t2_15m_07938",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_t3_20m_07978",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("18_met_t4_25m_15744",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("18_met_t5_30m_21318",     nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_i1_18m_107",		 nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_i2_15m_095",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_i3_10m_080",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_i4_21m_119",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_i5_7m_095",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_i6_13m_107",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_t1_30m_099",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("21_hym_t2_30m_087",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("21_hym_t3_30m_099",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_f1_15m_051",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_f2_12m_040",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_f3_9m_034",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_f4_17m_051",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_f5_19m_051",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_i1_20m_087",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("22_jaj_i2_12m_087",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_i3_19m_087",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("22_jaj_i4_15m_087",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("23_chm_i1_15m_071",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("23_chm_i2_15m_107",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("23_chm_i3_15m_107",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("23_chm_i4_10m_107",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("23_chm_i5_7m_071",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("100_yaj_e1_16m_031",     nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e2_10m_031",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e3_6m_031",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e4_10m_026",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e5_7m_026",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e6_4m_026",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e7_8m_026",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e8_5m_026",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;	
	psBillboardIndex[c++].set("100_yaj_e9_3m_026",       nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_e10_10m_026",     nIndex,    xpos+=w, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_g1_20m_045",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_g2_15m_045",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_g3_8m_041",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_g4_3m_041",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("100_yaj_h1_30m_057",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("100_yaj_h2_25m_050",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("04_bud_s1_20m_13920",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("04_bud_s2_20m_13920",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	line_count++ ;
	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("100_yaj_h3_15m_050",     nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_h4_8m_050",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("100_yaj_h5_30m_071",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("04_bud_y1_17m_12134",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("04_bud_y2_25m_13280",     nIndex,    1536,       1792,	   w,	   h) ;
	psBillboardIndex[c++].set("04_bud_y3_12m_06494",     nIndex,    1792,		1792,	   w,	   h) ;

	w=h=128 ;
	xpos=768 ;
	psBillboardIndex[c++].set("03_dan_o1_6m_10950",      nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_o3_10m_10962",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y10_5m_10440",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y11_8m_10908",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y12_7m_10854",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y13_10m_10910",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y14_6m_106",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y15_280c_10412",   nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y1_12m_10926",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y2_4m_10888",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=768 ;
	ypos+=h ;
	psBillboardIndex[c++].set("03_dan_y3_7m_10606",     nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y4_7m_10020",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y5_4m_09792",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y6_5m_09938",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y7_5m_10450",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y8_4m_09926",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("03_dan_y9_8m_10412",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("09_mok_i1_6m_13032",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("09_mok_i2_7m_13420",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("09_mok_y3_6m_12294",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("09_mok_y5_5m_13320",     nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("09_mok_y6_6m_12294",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_i2_8m_09300",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_i4_5m_04156",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("10_gaa_i5_3m_03776",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("11_hya_a1_6m_171",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_a2_9m_171",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_b1_5m_164",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("empty",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("11_hya_b2_8m_164",        nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_i1_6m_16263",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_i3_3m_08228",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_t1_10m_066",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_t2_14m_066",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("11_hya_t3_17m_066",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("14_buj_y1_7m_098",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("14_buj_y2_9m_098",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",				     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("empty",                   nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	psBillboardIndex[c++].set("15_jag_y1_12m_191",     nIndex,    1280,		1792,	   w,	   h) ;
	psBillboardIndex[c++].set("15_jag_y2_16m_192",       nIndex,    1408,		1792,	   w,	   h) ;
	//psBillboardIndex[c++].set("15_jag_y1_12m_19192",     nIndex,    xpos+=384,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("15_jag_y2_16m_192",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	nIndex++ ;//MassiveBillboard04.dds
	xpos=ypos=0 ;
	psBillboardIndex[c++].set("19_chi_i1_6m_134",      nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("19_chi_i2_10m_182",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i10_3m_074",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i11_2m_074",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i12_1m_074",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i1_11m_066",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i2_11m_066",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i3_8m_074",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i4_8m_074",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i5_8m_074",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i6_8m_074",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i7_8m_066",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i8_6m_066",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("24_hit_i9_4m_074",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("13_dae_i1_5m_017",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("13_dae_i2_350c_017",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("13_dae_i3_2m_015",         nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("13_dae_o1_1m_005",         nIndex,    xpos+=w,		ypos,	   w,	   h-64) ;
	psBillboardIndex[c++].set("13_dae_o2_1m_008",         nIndex,    xpos,  		ypos+64,   w,	   h-64) ;
	psBillboardIndex[c++].set("13_dae_t1_2m_015",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("17_odo_i1_5m_06144",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("17_odo_i2_2m_03558",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("17_odo_i3_3m_03256",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("17_odo_i4_8m_061",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_o1_130c_06213",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_o2_130c_06213",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_o3_130c_06213",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_t1_130c_08112",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_t2_130c_08112",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_t3_130c_08112",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_y1_2m_07552",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_y2_2m_07552",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_y3_2m_07552",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("32_moo_y4_2m_07552",        nIndex,    xpos,     	ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_y5_2m_07552",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("32_moo_y6_2m_07552",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("41_hon_i1_2m_09668",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("41_hon_i2_60c_04484",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("16_bae_i13_250c_01204",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("16_bae_i23_2m_00458",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("16_bae_i5_150c_00500",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("16_bae_i9_2m_00846",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("16_bae_w1_250c_10284",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("16_bae_w2_250c_05416",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("31_kae_x1_2m_030",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("31_kae_x2_2m_031",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("31_kae_x3_2m_032",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("31_kae_x4_2m1x4_187",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	//psBillboardIndex[c++].set("31_kae_x5_2m1x10_468",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("34_jop_x1_150c1x1_039",     nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("34_jop_x2_150c1x1_039",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("34_jop_x3_1m1x1_038",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("34_jop_x4_1m1x1_038",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("34_jop_x5_150c1x4_235",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("34_jop_x6_150c1x10_511",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_i12_60c_00562",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_i15_60c_00562",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_i18_60c_00562",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_i1_60c_00562",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_i4_60c_00554",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_i9_60c_00562",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x1_60c1x1_060",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x2_60c1x1_060",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x3_60c1x1_060",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x4_60c1x1_060",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("51_jan_x5_60c1x1_060",     nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x6_60c1x1_060",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x7_60c1x4_242",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x8_60c1x4_240",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("51_jan_x9_60c1x4_241",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i10_60c_00892",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i11_60c_00892",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i12_60c_00892",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i1_60c_00584",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i2_60c_00584",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i3_60c_00584",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i4_60c_00584",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_i9_60c_00892",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_x1_60c1x1_027",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_x2_60c1x1_027",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("52_hap_x3_60c1x1_027",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("52_hap_x4_60c1x1_027",     nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_i37_40c_00398",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_i38_40c_00398",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_i3_50c_00148",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_i41_70c_00410",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_i42_70c_00410",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_i4_50c_00148",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_x1_70c1x1_033",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_x2_70c1x1_033",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_x3_70c1x3_100",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("53_rud_x4_70c1x3_100",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_i1_1m_00008",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_i5_1m_00020",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_i6_1m_00020",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_i7_1m_00020",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_o1_1m_05456",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("62_chl_o2_1m_05456",       nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_o3_1m_05456",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t1_80c_099",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t2_150c_099",       nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t3_2m_123",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t4_80c_099",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t5_150c_099",       nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t6_2m_123",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t7_80c_099",        nIndex,    xpos+=w,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t8_150c_099",       nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_t9_2m_123",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x11_1m1x4_023",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x12_1m1x4_021",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x13_1m1x4_023",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x15_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x16_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x17_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x18_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x19_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	
	xpos=0 ;
	ypos+=h ;	
	psBillboardIndex[c++].set("62_chl_x1_1m1x1_004",      nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x20_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x21_1m1x2_007",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x2_1m1x1_004",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x3_1m1x1_004",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x4_1m1x1_005",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x5_1m1x1_005",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x6_1m1x1_005",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("62_chl_x7_1m1x1_005",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("65_nng_o1_1m_14256",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("65_nng_o2_1m_13690",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("65_nng_o3_150c_136",		  nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("74_jar_v1_60c_01046",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("74_jar_v2_60c_01046",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("74_jar_v3_60c_01046",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("74_jar_x1_60c1x1_026",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("74_jar_x2_60c1x1_030",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;	
	psBillboardIndex[c++].set("74_jar_x3_60c1x1_030",       nIndex,    xpos,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_f2_80c_00860",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_f3_70c_00324",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_i1_80c_03202",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_o1_50c_00960",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_o2_50c_00960",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_o3_50c_00960",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_o4_50c_00960",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("75_okj_o5_50c_00960",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("61_kkw_o2_150c_02595",       nIndex,    xpos+=w,		ypos+46,   w,	   h-46) ;
	psBillboardIndex[c++].set("61_kkw_o8_150c_02595",       nIndex,    xpos+=w,		ypos+46,   w,	   h-46) ;
	psBillboardIndex[c++].set("61_kkw_x2_40c03x2_008",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("61_kkw_x5_40c03x1_004",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("61_kkw_x6_30c03x03_002",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("61_kkw_x7_80c06x4_008",      nIndex,    xpos-512,	ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_a1_70c_023",			nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_a2_70c_023",			nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("66_soo_a3_70c_023",      nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_a4_70c_023",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_b1_70c_031",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_b2_70c_031",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_b3_70c_031",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_b4_70c_031",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_b5_70c_031",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_c1_1m_072",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_c2_1m_072",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_c3_1m_072",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_t1_2m_151",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_t2_4m_151",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_t3_250c_151",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("66_soo_t4_350c_151",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_m1_250c2x6_235",  nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_m2_250c2x6_188",  nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("69_top_o1_230c_09844",      nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_o2_230c_09844",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_o3_230c_09844",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_w1_2m_09930",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_w2_2m_10060",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_w3_2m_10060",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_w4_2m_10060",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_w5_2m_09930",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("69_top_w6_2m_09930",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("81_baw_x1_1x1_011",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("81_baw_x2_3x3_086",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("81_baw_x3_1x1_011",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("81_baw_x4_3x3_086",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i1_1m_002",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i1_2m_011",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i1_30c_001",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("82_dam_i1_50c_001",         nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i1_70c_005",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("70_dir_1m_001",             nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("70_dir_50c_001",            nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("71_gal_t2_150c_00600",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("71_gal_x1_150c1x2_018",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("71_gal_x2_150c1x2_018",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("71_gal_x3_150c2x4_032",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("71_gal_x4_150c2x10_074",    nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_o2_1m_118",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_o3_60c_118",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_o4_1m_118",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_o5_1m_118",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_o6_150c_118",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_o7_80c_118",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_t1_180c2x6_217",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("77_eok_w1_30c_00128",       nIndex,    xpos, 		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_w2_30c_00128",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_w3_30c_00128",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_w5_1m_00496",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("77_eok_w6_1m_00702",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("91_boo_i1_150c_00108",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("91_boo_v1_150c_00864",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("91_boo_x1_150c1x1_012",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x1_120c1x1_013",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x2_120c1x1_013",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x3_120c1x2_028",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x4_2m2x2_015",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x5_2m2x2_016",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x6_2m2x4_031",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x7_120c1x6_084",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("79_mul_x8_2m2x10_094",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("92_yon_i1_50c_00032",     nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("92_yon_i4_60c_00032",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("92_yon_i6_60c_00204",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("92_yon_w1_60c_00958",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("92_yon_w3_side_003",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_v1_1m_00945",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_v2_1m_00945",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_v3_1m_00945",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_v4_1m_00945",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_v5_1m_00945",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_v6_1m_00945",      nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_x1_1m1x1_016",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_x2_1m1x1_017",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_x3_1m1x1_013",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_x4_1m1x1_015",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("96_cha_x5_1m1x1_017",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("96_cha_x6_1m1x1_017",     nIndex,    xpos,   		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_c1_5m_015",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_c2_3m_015",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_c3_150c_015",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_d1_6m_076",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_d2_5m_074",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_d3_250c_072",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_d4_1m_072",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_a1_130c_008",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_a2_150c_008",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_b1_3m_150",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_b2_230c_150",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_b3_3m_032",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("100_yaj_b4_2m_032",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_a1_4m_111",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_a2_3m_111",       nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	xpos=0 ;
	ypos+=h ;
	psBillboardIndex[c++].set("104_yan_i1_3m_010",         nIndex,    xpos,  		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("104_yan_x1_4m_077",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("104_yan_x2_4m_096",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("104_yan_x3_4m_115",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("104_yan_y1_4m_026",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("104_yan_y2_4m_032",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_b1_30c_005",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_b2_30c_005",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_b3_50c_006",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_b4_70c_008",        nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("101_yak_b5_70c2x2_056",     nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i2_1m_001",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i2_2m_009",          nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i2_30c_001",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i2_50c_001",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;
	psBillboardIndex[c++].set("82_dam_i2_70c_005",         nIndex,    xpos+=w,		ypos,	   w,	   h) ;

	FILE *pfile = fopen("../../Media/etc/Plants.bbi", "w+b") ;
	if(pfile)
	{
		fwrite(&c, 4, 1, pfile) ;
		fwrite(psBillboardIndex, sizeof(SBillboardIndex), c, pfile) ;
		fclose(pfile) ;
	}

	SAFE_DELETEARRAY(psBillboardIndex) ;
}

////////////////////////////////////////////////////////////
//CSecretMassiveBillboardEffect
////////////////////////////////////////////////////////////
CSecretMassiveBillboardEffect::CSecretMassiveBillboardEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_hTechnique = NULL ;
}
CSecretMassiveBillboardEffect::~CSecretMassiveBillboardEffect()
{
	Release() ;
}
void CSecretMassiveBillboardEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}
HRESULT CSecretMassiveBillboardEffect::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)
{
	HRESULT hr ;
	LPD3DXBUFFER pError=NULL ;
	DWORD dwShaderFlag=NULL ;

#ifdef _SHADERDEBUG_
	dwShaderFlag = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION ;//|D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT|D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT|D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION|D3DXSHADER_SKIPVALIDATION ;
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, dwShaderFlag, NULL, &m_pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#else
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, 0, NULL, &m_pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#endif
	else
	{
		if((m_hTechnique = m_pEffect->GetTechniqueByName("TShader")) == NULL)
		{
			assert(false && "hTechnique is NULL (TShader)") ;
			return E_FAIL ;
		}
 	}
	SAFE_RELEASE(pError) ;

	SetD3DXHANDLE(&m_hmatWVP, "matWVP") ;
	SetD3DXHANDLE(&m_hmatWLP, "matWLP") ;
	SetD3DXHANDLE(&m_hmatWLPT, "matWLPT") ;
	SetD3DXHANDLE(&m_hvCameraPos, "vCameraPos") ;
	SetD3DXHANDLE(&m_hvLightDir, "vLightDir") ;
	SetD3DXHANDLE(&m_hvLightColor, "vLightColor") ;
	SetD3DXHANDLE(&m_hAtm, "atm") ;
	SetD3DXHANDLE(&m_hvLookat, "vLookat") ;

	return S_OK ;
}
bool CSecretMassiveBillboardEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
{
	if((*pHandle = m_pEffect->GetParameterByName(NULL, pszName)) == NULL)
	{
		char err[128] ;
		sprintf(err, "%s Handle is NULL", pszName) ;
		assert(false && err) ;
		return false ;
	}
	return true ;
}