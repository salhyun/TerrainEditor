#include <assert.h>
#include "SecretAOGenerator.h"
#include "ASELoader.h"
#include "ASEData.h"
#include "SecretMeshObject.h"

CSecretAOGenerator::CSecretAOGenerator()
{
	m_pd3dDevice = NULL ;
	m_pObjectQuery = NULL ;
	m_pcASELoader = NULL ;
	m_pcASEData = NULL ;
	m_pTexBuffer = NULL ;
	m_pSurfBuffer = NULL ;
	m_nTexSize = 128 ;
	m_dwColorKey = 0 ;
}
CSecretAOGenerator::~CSecretAOGenerator()
{
	Release() ;
}
void CSecretAOGenerator::Release()
{
	SAFE_RELEASE(m_pSurfBuffer) ;
	SAFE_RELEASE(m_pTexBuffer) ;
	SAFE_RELEASE(m_pSurfZBuffer) ;

	SAFE_DELETE(m_pcASEData) ;
	SAFE_DELETE(m_pcASELoader) ;
	SAFE_RELEASE(m_pObjectQuery) ;
	SAFE_DELETE(m_pcObject) ;
}
bool CSecretAOGenerator::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTextureContainer *pcTexContainer, SD3DEffect *psEffect)
{
	m_pd3dDevice = pd3dDevice ;
	m_psEffect = psEffect ;
	m_pcTexContainer = pcTexContainer ;

	if( D3DERR_NOTAVAILABLE == m_pd3dDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, NULL ) )
	{
		assert("couldn't create query object!" && false) ;
		return false ;
	}

	m_pd3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_pObjectQuery) ;

	m_dwColorKey = 0x00ff00ff ;	
	m_nTexSize = 128 ;
	if(FAILED(m_pd3dDevice->CreateTexture(m_nTexSize, m_nTexSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexBuffer, NULL)))
	{
		assert("couldn't create TexBuffer!" && false) ;
		return false ;
	}
	if(FAILED(m_pTexBuffer->GetSurfaceLevel(0, &m_pSurfBuffer)))
	{
		assert("couldn't create SurfBuffer!" && false) ;
		return false ;
	}
	if(FAILED(m_pd3dDevice->CreateDepthStencilSurface(m_nTexSize, m_nTexSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pSurfZBuffer, NULL)))
	{
		assert("couldn't create DepthStencilSurface!" && false) ;
		return false ;
	}

	m_pcASELoader = new CASELoader() ;
	m_pcASEData = new CASEData() ;

	m_pcObject = new CSecretMeshObject() ;
	m_pcObject->InitMemberVariable(m_pd3dDevice, 1) ;

    return true ;
}
void CSecretAOGenerator::LoadASE(char *pszFileName, CSecretObjectScatteringData *pcAtm)
{
	if(!m_pcASELoader->ReadASEFile(pszFileName, m_pcASEData))
		return ;

	m_pcASEData->WeldVertex() ;

	m_pcASEData->m_apMesh[m_pcASEData->m_nNumMesh] = _AddGroundMesh(m_pcASEData->m_vMax.x-m_pcASEData->m_vMin.x) ;
	m_pcASEData->m_nNumMesh++ ;

	m_pcObject->InitObject(m_pcASEData, "../../Media/map/", m_pcTexContainer, SECRETNODEATTR_SUNLIT) ;

	char str[256] ;
	GetFileNameFromPath(str, pszFileName) ;
	m_pcObject->SetObjName(str) ;
	m_pcObject->m_pcAtm = pcAtm ;
	m_pcObject->SetWireFrame(false) ;

	sprintf(m_szFileName, "%s", pszFileName) ;
}
float CSecretAOGenerator::_CalculateAmbientDegree(D3DXVECTOR3 *pvPos, D3DXVECTOR3 *pvLookat, D3DXVECTOR3 *pvUp)
{
	//카메라 위치를 각각 버텍스에다가 위치시키고
	//뷰행렬 설정
	D3DXMatrixLookAtLH(&m_matView, pvPos, pvLookat, pvUp) ;

	D3DXMATRIX matWorld, matWVP, matIdentity ;
	D3DXMatrixIdentity(&matWorld) ;
	D3DXMatrixIdentity(&matIdentity) ;

	matWVP = matWorld * m_matView * m_matProj ;
	m_psEffect->pEffect->SetMatrix(m_psEffect->hmatWVP, &matWVP) ;

	//카메라 위치, 뷰행렬, 투영행렬 교체
	D3DXMATRIX matOldView, matOldProj ;
	D3DXVECTOR4 vOldCameraPos ;

	vOldCameraPos = m_psEffect->vCameraPos ;
	matOldView = m_psEffect->matView ;
	matOldProj = m_psEffect->matProj ;

	m_psEffect->vCameraPos.x = pvPos->x ;
	m_psEffect->vCameraPos.y = pvPos->y ;
	m_psEffect->vCameraPos.z = pvPos->z ;

	m_psEffect->matView = m_matView ;
	m_psEffect->matProj = m_matProj ;

	//렌더링 하고
	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, m_dwColorKey, 1.0f, 0L );
	if(SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		m_psEffect->pEffect->SetTechnique(m_psEffect->hTechnique) ;
		m_psEffect->pEffect->Begin(NULL, 0) ;

		DWORD dwZEnable, dwZWriteEnable, dwZFunc, dwAlphaTest, dwAlphaBlend ;

		m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
		m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend) ;

		m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
		m_pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWriteEnable) ;
		m_pd3dDevice->GetRenderState(D3DRS_ZFUNC, &dwZFunc) ;

		m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x000000bf) ;//cut off greater that 0.75
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

		m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE) ;
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO) ;

		//m_pcObject->Render(m_psEffect, &matWorld) ;
		CSecretMesh *pcMesh ;
		for(int i=0 ; i<m_pcObject->GetNumMesh() ; i++)
		{
			pcMesh = m_pcObject->GetMesh(i) ;
			if( strstr(pcMesh->GetNodeName(), "fur") )
				continue ;

			m_pcObject->Render(m_psEffect, pcMesh, &matIdentity) ;
		}

		m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000) ;
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

		m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
		m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;

		m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) ;
		m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS) ;

		//m_pcObject->Render(m_psEffect, &matWorld) ;
		for(int i=0 ; i<m_pcObject->GetNumMesh() ; i++)
		{
			pcMesh = m_pcObject->GetMesh(i) ;
			if( strstr(pcMesh->GetNodeName(), "fur") )
				continue ;

			m_pcObject->Render(m_psEffect, pcMesh, &matIdentity) ;
		}

		//m_pObjectQuery->Issue(D3DISSUE_BEGIN) ;

		//m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
		//m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x000000bf) ;//cut off greater that 0.75
		//m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

		//m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
		//m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE) ;
		//m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO) ;

		//m_pcObject->Render(m_psEffect, &matWorld) ;

		//m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000) ;
		//m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

		//m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
		//m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;

		//m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) ;
		//m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS) ;

		//m_pcObject->Render(m_psEffect, &matWorld) ;

		//m_pObjectQuery->Issue(D3DISSUE_END) ;

		m_psEffect->pEffect->End() ;

		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
		m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWriteEnable) ;
		m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, dwZFunc) ;

		m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
		m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend) ;
	}
	m_pd3dDevice->EndScene() ;

	m_psEffect->vCameraPos = vOldCameraPos ;
	m_psEffect->matView = matOldView ;
	m_psEffect->matProj = matOldProj ;

	//쿼리값 받아서 저장하고
	//DWORD dwQueryData ;
	//while(m_pObjectQuery->GetData((void *)&dwQueryData, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE)
	//{//Additional Work
	//}

	//if(dwQueryData > (DWORD)(m_nTexSize*m_nTexSize))
	//{
	//	m_psEffect->vCameraPos = vOldCameraPos ;
	//	m_psEffect->matView = matOldView ;
	//	m_psEffect->matProj = matOldProj ;

	//	TRACE("QueryData is screw up %d\r\n", dwQueryData) ;
	//	return (float)dwQueryData ;
	//}

	//static int count=0 ;
	//float fTotalQuery=(float)(m_nTexSize*m_nTexSize), fAmbientDegree ;
	//fAmbientDegree = 1.0f - ((float)dwQueryData/fTotalQuery) ;
	////TRACE("[%04d]vertex ambientdegree=%06.04f\r\n", count, fAmbientDegree) ;
	////TRACE("[%04d]vertex ambientdegree=%06.04f\r\n", count, (float)dwQueryData) ;

	//g_cDebugMsg.SetDebugMsg(4, "1-%03.01f/%03.01f = %03.01f\r\n", (float)dwQueryData, fTotalQuery, fAmbientDegree) ;

	////TRACE("[%04d] pos(%07.03f %07.03f %07.03f), AmbientDegree : 1-%07.03f/%07.03f = %07.03f\r\n", count, enumVector((*pvPos)), (float)dwQueryData, fTotalQuery, fAmbientDegree) ;
	////TRACE("[%04d] pos(%07.03f %07.03f %07.03f)\r\n", count, enumVector((*pvPos))) ;
	////count++ ;

	//if(fAmbientDegree < 0.0f)
	//	fAmbientDegree = 0.0f ;

	return 0 ;
}
void CSecretAOGenerator::_Render()
{
	int i, n, count ;
	SMesh *psMesh ;
	D3DXVECTOR4 vRet ;
	D3DXVECTOR3 vPos, vLookat, vUp ;
	D3DXMATRIX d3dmat ;

	for(i=0 ; i<m_pcASEData->m_nNumMesh ; i++)
	{
		psMesh = m_pcASEData->m_apMesh[i] ;
		if( strstr(psMesh->szMeshName, "fur") )
		{
			TRACE("\r\n") ;
			TRACE("### fur mesh continue ###\r\n") ;
			TRACE("\r\n") ;
			continue ;
		}

		count = 0 ;
		TRACE("\"%s\" Mesh start\r\n", psMesh->szMeshName) ;
		for(n=0 ; n<psMesh->nNumVertex ; n++, count++)
		{
			vPos = D3DXVECTOR3(psMesh->psVertex[n].pos.x, psMesh->psVertex[n].pos.y, psMesh->psVertex[n].pos.z) ;
			vLookat = D3DXVECTOR3(psMesh->psVertex[n].normal.x, psMesh->psVertex[n].normal.y, psMesh->psVertex[n].normal.z) ;
			vLookat = vPos+vLookat ;

			//vPos = D3DXVECTOR3(0, 1, 0) ;
			//vLookat = D3DXVECTOR3(0, 2, 0) ;

			memcpy((void *)&d3dmat, (void *)&psMesh->smatLocal, sizeof(D3DXMATRIX)) ;
            D3DXVec3Transform(&vRet, &vPos, &d3dmat) ;
			vPos.x = vRet.x ;
			vPos.y = vRet.y ;
			vPos.z = vRet.z ;

			//D3DXVec3Transform(&vRet, &vLookat, &d3dmat) ;
			//vLookat.x = vRet.x ;
			//vLookat.y = vRet.y ;
			//vLookat.z = vRet.z ;

			Vector3 vOriginLookat(0, 0, 1), vOriginUp(0, 1, 0) ;
			Vector3 vNormal(psMesh->psVertex[n].normal.x, psMesh->psVertex[n].normal.y, psMesh->psVertex[n].normal.z) ;
			Matrix4 mat, matRot ;
			memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
			mat = mat.DecomposeRotation() ;
			vNormal *= mat ;
			vNormal = vNormal.Normalize() ;

			vLookat.x = vPos.x+vNormal.x ;
			vLookat.y = vPos.y+vNormal.y ;
			vLookat.z = vPos.z+vNormal.z ;

			Quaternion q1 ;
			if(vector_eq(vNormal, vOriginLookat))
			{
				matRot.Identity() ;
				//TRACE("same vector\r\n") ;
			}
			else if(float_eq((vOriginLookat+vNormal).Magnitude(), 0.0f))
			{
				matRot.Identity() ;
				matRot.m11 = matRot.m22 = matRot.m33 = -1 ;
				//TRACE("negative vector\r\n") ;
			}
			else
			{
				q1.TwoVectorsToQuat(vOriginLookat, vNormal) ;
				q1.GetMatrix(matRot) ;
			}
			vOriginUp *= matRot ;
			vUp.x = vOriginUp.x ;
			vUp.y = vOriginUp.y ;
			vUp.z = vOriginUp.z ;

   //         vPos = D3DXVECTOR3(-2.3692f, 90.6087f, 4.1466f) ;
			//vLookat = D3DXVECTOR3(-3.13892f, 90.7434f, 3.52259f) ;
			//vUp = D3DXVECTOR3(0.275762f, 0.951741f, -0.134703f) ;

			//if((n == 40) && !strcmp(psMesh->szMeshName, "Cylinder02"))
			psMesh->psVertex[n].fAmbientDegree = _CalculateAmbientDegree(&vPos, &vLookat, &vUp) ;

			LPDIRECT3DTEXTURE9 pTex=NULL ;
            LPDIRECT3DSURFACE9 pSurf=NULL ;
			m_pd3dDevice->CreateTexture(m_nTexSize, m_nTexSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pTex, NULL) ;
			pTex->GetSurfaceLevel(0, &pSurf) ;
			if(FAILED(m_pd3dDevice->GetRenderTargetData(m_pSurfBuffer, pSurf)))
				int a1=0 ;

			int x, y ;
			D3DLOCKED_RECT d3drect ;
			pSurf->LockRect(&d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
			DWORD *pdw = (DWORD *)d3drect.pBits ;
			float ao=0.0f ;
			for(y=0 ; y<m_nTexSize ; y++)
			{
				for(x=0 ; x<m_nTexSize ; x++, pdw++)
				{
					if((*pdw) != m_dwColorKey)
						ao += 1.0f ;
				}
			}
			pSurf->UnlockRect() ;

			SAFE_RELEASE(pSurf) ;
			SAFE_RELEASE(pTex) ;

            psMesh->psVertex[n].fAmbientDegree = 1.0f - ao/(float)(m_nTexSize*m_nTexSize) ;

			if(float_greater(psMesh->psVertex[n].fAmbientDegree, 1.0f) || float_less(psMesh->psVertex[n].fAmbientDegree, 0.0f))
			{
				TRACE("screw up\r\n") ;
				TRACE("[%04d] pos(%07.03f %07.03f %07.03f), AmbientDegree=%07.03f\r\n", count, enumVector(vPos), psMesh->psVertex[n].fAmbientDegree) ;
			}
			//return ;
		}
		TRACE("\"%s\" Mesh end\r\n", psMesh->szMeshName) ;
	}
}
void CSecretAOGenerator::Process()
{
	D3DXMATRIX mat, matOldView, matOldProj ;
	D3DVIEWPORT9 OldViewport, Viewport = {0, 0, m_nTexSize, m_nTexSize, 0.1f, 1.0f} ;
	m_pd3dDevice->GetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matOldProj) ;

	//store the current back buffer and z-buffer
	LPDIRECT3DSURFACE9 pSurfBack, pSurfZBuffer ;
	m_pd3dDevice->GetRenderTarget(0, &pSurfBack) ;
	m_pd3dDevice->GetDepthStencilSurface(&pSurfZBuffer) ;
	m_pd3dDevice->GetViewport(&OldViewport) ;

	//투영행렬 설정 180도 각도
	D3DXMatrixPerspectiveFovLH(&m_matProj, 3.141592f/2.f, 1.0f, 0.1f, 1024.f) ;

	m_pd3dDevice->SetViewport(&Viewport) ;
	m_pd3dDevice->SetRenderTarget(0, m_pSurfBuffer) ;
	m_pd3dDevice->SetDepthStencilSurface(m_pSurfZBuffer) ;

	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, m_dwColorKey, 1.0f, 0L );

	//Rendering Object
	_Render() ;

	//Restore RenderTarget
	m_pd3dDevice->SetRenderTarget(0, pSurfBack) ;
	m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
	m_pd3dDevice->SetViewport(&OldViewport) ;


	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, m_dwColorKey, 1.0f, 0L );

	SAFE_RELEASE(pSurfBack) ;
	SAFE_RELEASE(pSurfZBuffer) ;

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOldProj) ;
}
void CSecretAOGenerator::ExportRMD()
{
	char *pstr1=NULL, *pstr2=NULL, str[256], szName[256] ;
	int l ;

	SAFE_DELETE(m_pcASEData->m_apMesh[m_pcASEData->m_nNumMesh-1]) ;
	m_pcASEData->m_nNumMesh-- ;

	pstr1 = m_szFileName ;
	pstr2 = strrchr(m_szFileName, '.') ;
	l = (int)(pstr2-pstr1) ;
	memcpy(str, pstr1, l) ;
	str[l] = '\0' ;
	sprintf(szName, "%s.RMD", str) ;

	m_pcASEData->ExportRMD(szName) ;    
}
SMesh *CSecretAOGenerator::_AddGroundMesh(float fGroundSize)
{
    SMesh *psMesh = new SMesh ;

	psMesh->MeshKind = SMESHKIND_RIGID ;
	psMesh->nAttr |= SMESHATTR_NOTEXTURE ;
	sprintf(psMesh->szMeshName, "GroundAOPlane") ;
	psMesh->nNumVertex = 4 ;
	psMesh->psVertex = new SMeshVertex[psMesh->nNumVertex] ;

	psMesh->psVertex[0].pos.set(-fGroundSize, 0, fGroundSize) ;
	psMesh->psVertex[0].normal.set(0, 1, 0) ;
	psMesh->psVertex[0].tex.set(0, 0) ;

	psMesh->psVertex[1].pos.set(fGroundSize, 0, fGroundSize) ;
	psMesh->psVertex[1].normal.set(0, 1, 0) ;
	psMesh->psVertex[1].tex.set(1, 0) ;

    psMesh->psVertex[2].pos.set(-fGroundSize, 0, -fGroundSize) ;
	psMesh->psVertex[2].normal.set(0, 1, 0) ;
	psMesh->psVertex[2].tex.set(0, 1) ;

    psMesh->psVertex[3].pos.set(fGroundSize, 0, -fGroundSize) ;
	psMesh->psVertex[3].normal.set(0, 1, 0) ;
	psMesh->psVertex[3].tex.set(1, 1) ;

	psMesh->nNumTriangle = 2 ;

	psMesh->psTriangle = new SMeshTriangle[psMesh->nNumTriangle] ;

	psMesh->psTriangle[0].anVertexIndex[0] = 0 ;
	psMesh->psTriangle[0].anVertexIndex[1] = 1 ;
	psMesh->psTriangle[0].anVertexIndex[2] = 2 ;
	psMesh->psTriangle[0].asVertexNormal[0].set(0, 1, 0) ;
	psMesh->psTriangle[0].asVertexNormal[1].set(0, 1, 0) ;
	psMesh->psTriangle[0].asVertexNormal[2].set(0, 1, 0) ;
	psMesh->psTriangle[0].sFaceNormal.set(0, 1, 0) ;
	psMesh->psTriangle[0].asTexCoord[0].set(0, 0) ;
	psMesh->psTriangle[0].asTexCoord[1].set(1, 0) ;
	psMesh->psTriangle[0].asTexCoord[2].set(0, 1) ;

	psMesh->psTriangle[1].anVertexIndex[0] = 3 ;
	psMesh->psTriangle[1].anVertexIndex[1] = 2 ;
	psMesh->psTriangle[1].anVertexIndex[2] = 1 ;
	psMesh->psTriangle[1].asVertexNormal[0].set(0, 1, 0) ;
	psMesh->psTriangle[1].asVertexNormal[1].set(0, 1, 0) ;
	psMesh->psTriangle[1].asVertexNormal[2].set(0, 1, 0) ;
	psMesh->psTriangle[1].sFaceNormal.set(0, 1, 0) ;
	psMesh->psTriangle[1].asTexCoord[0].set(1, 1) ;
	psMesh->psTriangle[1].asTexCoord[1].set(0, 1) ;
	psMesh->psTriangle[1].asTexCoord[2].set(1, 0) ;

	psMesh->smatLocal.m11 = psMesh->smatLocal.m22 = psMesh->smatLocal.m33 = 1 ;
	psMesh->smatLocal.m42 = -0.25f ;
	psMesh->smatLocal.m44 = 1 ;
	psMesh->smatChild = psMesh->smatLocal ;

	return psMesh ;
}