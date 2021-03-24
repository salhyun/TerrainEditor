#include <assert.h>
#include "def.h"
#include "ASEData.h"
#include "DNode.h"
#include "Matrix.h"
#include "Vector3.h"
#include "Vector4.h"
#include "GeoLib.h"

#include <d3d9.h>
#include <d3dx9.h>

#define TYPERMD 0x646d72
#define RMDVER 20090204

UINT ltemp ;
int nNumVisibleVector = 481 ;
Vector3 avVisibleVectors[] ;//481

CASEData::CASEData()
{
	m_lAttr = 0 ;
	m_lSubAttr = 0 ;
	m_nNumMaterial = 0 ;
	m_psMaterial = NULL ;
	m_psTriangles = NULL ;

	m_nNumMesh = 0 ;
	m_nMeshNode = 0 ;
	m_bUseDummyObject = false ;

	int i ;
	m_nNumBone = 0 ;
	for(i=0 ; i<MAXNUM_BONE ; i++)
		m_apszBoneName[i] = NULL ;

	for(i=0 ; i<MAXNUM_ASEDATAMESH ; i++)
		m_apMesh[i] = NULL ;

	m_nNumShapeObject = 0 ;
    for(i=0 ; i<MAXNUM_SHAPEOBJECT ; i++)
		m_apsShapeObject[i] = NULL ;
}
CASEData::~CASEData()
{
	Release() ;
}
void CASEData::Release()
{
	int i ;

	if(m_psMaterial)
	{
		delete []m_psMaterial ;
		m_psMaterial = NULL ;
	}
	m_nNumMaterial = 0 ;

	for(i=0 ; i<m_nNumMesh ; i++)
	{
		delete m_apMesh[i] ;
		m_apMesh[i] = NULL ;
	}
	m_nNumMesh = 0 ;
	m_nMeshNode = 0 ;

	for(i=0 ; i<m_nNumBone ; i++)
	{
		if(m_apszBoneName[i])
		{
			delete []m_apszBoneName[i] ;
			m_apszBoneName[i] = NULL ;
		}
	}
	m_nNumBone = 0 ;

	if(m_psTriangles)
	{
		delete []m_psTriangles ;
		m_psTriangles = NULL ;
	}

	for(i=0 ; i<m_nNumShapeObject ; i++)
	{
		delete m_apsShapeObject[i] ;
		m_apsShapeObject[i] = NULL ;
	}

	m_lAttr = m_lSubAttr = 0 ;
	m_bUseDummyObject = false ;
	m_nNumMaterial = m_nNumMesh = m_nMeshNode = 0 ;	
	memset(&m_sSceneInfo, 0, sizeof(SSceneInfo)) ;
}

bool IdentityVertex(void *pv1, void *pv2)
{
	SMeshVertex *pVertex1, *pVertex2 ;
	pVertex1 = (SMeshVertex *)pv1 ;
	pVertex2 = (SMeshVertex *)pv2 ;

	if(float_eq(pVertex1->pos.x, pVertex2->pos.x)
		&& float_eq(pVertex1->pos.y, pVertex2->pos.y)
		&& float_eq(pVertex1->pos.z, pVertex2->pos.z)

		&& float_eq(pVertex1->normal.x, pVertex2->normal.x)
		&& float_eq(pVertex1->normal.y, pVertex2->normal.y)
		&& float_eq(pVertex1->normal.z, pVertex2->normal.z)

		&& float_eq(pVertex1->tex.u, pVertex2->tex.u)
		&& float_eq(pVertex1->tex.v, pVertex2->tex.v))
		return true ;

    return false ;
}

//현재 WeldVertex()함수는 순전히 파싱을 위한 것이기 때문에 앞으로 파싱을 하는 툴에서 사용되어 져야 한다.
void CASEData::WeldVertex(HWND hwnd)
{
	int i, n, t, nSkinCount, nBoneIndex ;
	SMesh *pMesh, *psParentMesh ;
	SMeshTriangle *psTriangle ;
	int nTriangleCount, nVertexCount ;
	SMeshTexCoord texTiling ;

	bool bFirstStrike=true, bMeshFirstStrike ;

	//SMeshVertex *pVertex ;
	//SMeshVertex *pHead = new SMeshVertex ;
	//SMeshVertex *pTail = new SMeshVertex ;

	//CDNode cdNode(MAXNUM_ASEDATAMESH) ;
	//cdNode.Initialize(pHead, pTail, IdentityVertex) ;

	Matrix4 matChild ;

	TRACE("WELDING VERTEICES\r\n") ;

	for(i=0 ; i<m_nNumMesh ; i++)
	{
		pMesh = m_apMesh[i] ;
		bMeshFirstStrike = false ;

		SMeshVertex *pVertex ;
		SMeshVertex *pHead = new SMeshVertex ;
		SMeshVertex *pTail = new SMeshVertex ;

		CDNode cdNode ;
		cdNode.Initialize(pHead, pTail, IdentityVertex) ;

		Vector3 v ;

		if(pMesh->bParent)
		{
			Matrix4 matLocal((Matrix4 &)pMesh->smatLocal) ;
			pMesh->smatLocal.set((SMeshMatrix &)matLocal) ;//현제 로칼매트릭스

			psParentMesh = pMesh->psMeshParent ;
			Matrix4 matParentLocal((Matrix4 &)psParentMesh->smatLocal) ;//부모 로칼매트릭스
			matChild = matLocal*matParentLocal.Inverse() ;//자식매트릭스 = 현재로칼매트릭스 * 부모로칼인버스매트릭스, 의미를 따지자면 부모의 상대로칼매트릭스
			pMesh->smatChild.set((SMeshMatrix &)matChild) ;
			matChild = matLocal.Inverse() ;//지금 버텍스에다가 자식매트릭스의 역행렬을 곱해두므로써 랜더링시에는 월드매트릭스 = 자식매트릭스*부모매트릭스
			//현재 버텍스에다가는 로칼의역행렬을 곱해두고
			//matChild 에다가 현재의 로칼행렬x부모의역행렬
			//이렇게 해두면 나중에 로칼행렬x부모로칼행렬 이렇게 할것이기 때문에 딱맞아떨어진다.
		}
		else
		{
			Matrix4 matLocal((Matrix4 &)pMesh->smatLocal) ;
			pMesh->smatLocal.set((SMeshMatrix &)matLocal) ;

			pMesh->smatChild.set(pMesh->smatLocal) ;//부모가 없을경우는 로칼매트릭스를 저장한다
			matChild = matLocal.Inverse() ;
		}

		//Weld Vertex -> ASE파일의 리스트와 인덱스를 취합하여 중복된 버텍스를 제거하고 D3D의 DrawPrimitiveIndex함수에 맞게끔 버텍스와 인덱스를 제구성한다.
		//이미 ASE파싱할때 삼각형단위로 데이타를 저장했기때문에 메쉬의 모든 삼각형을 루프를 돌면서 중복된것이 아닌 새로운 버텍스를 뽑아낸다.
		//새로운 버텍스를 뽑아내는 기준은,
		//1. 버텍스가 다른경우
		//2. 텍스쳐좌표가 다른경우
		//3. 노말벡터가 다른경우
		//위의 3가지중 한가지라도 다르면 새로운 버텍스를 추가 시킨다. IdentityVertex함수가 1,2,3 번을 비교검색한다.

		nVertexCount=0 ;
		nTriangleCount=0 ;
		while(nTriangleCount < pMesh->nNumTriangle)
		{
			psTriangle = &pMesh->psTriangle[nTriangleCount++] ;

			static int nTriangles=0 ;

			nTriangles = nTriangleCount ;
			if(!(nTriangles % 1000))
				TRACE("Triangles=%d/%d\r\n", nTriangles, pMesh->nNumTriangle) ;

			for(n=0 ; n<3 ; n++)
			{
				if(cdNode.GetCurNumNode() == 0)//First-Strike
				{
					pVertex = new SMeshVertex ;

					//Vertex
					v.set(pMesh->psVertex[psTriangle->anVertexIndex[n]].pos.x,
						pMesh->psVertex[psTriangle->anVertexIndex[n]].pos.y,
						pMesh->psVertex[psTriangle->anVertexIndex[n]].pos.z) ;

					if(bFirstStrike)//전체오브젝트에서 사용
					{
						m_vMin.set(v.x, v.y, v.z) ;
						m_vMax.set(v.x, v.y, v.z) ;
						bFirstStrike = false ;
					}
					if(bMeshFirstStrike)//메쉬에서만 사용
					{
						pMesh->vMin.set(v.x, v.y, v.z) ;
						pMesh->vMax.set(v.x, v.y, v.z) ;
						bMeshFirstStrike = false ;
					}

					v *= matChild ;//지금 버텍스에다가 자식매트릭스의 역행렬을 곱해두므로써 랜더링시에는 월드매트릭스 = 자식매트릭스*부모매트릭스

					pVertex->pos.set(v.x, v.y, v.z) ;

					//Normal
					pVertex->normal = psTriangle->asVertexNormal[n] ;
					
					//Texture Coordinate
					if(m_psMaterial[pMesh->nMaterialIndex].bUseTexDecal)
						texTiling = m_psMaterial[pMesh->nMaterialIndex].sTexTiling ;
					else if(m_psMaterial[pMesh->nMaterialIndex].nNumSubMaterial > 0)//use submaterials
					{
						if(m_psMaterial[pMesh->nMaterialIndex].psSubMaterial[psTriangle->nMaterialID].bUseTexDecal)
							texTiling = m_psMaterial[pMesh->nMaterialIndex].psSubMaterial[psTriangle->nMaterialID].sTexTiling ;
						else
							texTiling.set(1, 1) ;
					}
					else
						texTiling.set(1, 1) ;

					pVertex->tex.u = psTriangle->asTexCoord[n].u * texTiling.u ;
					pVertex->tex.v = psTriangle->asTexCoord[n].v * texTiling.v ;
					//pVertex->tex = psTriangle->asTexCoord[n] ;

					///////////////////////////////////////////////////////////////
                    //Skin
					for(nSkinCount=0 ; nSkinCount<pMesh->psVertex[psTriangle->anVertexIndex[n]].nNumSkin ; nSkinCount++)
					{
						pVertex->apsSkin[nSkinCount] = new SMeshSkin ;
						memcpy(pVertex->apsSkin[nSkinCount], pMesh->psVertex[psTriangle->anVertexIndex[n]].apsSkin[nSkinCount], sizeof(SMeshSkin)) ;

						if(FindBoneIndex(pVertex->apsSkin[nSkinCount]->szBoneName, nBoneIndex))
							pVertex->apsSkin[nSkinCount]->nBoneIndex = nBoneIndex ;
						else
							assert(!"Cannot Find BoneIndex") ;
					}
					pVertex->nNumSkin = pMesh->psVertex[psTriangle->anVertexIndex[n]].nNumSkin ;
					///////////////////////////////////////////////////////////////

					cdNode.InsertNode((void *)pVertex) ;

					psTriangle->anVertexIndex[n] = 0 ;

					//TRACE("[%02d] pos(%+08.03f, %+08.03f, %+08.03f) normal(%+08.03f, %+08.03f, %+08.03f) tex(%+08.03f, %+08.03f, %+08.03f)\r\n",
					//	++nVertexCount,
					//	pVertex->pos.x, pVertex->pos.y, pVertex->pos.z,
					//	pVertex->normal.x, pVertex->normal.y, pVertex->normal.z,
					//	pVertex->tex.u, pVertex->tex.v) ;
				}
				else //After First-Strike
				{
					DNode *pCurrentNode ;
					pVertex = new SMeshVertex ;

					//Vertex
					v.set(pMesh->psVertex[psTriangle->anVertexIndex[n]].pos.x,
						pMesh->psVertex[psTriangle->anVertexIndex[n]].pos.y,
						pMesh->psVertex[psTriangle->anVertexIndex[n]].pos.z) ;

					if(m_vMin.x > v.x)	m_vMin.x = v.x ;
					if(m_vMin.y > v.y)	m_vMin.y = v.y ;
					if(m_vMin.z > v.z)	m_vMin.z = v.z ;

					if(m_vMax.x < v.x)	m_vMax.x = v.x ;
					if(m_vMax.y < v.y)	m_vMax.y = v.y ;
					if(m_vMax.z < v.z)	m_vMax.z = v.z ;

					if(pMesh->vMin.x > v.x)	pMesh->vMin.x = v.x ;
					if(pMesh->vMin.y > v.y)	pMesh->vMin.y = v.y ;
					if(pMesh->vMin.z > v.z)	pMesh->vMin.z = v.z ;

					if(pMesh->vMax.x < v.x)	pMesh->vMax.x = v.x ;
					if(pMesh->vMax.y < v.y)	pMesh->vMax.y = v.y ;
					if(pMesh->vMax.z < v.z)	pMesh->vMax.z = v.z ;

					v *= matChild ;//지금 버텍스에다가 자식매트릭스의 역행렬을 곱해두므로써 랜더링시에는 월드매트릭스 = 자식매트릭스*부모매트릭스

					pVertex->pos.set(v.x, v.y, v.z) ;

					//Noraml
					pVertex->normal = psTriangle->asVertexNormal[n] ;

					//Texture Coordinate
					if(m_psMaterial[pMesh->nMaterialIndex].bUseTexDecal)
						texTiling = m_psMaterial[pMesh->nMaterialIndex].sTexTiling ;
					else if(m_psMaterial[pMesh->nMaterialIndex].nNumSubMaterial > 0)//use submaterials
					{
						if(m_psMaterial[pMesh->nMaterialIndex].psSubMaterial[psTriangle->nMaterialID].bUseTexDecal)
							texTiling = m_psMaterial[pMesh->nMaterialIndex].psSubMaterial[psTriangle->nMaterialID].sTexTiling ;
						else
							texTiling.set(1, 1) ;
					}
					else
						texTiling.set(1, 1) ;

					pVertex->tex.u = psTriangle->asTexCoord[n].u * texTiling.u ;
					pVertex->tex.v = psTriangle->asTexCoord[n].v * texTiling.v ;
					//pVertex->tex = psTriangle->asTexCoord[n] ;

					////////////////////////////////////////////////////////
					//Skin
					for(nSkinCount=0 ; nSkinCount<pMesh->psVertex[psTriangle->anVertexIndex[n]].nNumSkin ; nSkinCount++)
					{
						pVertex->apsSkin[nSkinCount] = new SMeshSkin ;
						memcpy(pVertex->apsSkin[nSkinCount], pMesh->psVertex[psTriangle->anVertexIndex[n]].apsSkin[nSkinCount], sizeof(SMeshSkin)) ;

						if(FindBoneIndex(pVertex->apsSkin[nSkinCount]->szBoneName, nBoneIndex))
							pVertex->apsSkin[nSkinCount]->nBoneIndex = nBoneIndex ;
						else
							assert(!"Cannot Find BoneIndex") ;
					}
					pVertex->nNumSkin = pMesh->psVertex[psTriangle->anVertexIndex[n]].nNumSkin ;
					/////////////////////////////////////////////////////////

					pCurrentNode = cdNode.FindNode((void *)pVertex) ;

					if(pCurrentNode == NULL)//새로운 버텍스인경우
					{
						cdNode.InsertNode((void *)pVertex) ;
						psTriangle->anVertexIndex[n] = cdNode.GetCurNumNode()-1 ;

						//TRACE("[%02d] pos(%+08.03f, %+08.03f, %+08.03f) normal(%+08.03f, %+08.03f, %+08.03f) tex(%+08.03f, %+08.03f, %+08.03f)\r\n",
						//	++nVertexCount,
						//	pVertex->pos.x, pVertex->pos.y, pVertex->pos.z,
						//	pVertex->normal.x, pVertex->normal.y, pVertex->normal.z,
						//	pVertex->tex.u, pVertex->tex.v) ;
					}
					else//이미 저장된 버텍스인경우
					{
						psTriangle->anVertexIndex[n] = pCurrentNode->nCurrentIndex ;//저장된 버텍스인덱스를 링크시킨다.
						delete pVertex ;
					}
				}
			}
		}

		if(pMesh->psVertex)
			delete []pMesh->psVertex ;

		nVertexCount=0 ;
		pMesh->nNumVertex = cdNode.GetCurNumNode() ;
		pMesh->psVertex = new SMeshVertex[pMesh->nNumVertex] ;

		//TRACE("COPY VERTEX DATA Node to Mesh\r\n") ;

		DNode *pNodeTail = cdNode.GetTail() ;
		DNode *pNode = cdNode.GetHead()->pNext ;
		while(pNode != pNodeTail)//버텍스단위로 저장된 노드를 Mesh구조체로 저장한다.
		{
			pVertex = (SMeshVertex *)pNode->pvData ;
			memcpy(&pMesh->psVertex[nVertexCount], pVertex, sizeof(SMeshVertex)) ;

			//TRACE("[%03d] Vertex(%+08.03f, %+08.03f, %+08.03f) normal(%+08.03f, %+08.03f, %+08.03f) tex(%+08.03f, %+08.03f)\r\n",
			//	nVertexCount,
			//	pMesh->psVertex[nVertexCount].pos.x,
			//	pMesh->psVertex[nVertexCount].pos.y,
			//	pMesh->psVertex[nVertexCount].pos.z,
			//	pMesh->psVertex[nVertexCount].normal.x,
			//	pMesh->psVertex[nVertexCount].normal.y,
			//	pMesh->psVertex[nVertexCount].normal.z,
			//	pMesh->psVertex[nVertexCount].tex.u,
			//	pMesh->psVertex[nVertexCount].tex.v) ;

			for(t=0 ; t<pVertex->nNumSkin ; t++)
			{
				pMesh->psVertex[nVertexCount].apsSkin[t] = new SMeshSkin ;
				memcpy(pMesh->psVertex[nVertexCount].apsSkin[t], pVertex->apsSkin[t], sizeof(SMeshSkin)) ;
			}
			nVertexCount++ ;

            delete pVertex ;
			pNode = pNode->pNext ;
		}

		delete cdNode.GetHead()->pvData ;
		delete cdNode.GetTail()->pvData ;

		cdNode.DeleteAll() ;
	}
}

bool CASEData::FindMesh(char *pszNodeName, SMesh **ppMesh, int &nIndex)
{
	for(int i=0 ; i<m_nNumMesh ; i++)
	{
		if(strcmp(m_apMesh[i]->szMeshName, pszNodeName) == 0)
		{
			*ppMesh = m_apMesh[i] ;
			nIndex = i ;
			return true ;
		}
			
	}
    return false ;
}

bool CASEData::FindBoneIndex(char *pszNodeName, int &nIndex)
{
	for(int i=0 ; i<m_nNumBone ; i++)
	{
		if(strcmp(m_apszBoneName[i], pszNodeName) == 0)
		{
			nIndex = i ;
			return true ;
		}
	}
	return false ;
}

void CASEData::_WriteString(char *psz, FILE *pf)
{
	UINT lLength ;
	lLength = (UINT)CloseToExp((float)(strlen(psz)+1), 2) ;
    fwrite(&lLength, sizeof(UINT), 1, pf) ;

	fwrite(psz, lLength, 1, pf) ;
}

void CASEData::_ReadString(char *psz, FILE *pf)
{
	fread(&ltemp, sizeof(UINT), 1, pf) ;
	fread(psz, ltemp, 1, pf) ;
}

bool CASEData::_WriteMaterial(SMaterial *psMaterial, FILE *pf)
{
	ltemp = (UINT)psMaterial->nAttr ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	ltemp = (UINT)psMaterial->bUseTexDecal ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	ltemp = (UINT)psMaterial->bUseOpacity ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	//ltemp = (UINT)CloseToExp((float)strlen(psMaterial->szDecaleName), 2) ;
	//fwrite(&ltemp, sizeof(UINT), 1, pf) ;
	//fwrite(psMaterial->szDecaleName, ltemp, 1, pf) ;
	_WriteString(psMaterial->szDecaleName, pf) ;

	fwrite(&psMaterial->nNumSubMaterial, sizeof(UINT), 1, pf) ;
	if(psMaterial->nNumSubMaterial)
	{
		for(int i=0 ; i<psMaterial->nNumSubMaterial ; i++)
			_WriteMaterial(&psMaterial->psSubMaterial[i], pf) ;
	}

	fwrite(&psMaterial->sAmbient, sizeof(SColor), 1, pf) ;
	fwrite(&psMaterial->sDiffuse, sizeof(SColor), 1, pf) ;
	fwrite(&psMaterial->sSpecular, sizeof(SColor), 1, pf) ;

	return true ;
}

bool CASEData::_ReadMaterial(SMaterial *psMaterial, FILE *pf)
{
	fread(&ltemp, sizeof(UINT), 1, pf) ;
	psMaterial->nAttr = ltemp ;

	fread(&ltemp, sizeof(UINT), 1, pf) ;
	psMaterial->bUseTexDecal = (ltemp) ? true : false ;

	fread(&ltemp, sizeof(UINT), 1, pf) ;
	psMaterial->bUseOpacity = (ltemp) ? true : false ;

	_ReadString(psMaterial->szDecaleName, pf) ;

	fread(&psMaterial->nNumSubMaterial, sizeof(UINT), 1, pf) ;
	if(psMaterial->nNumSubMaterial)
	{
		psMaterial->psSubMaterial = new SMaterial[psMaterial->nNumSubMaterial] ;
		for(int i=0 ; i<psMaterial->nNumSubMaterial ; i++)
			_ReadMaterial(&psMaterial->psSubMaterial[i], pf) ;
	}

    fread(&psMaterial->sAmbient, sizeof(SColor), 1, pf) ;
	fread(&psMaterial->sDiffuse, sizeof(SColor), 1, pf) ;
	fread(&psMaterial->sSpecular, sizeof(SColor), 1, pf) ;

	return true ;
}

void CASEData::_WriteMeshSkin(SMeshSkin *psSkin, FILE *pf)
{
	ltemp = (UINT)psSkin->bEnable ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	fwrite(&psSkin->nBoneIndex, sizeof(UINT), 1, pf) ;
	fwrite(&psSkin->fWeight, sizeof(UINT), 1, pf) ;
	_WriteString(psSkin->szBoneName, pf) ;
}

void CASEData::_ReadMeshSkin(SMeshSkin *psSkin, FILE *pf)
{
	fread(&ltemp, sizeof(UINT), 1, pf) ;
	psSkin->bEnable = (ltemp) ? true : false ;

	fread(&psSkin->nBoneIndex, sizeof(UINT), 1, pf) ;
	fread(&psSkin->fWeight, sizeof(UINT), 1, pf) ;
	_ReadString(psSkin->szBoneName, pf) ;
}

void CASEData::_WriteMeshVertex(SMeshVertex *psVertex, FILE *pf)
{
	ltemp = (UINT)psVertex->bEnable ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	fwrite(&psVertex->pos, sizeof(SMeshVector), 1, pf) ;
	fwrite(&psVertex->normal, sizeof(SMeshVector), 1, pf) ;
	fwrite(&psVertex->tex, sizeof(SMeshTexCoord), 1, pf) ;
	fwrite(&psVertex->fAmbientDegree, sizeof(float), 1, pf) ;

	fwrite(&psVertex->nNumSkin, sizeof(UINT), 1, pf) ;
	if(psVertex->nNumSkin)
	{
		for(int i=0 ; i<psVertex->nNumSkin ; i++)
			_WriteMeshSkin(psVertex->apsSkin[i], pf) ;
	}
}

void CASEData::_ReadMeshVertex(SMeshVertex *psVertex, FILE *pf)
{
    fread(&ltemp, sizeof(UINT), 1, pf) ;
	psVertex->bEnable = (ltemp) ? true : false ;

	int ii = sizeof(SMeshVector) ;

	fread(&psVertex->pos, sizeof(SMeshVector), 1, pf) ;
	fread(&psVertex->normal, sizeof(SMeshVector), 1, pf) ;
	fread(&psVertex->tex, sizeof(SMeshTexCoord), 1, pf) ;
	fread(&psVertex->fAmbientDegree, sizeof(float), 1, pf) ;

	//TRACE("pos(%+02.03f, %+02.03f, %+02.03f)\r\n", psVertex->pos.x, psVertex->pos.y, psVertex->pos.z) ;

	fread(&psVertex->nNumSkin, sizeof(UINT), 1, pf) ;
	if(psVertex->nNumSkin)
	{
		for(int i=0 ; i<psVertex->nNumSkin ; i++)
		{
			psVertex->apsSkin[i] = new SMeshSkin ;
			_ReadMeshSkin(psVertex->apsSkin[i], pf) ;
		}
	}
}

bool CASEData::_WriteMesh(SMesh *pMesh, FILE *pf)
{
	fwrite(&pMesh->nAttr, sizeof(UINT), 1, pf) ;

	ltemp = (UINT)pMesh->MeshKind ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	fwrite(&pMesh->lVertexType, sizeof(UINT), 1, pf) ;

	_WriteString(pMesh->szMeshName, pf) ;

	ltemp = (UINT)pMesh->bParent ;
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	if(pMesh->bParent)
		_WriteString(pMesh->szParentName, pf) ;

	int i ;

	fwrite(&pMesh->nNumVertex, sizeof(UINT), 1, pf) ;
	if(pMesh->nNumVertex > 0)
	{
		for(i=0 ; i<pMesh->nNumVertex ; i++)
			_WriteMeshVertex(&pMesh->psVertex[i], pf) ;
	}

	fwrite(&pMesh->nNumTriangle, sizeof(UINT), 1, pf) ;
	if(pMesh->nNumTriangle > 0)
		fwrite(pMesh->psTriangle, sizeof(SMeshTriangle), pMesh->nNumTriangle, pf) ;

	fwrite(&pMesh->nNumTexCoord, sizeof(UINT), 1, pf) ;
	if(pMesh->nNumTexCoord > 0)
		fwrite(pMesh->psTexCoord, sizeof(SMeshTexCoord), pMesh->nNumTexCoord, pf) ;

	fwrite(&pMesh->smatLocal, sizeof(SMeshMatrix), 1, pf) ;
	fwrite(&pMesh->smatChild, sizeof(SMeshMatrix), 1, pf) ;
	fwrite(&pMesh->svPos, sizeof(SMeshVector), 1, pf) ;
	fwrite(&pMesh->svRotAxis, sizeof(SMeshVector), 1, pf) ;
	fwrite(&pMesh->fRotAngle, sizeof(float), 1, pf) ;
	fwrite(&pMesh->svScale, sizeof(SMeshVector), 1, pf) ;

	fwrite(&pMesh->nNumMeshKeyPos, sizeof(int), 1, pf) ;
	if(pMesh->nNumMeshKeyPos > 0)
	{
		for(i=0 ; i<pMesh->nNumMeshKeyPos ; i++)
			fwrite(pMesh->apMeshKeyPos[i], sizeof(SMeshKeyPos), 1, pf) ;
	}

	fwrite(&pMesh->nNumMeshKeyRot, sizeof(int), 1, pf) ;
	if(pMesh->apMeshKeyRot > 0)
	{
		for(i=0 ; i<pMesh->nNumMeshKeyRot ; i++)
			fwrite(pMesh->apMeshKeyRot[i], sizeof(SMeshKeyRot), 1, pf) ;
	}

	fwrite(&pMesh->nNumMeshKeyScale, sizeof(int), 1, pf) ;
	if(pMesh->apMeshKeyScale > 0)
	{
		for(i=0 ; i<pMesh->nNumMeshKeyScale ; i++)
			fwrite(pMesh->apMeshKeyScale[i], sizeof(SMeshKeyScale), 1, pf) ;
	}

	if(pMesh->bParent)
		fwrite(&pMesh->nMeshParentIndex, sizeof(int), 1, pf) ;

	fwrite(&pMesh->nMaterialIndex, sizeof(int), 1, pf) ;

	fwrite(&pMesh->vMin, sizeof(SMeshVector), 1, pf) ;
	fwrite(&pMesh->vMax, sizeof(SMeshVector), 1, pf) ;

	return true ;
}

bool CASEData::_ReadMesh(SMesh *pMesh, FILE *pf)
{
	int i ;

	fread(&pMesh->nAttr, sizeof(UINT), 1, pf) ;

	fread(&ltemp, sizeof(UINT), 1, pf) ;
	pMesh->MeshKind = (SMESHKIND)ltemp ;

	fread(&pMesh->lVertexType, sizeof(UINT), 1, pf) ;

	_ReadString(pMesh->szMeshName, pf) ;

	fread(&ltemp, sizeof(UINT), 1, pf) ;
	pMesh->bParent = (ltemp) ? true : false ;

	if(pMesh->bParent)
		_ReadString(pMesh->szParentName, pf) ;

	fread(&pMesh->nNumVertex, sizeof(UINT), 1, pf) ;
	//assert(pMesh->nNumVertex > 0) ;
	if(pMesh->nNumVertex > 0)
	{
		pMesh->psVertex = new SMeshVertex[pMesh->nNumVertex] ;

		for(i=0 ; i<pMesh->nNumVertex ; i++)
			_ReadMeshVertex(&pMesh->psVertex[i], pf) ;
	}

	fread(&pMesh->nNumTriangle, sizeof(int), 1, pf) ;
	if(pMesh->nNumTriangle > 0)
	{
		pMesh->psTriangle = new SMeshTriangle[pMesh->nNumTriangle] ;
		fread(pMesh->psTriangle, sizeof(SMeshTriangle), pMesh->nNumTriangle, pf) ;
	}

	fread(&pMesh->nNumTexCoord, sizeof(int), 1, pf) ;
	if(pMesh->nNumTexCoord > 0)
	{
		pMesh->psTexCoord = new SMeshTexCoord[pMesh->nNumTexCoord] ;
		fread(pMesh->psTexCoord, sizeof(SMeshTexCoord), pMesh->nNumTexCoord, pf) ;
	}

    fread(&pMesh->smatLocal, sizeof(SMeshMatrix), 1, pf) ;
	fread(&pMesh->smatChild, sizeof(SMeshMatrix), 1, pf) ;
	fread(&pMesh->svPos, sizeof(SMeshVector), 1, pf) ;
	fread(&pMesh->svRotAxis, sizeof(SMeshVector), 1, pf) ;
	fread(&pMesh->fRotAngle, sizeof(float), 1, pf) ;
	fread(&pMesh->svScale, sizeof(SMeshVector), 1, pf) ;

	fread(&pMesh->nNumMeshKeyPos, sizeof(int), 1, pf) ;
	if(pMesh->nNumMeshKeyPos > 0)
	{
		for(i=0 ; i<pMesh->nNumMeshKeyPos ; i++)
		{
			pMesh->apMeshKeyPos[i] = new SMeshKeyPos ;
			fread(pMesh->apMeshKeyPos[i], sizeof(SMeshKeyPos), 1, pf) ;
		}
	}

	fread(&pMesh->nNumMeshKeyRot, sizeof(int), 1, pf) ;
	if(pMesh->nNumMeshKeyRot > 0)
	{
		for(i=0 ; i<pMesh->nNumMeshKeyRot ; i++)
		{
			pMesh->apMeshKeyRot[i] = new SMeshKeyRot ;
			fread(pMesh->apMeshKeyRot[i], sizeof(SMeshKeyRot), 1, pf) ;
		}
	}

	fread(&pMesh->nNumMeshKeyScale, sizeof(int), 1, pf) ;
	if(pMesh->nNumMeshKeyScale > 0)
	{
		for(i=0 ; i<pMesh->nNumMeshKeyScale ; i++)
		{
			pMesh->apMeshKeyScale[i] = new SMeshKeyScale ;
			fread(pMesh->apMeshKeyScale[i], sizeof(SMeshKeyScale), 1, pf) ;
		}
	}

	if(pMesh->bParent)
	{
		fread(&pMesh->nMeshParentIndex, sizeof(int), 1, pf) ;
		FindMesh(pMesh->szParentName, &pMesh->psMeshParent, pMesh->nMeshParentIndex) ;
	}

	fread(&pMesh->nMaterialIndex, sizeof(int), 1, pf) ;

	fread(&pMesh->vMin, sizeof(SMeshVector), 1, pf) ;
	fread(&pMesh->vMax, sizeof(SMeshVector), 1, pf) ;

	return true ;
}

bool CASEData::ExportRMD(char *pszFileName)//Export to Renderable Model Data
{
	FILE *pf=NULL ;
	int i ;

    pf = fopen(pszFileName, "wb+") ;
	if(pf == NULL)
		return false ;

    ltemp = TYPERMD ;//File Type
	fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	ltemp = RMDVER ;//RMD File Version
    fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	fwrite(m_szFileName, 256, 1, pf) ;

	fwrite(&m_sSceneInfo, sizeof(SSceneInfo), 1, pf) ;

	fwrite(&m_lAttr, sizeof(UINT), 1, pf) ;
	fwrite(&m_lSubAttr, sizeof(UINT), 1, pf) ;

    fwrite(&m_vMin, sizeof(SMeshVector), 1, pf) ;
	fwrite(&m_vMax, sizeof(SMeshVector), 1, pf) ;

	ltemp = m_bUseDummyObject ;
    fwrite(&ltemp, sizeof(UINT), 1, pf) ;

	fwrite(&m_nNumMaterial, sizeof(int), 1, pf) ;
    for(i=0 ; i<m_nNumMaterial ; i++)
		_WriteMaterial(&m_psMaterial[i], pf) ;

	fwrite(&m_nNumMesh, sizeof(int), 1, pf) ;

	for(i=0 ; i<m_nNumMesh ; i++)
		_WriteMesh(m_apMesh[i], pf) ;

	fwrite(&m_nNumBone, sizeof(int), 1, pf) ;
	for(i=0 ; i<m_nNumBone ; i++)
		_WriteString(m_apszBoneName[i], pf) ;

	fclose(pf) ;
	pf=NULL ;

	return true ;
}

bool CASEData::ImportRMD(char *pszFileName)
{
	FILE *pf=NULL ;
	int i ;

	pf = fopen(pszFileName, "rb+") ;
	if(pf == NULL)
		return false ;

	fread(&ltemp, sizeof(UINT), 1, pf) ;
	if(ltemp != TYPERMD)
		return false ;

    fread(&ltemp, sizeof(UINT), 1, pf) ;
	if(ltemp != RMDVER)
		return false ;

	fread(m_szFileName, 256, 1, pf) ;

    fread(&m_sSceneInfo, sizeof(SSceneInfo), 1, pf) ;

	fread(&m_lAttr, sizeof(UINT), 1, pf) ;
	fread(&m_lSubAttr, sizeof(UINT), 1, pf) ;

    fread(&m_vMin, sizeof(SMeshVector), 1, pf) ;
	fread(&m_vMax, sizeof(SMeshVector), 1, pf) ;	

	fread(&ltemp, sizeof(UINT), 1, pf) ;
	m_bUseDummyObject = (ltemp) ? true : false ;

	fread(&m_nNumMaterial, sizeof(UINT), 1, pf) ;
	if(m_nNumMaterial > 0)
	{
	assert(m_nNumMaterial > 0) ;

	m_psMaterial = new SMaterial[m_nNumMaterial] ;

	for(i=0 ; i<m_nNumMaterial ; i++)
		_ReadMaterial(&m_psMaterial[i], pf) ;
	}

	fread(&m_nNumMesh, sizeof(int), 1, pf) ;
	assert(m_nNumMesh > 0) ;

	for(i=0 ; i<m_nNumMesh ; i++)
	{
		m_apMesh[i] = new SMesh ;
		_ReadMesh(m_apMesh[i], pf) ;
	}

	fread(&m_nNumBone, sizeof(int), 1, pf) ;
	for(i=0 ; i<m_nNumBone ; i++)
	{
        m_apszBoneName[i] = new char[256] ;
		_ReadString(m_apszBoneName[i], pf) ;
	}

	fclose(pf) ;
    return true ;
}

#include "LoadTGA.h"

//void CASEData::ComputeTangent(SMeshTriangle *psTriangle)
void ComputeTangent(SMeshVertex &vt0, SMeshVertex &vt1, SMeshVertex &vt2)
{
	if( (Vector3(vt0.tangent.x, vt0.tangent.y, vt0.tangent.z)).Magnitude() > 0.0f )
		return ;

	//v1.pos-v0.pos = (v1.tex.u-v0.tex.u)*T + (v1.tex.v-v0.tex.v)*B ;
	//v2.pos-v0.pos = (v2.tex.u-v0.tex.u)*T + (v2.tex.v-v0.tex.v)*B ;

	//Refer to http://www.blacksmith-studios.dk/projects/downloads/tangent_matrix_derivation.php
	//Refer to DirectX9 셰이더프로그래밍 Chapter5 범프맵핑 p322

//	 ---------u
//	 |
//	 |
//	 |
//	 v

	//|p1-p0|   |(u1-u0) (v1-v0)| |Tangent |
	//|     | = |               |*|        |
	//|p2-p0|   |(u2-u0) (v2-v0)| |Binormal|
	//
	//    |(u1-u0) (v1-v0)|
	//det=|               |=( (u1-u0)*(v2-v0)-(v1-v0)*(u2-u0) )
	//    |(u2-u0) (v2-v0)|
	//
	//|Tangent |          | (v2-v0) -(v1-v0)| |p1-p0|
	//|        |= (1/det)*|                 |*|     |
	//|Binormal|          |-(u2-u0)  (u1-u0)| |p2-p0|
	//
	//
	//Tangent  = (1/det)*( (v2-v0)*(p1-p0)-(v1-v0)*(p2-p0) )
	//Binormal = (1/det)*(-(u2-u0)*(p1-p0)+(u1-u0)*(p2-p0) )
	//
	//각 성분별로 계산을 실시한다.
	//그러나 (1/det)는 나중에 Normalize 계산시에 모두 날라가 버리기 때문에 무용지물이다

	float   v0, v1, v2 ;
	float	u0, u1, u2 ;
	Vector3 p0, p1, p2 ;

	p0.set(vt0.pos.x, vt0.pos.y, vt0.pos.z) ;
	p1.set(vt1.pos.x, vt1.pos.y, vt1.pos.z) ;
	p2.set(vt2.pos.x, vt2.pos.y, vt2.pos.z) ;

	u0 = vt0.tex.u ;
	u1 = vt1.tex.u ;
	u2 = vt2.tex.u ;

	v0 = vt0.tex.v ;
	v1 = vt1.tex.v ;
	v2 = vt2.tex.v ;

    float det = (u1-u0)*(v2-v0) - (v1-v0)*(u2-u0) ;

	Vector3 vTangent, vBinormal, vNormal ;

	//vTangent  = ( (p1-p0)*(v2-v0) - (p2-p0)*(v1-v0) )*(1/det) ;
	//vTangent = vTangent.Normalize() ;
	vBinormal = (-(p1-p0)*(u2-u0) + (p2-p0)*(u1-u0) )*(1/det) ;
	vBinormal = vBinormal.Normalize() ;

	vNormal.set(vt0.normal.x, vt0.normal.y, vt0.normal.z) ;

	vTangent = vBinormal.cross(vNormal) ;
	vBinormal = vTangent.cross(vNormal) ;

	vt0.tangent.set(vTangent.x, vTangent.y, vTangent.z) ;

	static int count=0 ;
	//TRACE("### Base Vector about [%2d] pos(%3.3f %3.3f %3.3f) ###\r\n", count++, enumVector(p0)) ;
	//TRACE("vTangent (%3.3f %3.3f %3.3f)\r\n", enumVector(vTangent)) ;
	//TRACE("vBinormal(%3.3f %3.3f %3.3f)\r\n", enumVector(vBinormal)) ;
	//TRACE("\r\n") ;
}

void CASEData::MakeTangent()
{
	//CTga tga ;
	//tga.LoadTGAFile(pszFileName) ;
	//if(!tga.m_bEnable)
	//{
	//	char err[256] ;
	//	sprintf(err, "can't found %s", pszFileName) ;
	//	assert(false && err) ;		
	//}

	int i, n ;
	SMeshTriangle *psTriangle ;
	for(i=0 ; i<m_nNumMesh ; i++)
	{
		for(n=0 ; n<m_apMesh[i]->nNumTriangle ; n++)
		{
			psTriangle = &m_apMesh[i]->psTriangle[n] ;

			ComputeTangent(m_apMesh[i]->psVertex[psTriangle->anVertexIndex[0]],
				m_apMesh[i]->psVertex[psTriangle->anVertexIndex[1]],
				m_apMesh[i]->psVertex[psTriangle->anVertexIndex[2]]) ;

			ComputeTangent(m_apMesh[i]->psVertex[psTriangle->anVertexIndex[1]],
				m_apMesh[i]->psVertex[psTriangle->anVertexIndex[0]],
				m_apMesh[i]->psVertex[psTriangle->anVertexIndex[2]]) ;

			ComputeTangent(m_apMesh[i]->psVertex[psTriangle->anVertexIndex[2]],
				m_apMesh[i]->psVertex[psTriangle->anVertexIndex[0]],
				m_apMesh[i]->psVertex[psTriangle->anVertexIndex[1]]) ;
		}
	}
}

//Ambient Occlusion

float CASEData::Corresponded(SMeshVertex *psVertex, Vector3 *pv)
{
	for(int i=0 ; i<3 ; i++)
	{
		//if(VECTOR_EQ(psVertex->pos, pv[i]))
		if( (float_eq(psVertex->pos.x, pv[i].x)) && (float_eq(psVertex->pos.y, pv[i].y)) && (float_eq(psVertex->pos.z, pv[i].z)) )
		{
			Vector3 pos(psVertex->pos.x, psVertex->pos.y, psVertex->pos.z) ;
            Vector3 center = GetMidPoint(pv[0], pv[1], pv[2]) ;
			return (pos-center).Magnitude() ;
		}
	}
	return 0 ;
}

float CASEData::ComputeDiskAreaPerVertex(SMeshVertex *psVertex, SMesh *psMesh)
{
	int n, i, nCorresponded=0 ;
	float fRadius, fSumRadius=0 ;
	Vector3 avPos[3] ;

	for(i=0 ; i<psMesh->nNumTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++)
		{
			avPos[n].set(psMesh->psVertex[psMesh->psTriangle[i].anVertexIndex[n]].pos.x,
						 psMesh->psVertex[psMesh->psTriangle[i].anVertexIndex[n]].pos.y,
						 psMesh->psVertex[psMesh->psTriangle[i].anVertexIndex[n]].pos.z);
		}

		fRadius = Corresponded(psVertex, avPos) ;//현재의 버텍스가 삼각형의 꼭지점에 포함되어 있다면
		if(fRadius > (0+g_fEpsilon))
		{
			nCorresponded++ ;
			fSumRadius += fRadius ;
		}
	}
    assert(nCorresponded) ;//부합되는 버텍스가 없으면 안된다.
	return fSumRadius/(float)nCorresponded ;//평균 반지름을 구한다
}

void CASEData::SubSetVertexDisk(std::vector<SVertexDisk> *pVertexDisks, SMesh *pMesh)
{
	static int nVertexCount=0 ;
	SVertexDisk sVertexDisk ;

	for(int i=0 ; i<pMesh->nNumVertex ; i++)
	{
		sVertexDisk.vPos.set(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z) ;
		sVertexDisk.vNormal.set(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z) ;
		sVertexDisk.fRadius = ComputeDiskAreaPerVertex(&pMesh->psVertex[i], pMesh) ;
		pVertexDisks->push_back(sVertexDisk) ;
		TRACE("[%02d] registered Disk Pos(%3.3f %3.3f %3.3f) Normal(%3.3f %3.3f %3.3f) Radius(%3.3f)\r\n",
			nVertexCount++, enumVector(sVertexDisk.vPos), enumVector(sVertexDisk.vNormal), sVertexDisk.fRadius) ;
	}
}

void CASEData::SetVertexDisk()
{
	int i ;
	m_VertexDisks.resize(0) ;
	for(i=0 ; i<m_nNumMesh ; i++)
	{
		SubSetVertexDisk(&m_VertexDisks, m_apMesh[i]) ;
		m_apMesh[i]->bAmbientDegree = true ;
		m_apMesh[i]->nAttr |= SMESHATTR_USEAMBIENTDEGREE ;
	}
}

bool CASEData::AnotherDiskTest(Vector3 &v1, Vector3 &v2, SVertexDisk *psCurDisk)
{
	SVertexDisk *psDisk ;
	for(UINT i=0 ; i<m_VertexDisks.size() ; i++)
	{
        psDisk = &m_VertexDisks[i] ;

		if(psCurDisk != psDisk)
		{
			if(IntersectDiskToRay(v1, v2, psDisk->vPos, psDisk->vNormal, psDisk->fRadius))
				return false ;
		}
	}
	return true ;
}

struct SOccludeMesh
{
	SMesh *psMesh ;
	float fDistfromVisionRay ;
} ;
int compare_DistfromVisionRay_greater(const void *pv1, const void *pv2)//decreasing order 내림차순 (적은숫자 -> 큰숫자)
{
	SOccludeMesh *psOccludeMesh1 = (SOccludeMesh *)pv1 ;
	SOccludeMesh *psOccludeMesh2 = (SOccludeMesh *)pv2 ;

	float dist = psOccludeMesh1->fDistfromVisionRay - psOccludeMesh2->fDistfromVisionRay ;
    if(float_less(dist, 0.0f))
		return 1 ;
	else if(float_greater(dist, 0.0f))
		return -1 ;
	else
		return 0 ;
}

bool CASEData::TestOcclusion(geo::SLine *psLine, SMesh *psCurMesh)
{
    int i, n ;
	SMesh *psMesh ;
	Vector3 v, pos[3], vMin, vMax ;
	Matrix4 mat ;
	geo::STriangle tri ;
	bool bCullingTest ;
	geo::SAACube bbox ;
	int nOccludeMeshCount=0 ;
	SOccludeMesh *psOccludeMesh = new SOccludeMesh[m_nNumMesh] ;

	for(i=0 ; i<m_nNumMesh ; i++)
	{
        psMesh = m_apMesh[i] ;
		if(psCurMesh == psMesh)
		{
            psOccludeMesh[nOccludeMeshCount].fDistfromVisionRay = 0 ;
			psOccludeMesh[nOccludeMeshCount++].psMesh = psMesh ;
			continue ;
		}

		bCullingTest = (psMesh->nAttr&SMESHATTR_TWOSIDED) ? true : false ;

        vMin.set(psMesh->vMin.x, psMesh->vMin.y, psMesh->vMin.z) ;
		vMax.set(psMesh->vMax.x, psMesh->vMax.y, psMesh->vMax.z) ;

		v.x = vMin.x+((vMax.x-vMin.x)/2.0f) ;
		v.y = vMin.y+((vMax.y-vMin.y)/2.0f) ;
		v.z = vMin.z+((vMax.z-vMin.z)/2.0f) ;
		bbox.set(v, (vMax.x-vMin.x)/2.0f, (vMax.y-vMin.y)/2.0f, (vMax.z-vMin.z)/2.0f) ;

		if(IntersectLineToCube(psLine, &bbox))
		{
			psOccludeMesh[nOccludeMeshCount].fDistfromVisionRay = IntersectLinetoPoint(*psLine, v) ;
			psOccludeMesh[nOccludeMeshCount++].psMesh = psMesh ;
		}
	}

    qsort(psOccludeMesh, nOccludeMeshCount, sizeof(SOccludeMesh), compare_DistfromVisionRay_greater) ;

	for(i=0 ; i<nOccludeMeshCount ; i++)
	{
		psMesh = psOccludeMesh[i].psMesh ;
        for(n=0 ; n<psMesh->nNumTriangle ; n++)
		{
			memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
			//mat.m41 = mat.m42 = mat.m43 = 0.0f ;
			mat = mat.DecomposeRotation() ;

			v.set(psMesh->psTriangle[n].sFaceNormal.x, psMesh->psTriangle[n].sFaceNormal.y, psMesh->psTriangle[n].sFaceNormal.z) ;
			v *= mat ;
			if(float_greater(v.dot(psLine->v), 0.0f))
				continue ;

			pos[0].set(psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[0]].pos.x, psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[0]].pos.y, psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[0]].pos.z) ;
			pos[1].set(psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[1]].pos.x, psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[1]].pos.y, psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[1]].pos.z) ;
			pos[2].set(psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[2]].pos.x, psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[2]].pos.y, psMesh->psVertex[psMesh->psTriangle[n].anVertexIndex[2]].pos.z) ;

			memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
			pos[0] *= mat ;
			pos[1] *= mat ;
			pos[2] *= mat ;

			if(vector_eq(pos[0], pos[1]) || vector_eq(pos[0], pos[2]) || vector_eq(pos[1], pos[2]))
			{
				int sex=1000 ;
				//TRACE("It isn't triangle meshname=%s num=%d\r\n", psMesh->szMeshName, n) ;
			}
			else
			{                
				tri.set(pos[0], pos[1], pos[2]) ;
				if(IntersectLinetoTriangle(*psLine, tri, v, bCullingTest) == geo::INTERSECT_POINT)
				{
					SAFE_DELETEARRAY(psOccludeMesh) ;
					return true ;
				}
			}
		}
		Sleep(1) ;
	}

	SAFE_DELETEARRAY(psOccludeMesh) ;

	return false ;
}

struct SAOParameter
{
	bool bProcEnd ;
	CASEData *pcASEData ;
	int nVertexCount ;
	int nNumMesh ;
	HANDLE hThread ;
	DWORD dwThreadID ;
} ;

DWORD ComputeAOProc(SAOParameter *psParam)
{
	int n, t ;
	SMesh *psMesh ;
	Quaternion q1 ;
	Matrix4 matRot, mat ;
	Vector3 v, vDir, vOriginNormal(0, 1, 0) ;
	geo::SLine sLine ;
	sLine.d = 10000.0f ;
	float fOcclusion ;
	CASEData *pcASEData = psParam->pcASEData ;
	int nNumMesh = psParam->nNumMesh ;

	psMesh = pcASEData->m_apMesh[nNumMesh] ;
	for(n=0 ; n<psMesh->nNumVertex ; n++)
	{
		//q1.TwoVectorsToQuat(Vector3(0, 1, 0), Vector3(psMesh->psVertex[n].normal.x, psMesh->psVertex[n].normal.y, psMesh->psVertex[n].normal.z)) ;
		v.set(psMesh->psVertex[n].normal.x, psMesh->psVertex[n].normal.y, psMesh->psVertex[n].normal.z) ;
		memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
		mat.m41 = mat.m42 = mat.m43 = 0.0f ;
		v *= mat ;

		if(vector_eq(v, vOriginNormal))
		{
			matRot.Identity() ;
			TRACE("same vector\r\n") ;
		}
		else if(float_eq((vOriginNormal+v).Magnitude(), 0.0f))
		{
			matRot.Identity() ;
			matRot.m11 = matRot.m22 = matRot.m33 = -1 ;
			TRACE("negative vector\r\n") ;
		}
		else
		{
			q1.TwoVectorsToQuat(Vector3(0, 1, 0), v) ;
			q1.GetMatrix(matRot) ;
		}

		sLine.s.set(psMesh->psVertex[n].pos.x, psMesh->psVertex[n].pos.y, psMesh->psVertex[n].pos.z) ;
		memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
		sLine.s *= mat ;

		fOcclusion = (float)nNumVisibleVector ;

		for(t=0 ; t<nNumVisibleVector ; t++)
		{
			sLine.v = avVisibleVectors[t]*matRot ;
			if(pcASEData->TestOcclusion(&sLine, psMesh))
				fOcclusion -= 1.0f ;
		}
		psMesh->psVertex[n].fAmbientDegree = fOcclusion/(float)nNumVisibleVector ;
		psParam->nVertexCount++ ;
		//TRACE("[%04d] vertex ambient degree = %10.06f\r\n", n, psMesh->psVertex[n].fAmbientDegree) ;
	}
	psMesh->nAttr |= SMESHATTR_USEAMBIENTDEGREE ;

	psParam->bProcEnd = true ;
	return 0 ;
}
void CASEData::PreComputeAmbientOcclusionPerVertex(int &nTotalVertex, int &nCurVertexCount, int &nCurRatio)
{
	int i ;
	SAOParameter *psAOParameter = new SAOParameter[m_nNumMesh] ;

	for(i=0 ; i<m_nNumMesh ; i++)
	{
		psAOParameter[i].bProcEnd = false ;
		psAOParameter[i].pcASEData = this ;
		psAOParameter[i].nNumMesh = i ;
		psAOParameter[i].nVertexCount = 0 ;
		psAOParameter[i].hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ComputeAOProc, &psAOParameter[i], 0, &psAOParameter[i].dwThreadID) ;
		//nTotalVertex += m_apMesh[i]->nNumVertex ;
	}

	TRACE("Total VertexCount=%d\r\n", nTotalVertex) ;

	bool bAllThreadEnd = true ;
	while(true)
	{
		bAllThreadEnd = true ;
		nCurVertexCount = 0 ;
        for(i=0 ; i<m_nNumMesh ; i++)
		{
			if(!psAOParameter[i].bProcEnd)
				bAllThreadEnd = false ;

			nCurVertexCount += psAOParameter[i].nVertexCount ;
		}

        nCurRatio = nCurVertexCount*100/nTotalVertex ;

		if(bAllThreadEnd)
		{
			for(i=0 ; i<m_nNumMesh ; i++)
				CloseHandle(psAOParameter[i].hThread) ;

			int count=0 ;
			for(i=0 ; i<m_nNumMesh ; i++)
			{
				SMesh *psMesh = m_apMesh[i] ;
				for(int n=0 ; n<psMesh->nNumVertex ; n++)
				{
					TRACE("[%04d]vertex ambientdegree=%06.04f\r\n", count, psMesh->psVertex[n].fAmbientDegree) ;
					count++ ;
				}
			}

			break ;
		}

		Sleep(1) ;
	}
	SAFE_DELETEARRAY(psAOParameter) ;
}
/*
void CASEData::PreComputeAmbientOcclusionPerVertex()
{
	int i, n, t ;
	SMesh *psMesh ;
	Quaternion q1 ;
	Matrix4 matRot, mat ;
	Vector3 v, vDir, vOriginNormal(0, 1, 0) ;
	geo::SLine sLine ;
	sLine.d = 10000.0f ;
	float fOcclusion ;

	for(i=0 ; i<m_nNumMesh ; i++)
	{
		psMesh = m_apMesh[i] ;
		for(n=0 ; n<psMesh->nNumVertex ; n++)
		{
			//q1.TwoVectorsToQuat(Vector3(0, 1, 0), Vector3(psMesh->psVertex[n].normal.x, psMesh->psVertex[n].normal.y, psMesh->psVertex[n].normal.z)) ;
			v.set(psMesh->psVertex[n].normal.x, psMesh->psVertex[n].normal.y, psMesh->psVertex[n].normal.z) ;
			memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
			mat.m41 = mat.m42 = mat.m43 = 0.0f ;
			v *= mat ;

			if(vector_eq(v, vOriginNormal))
			{
				matRot.Identity() ;
				TRACE("same vector\r\n") ;
			}
			else if(float_eq((vOriginNormal+v).Magnitude(), 0.0f))
			{
				matRot.Identity() ;
				matRot.m11 = matRot.m22 = matRot.m33 = -1 ;
				TRACE("negative vector\r\n") ;
			}
			else
			{
				q1.TwoVectorsToQuat(Vector3(0, 1, 0), v) ;
				q1.GetMatrix(matRot) ;
			}

			sLine.s.set(psMesh->psVertex[n].pos.x, psMesh->psVertex[n].pos.y, psMesh->psVertex[n].pos.z) ;
			memcpy((void *)&mat, (void *)&psMesh->smatLocal, sizeof(Matrix4)) ;
			sLine.s *= mat ;

			fOcclusion = (float)nNumVisibleVector ;

			for(t=0 ; t<nNumVisibleVector ; t++)
			{
				sLine.v = avVisibleVectors[t]*matRot ;
				if(TestOcclusion(&sLine, psMesh))
					fOcclusion -= 1.0f ;
			}
			psMesh->psVertex[n].fAmbientDegree = fOcclusion/(float)nNumVisibleVector ;
			TRACE("[%04d] vertex ambient degree = %10.06f\r\n", n, psMesh->psVertex[n].fAmbientDegree) ;
		}
		m_apMesh[i]->nAttr |= SMESHATTR_USEAMBIENTDEGREE ;
	}
}

/*
void CASEData::PreComputedAmbientOcclusionPerVertex()
{
    int recv, emt ;
	Vector3 vDist ;
	float ftemp, fSolidAngle, fRadius, fHemisphere=(2.0f*3.141592f), fBestSolidAngle=0 ;

	for(recv=0 ; recv<(int)m_VertexDisks.size() ; recv++)
	{
		//TRACE("### SolidAngle Test START ###\r\n") ;

		if(recv == 87)
			TRACE("### SolidAngle Test START ###\r\n") ;

		fSolidAngle = 0 ;
        for(emt=0 ; emt<(int)m_VertexDisks.size() ; emt++)
		{
			vDist = m_VertexDisks[emt].vPos - m_VertexDisks[recv].vPos ;
            if((vDist.Magnitude() > 0))
			//if((vDist.Magnitude() > 0) && (m_VertexDisks[recv].vNormal.dot(m_VertexDisks[emt].vNormal) != (0+epsilon)))
			{
				//if( (m_VertexDisks[recv].vNormal.dot(m_VertexDisks[emt].vNormal)) != 0.0f// (0.0f+epsilon)
					if((m_VertexDisks[recv].vNormal.dot(m_VertexDisks[emt].vNormal) < 0)
					&& (m_VertexDisks[recv].vNormal.dot(vDist.Normalize()) > 0)
					&& AnotherDiskTest(m_VertexDisks[recv].vPos, m_VertexDisks[emt].vPos, &m_VertexDisks[emt]))
				//if(m_VertexDisks[recv].vNormal.dot(vDist.Normalize()) > 0)//다른 디스크가 현재 디스크의 노말방향의 앞쪽에 있다면
				{
					if(emt >= 375)
						fRadius = 0 ;

					float ftheta = IncludedAngle((-vDist).Normalize(), m_VertexDisks[emt].vNormal) ;

					Vector3 vDistNormal, vEmtNormal ;

					vDistNormal = (-vDist).Normalize() ;
					vEmtNormal = m_VertexDisks[emt].vNormal ;

					float dot = vDistNormal.dot(vEmtNormal) ;
					
					if(float_eq(dot, 0.0f) || float_eq(dot, 1.0f) || float_eq(dot, -1.0f))
						dot = dot ;

                    ftheta = IncludedAngle(vDistNormal, vEmtNormal) ;

					if(ftheta > (3.141592f/2.0f))
						ftheta = 3.141592f-ftheta ;

					fRadius = cosf(ftheta)*m_VertexDisks[emt].fRadius ;
                    
					//F:\Study\3D 관련자료\Grobal Illumination\AmbientOcclusion\DynamicAmbientOcclusion_OPerez.pdf 참조
					//입체각 공식 2pi(1-cos(theta))
					//theta = atan(EmtterRadius/Distance)
					ftemp = 2.0f*3.141592f*(1.0f - cosf(atanf(fRadius/vDist.Magnitude()))) ;
					fSolidAngle += ftemp ;

					//TRACE("[emt=%03d] SolidAngle=%3.3f vDist(%3.3f, %3.3f, %3.3f) Radius(emt)=%3.3f\r\n",
					//	emt, ftemp, enumVector(vDist), fRadius) ;

					//TRACE("[emt=%2d] SolidAngle=%3.3f vDist(%3.3f, %3.3f, %3.3f) Radius(emt)=%3.3f\r\n", emt, ftemp, enumVector(vDist), m_VertexDisks[emt].fRadius) ;
					//TRACE(" recvNormal(%3.3f, %3.3f, %3.3f) emtNormal(%3.3f, %3.3f, %3.3f)\r\n", enumVector(m_VertexDisks[recv].vNormal), enumVector(m_VertexDisks[emt].vNormal));
					//TRACE("normal to normal dot %3.3f \r\n", (m_VertexDisks[recv].vNormal.dot(m_VertexDisks[emt].vNormal))) ;
				}
			}
		}

		if(fBestSolidAngle < fSolidAngle)
			fBestSolidAngle = fSolidAngle ;

		m_VertexDisks[recv].fAmbientDegree = 1.0f - (fSolidAngle/fHemisphere) ;
		TRACE("[recv=%2d] pos(%3.3f %3.3f %3.3f) Total TotalSolidAngle=%3.3f ratio=%3.3f\r\n",
			recv, enumVector(m_VertexDisks[recv].vPos), fSolidAngle, m_VertexDisks[recv].fAmbientDegree) ;

		//TRACE("### SolidAngle Test END ###\r\n") ;
	}
	//TRACE("BestSolidAngle=%3.3f\r\n", fBestSolidAngle) ;

	UINT i, n ;
	bool bEqual ;
	std::vector<SVertexDisk> Disks ;
	Disks.resize(0) ;

	for(i=0 ; i<(int)m_VertexDisks.size() ; i++)
	{
		bEqual = false ;
		for(n=0 ; n<(int)Disks.size() ; n++)
		{
			if(vector_eq(Disks[n].vPos, m_VertexDisks[i].vPos))
			{
				Disks[n].fAmbientDegree += m_VertexDisks[i].fAmbientDegree ;
				Disks[n].nAccumulatedPos++ ;
				bEqual = true ;
				break ;
			}
		}
		if(!bEqual)
		{
			Disks.push_back(m_VertexDisks[i]) ;
			//TRACE("Input Disk pos(%3.3f %3.3f %3.3f)\r\n", enumVector(m_VertexDisks[i].vPos)) ;
			(Disks.end()-1)->nAccumulatedPos = 1 ;
			//TRACE("Disks.push_back pos(%3.3f %3.3f %3.3f)\r\n", enumVector((Disks.end()-1)->vPos)) ;
		}
	}

	//TRACE("calculate AmbientDegree\r\n") ;
	for(i=0 ; i<Disks.size() ; i++)
	{
		//float degree = Disks[i].fAmbientDegree ;
		//float accmulated = Disks[i].nAccumulatedPos ;
		//TRACE("[%03d] degree=%3.3f accmulated=%3.3f result=%3.3f \r\n", i, degree, accmulated, degree/accmulated) ;
		Disks[i].fAmbientDegree = Disks[i].fAmbientDegree/Disks[i].nAccumulatedPos ;
	}


	for(i=0 ; i<Disks.size() ; i++)
	{
		for(n=0 ; n<m_VertexDisks.size() ; n++)
		{
			if(vector_eq(Disks[i].vPos, m_VertexDisks[n].vPos))
			{
				m_VertexDisks[n].fAmbientDegree = max(0.1f, Disks[i].fAmbientDegree) ;
			}
		}
	}

	recv = 0 ;
	for(i=0 ; i<(UINT)m_nNumMesh ; i++)
	{
		for(n=0 ; n<(UINT)m_apMesh[i]->nNumVertex ; n++)
		{
			m_apMesh[i]->psVertex[n].fAmbientDegree = m_VertexDisks[recv++].fAmbientDegree ;
			TRACE("[%2d] Vertex AmbientDegree=%3.3f\r\n", n, m_apMesh[i]->psVertex[n].fAmbientDegree) ;
		}
	}
}
*/
#include "GeoLib.h"
#include "SimpleQuadTree.h"

CSimpleQuadTree *CASEData::BuildSimpleQuadTree()
{
	int i, n ;
	SMesh *pMesh ;

	m_vMin = m_vMax = m_apMesh[0]->psVertex->pos ;

	int nNumTriangle=0 ;
	for(i=0 ; i<m_nNumMesh ; i++)
	{
		nNumTriangle += m_apMesh[i]->nNumTriangle ;

		pMesh = m_apMesh[i] ;
		for(n=0 ; n<pMesh->nNumVertex ; n++)
		{
			if(m_vMin.x > pMesh->psVertex[n].pos.x)
				m_vMin.x = pMesh->psVertex[n].pos.x ;
			if(m_vMax.x < pMesh->psVertex[n].pos.x)
				m_vMax.x = pMesh->psVertex[n].pos.x ;

			if(m_vMin.y > pMesh->psVertex[n].pos.y)
				m_vMin.y = pMesh->psVertex[n].pos.y ;
			if(m_vMax.y < pMesh->psVertex[n].pos.y)
				m_vMax.y = pMesh->psVertex[n].pos.y ;

			if(m_vMin.z > pMesh->psVertex[n].pos.z)
				m_vMin.z = pMesh->psVertex[n].pos.z ;
			if(m_vMax.z < pMesh->psVertex[n].pos.z)
				m_vMax.z = pMesh->psVertex[n].pos.z ;
		}
	}

	m_nWidth = CloseToExp(m_vMax.x-m_vMin.x, 2) ;
	m_nHeight = CloseToExp(m_vMax.z-m_vMin.z, 2) ;

	m_psTriangles = new geo::STriangle[nNumTriangle] ;
    
	int count=0 ;
	for(i=0 ; i<m_nNumMesh ; i++)
	{
		pMesh = m_apMesh[i] ;
		for(n=0 ; n<pMesh->nNumTriangle ; n++, count++)
		{
			m_psTriangles[count].set(
				Vector3(pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[0]].pos.x, pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[0]].pos.y, pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[0]].pos.z),
				Vector3(pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[1]].pos.x, pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[1]].pos.y, pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[1]].pos.z),
				Vector3(pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[2]].pos.x, pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[2]].pos.y, pMesh->psVertex[pMesh->psTriangle[n].anVertexIndex[2]].pos.z)) ;

			m_psTriangles[count].avVertex[0].vNormal.set(
				pMesh->psTriangle[n].asVertexNormal[0].x, pMesh->psTriangle[n].asVertexNormal[0].y, pMesh->psTriangle[n].asVertexNormal[0].z) ;

			m_psTriangles[count].avVertex[1].vNormal.set(
				pMesh->psTriangle[n].asVertexNormal[1].x, pMesh->psTriangle[n].asVertexNormal[1].y, pMesh->psTriangle[n].asVertexNormal[1].z) ;

			m_psTriangles[count].avVertex[2].vNormal.set(
				pMesh->psTriangle[n].asVertexNormal[2].x, pMesh->psTriangle[n].asVertexNormal[2].y, pMesh->psTriangle[n].asVertexNormal[2].z) ;

			m_psTriangles[count].avVertex[0].vTex.set(pMesh->psTriangle[n].asTexCoord[0].u, pMesh->psTriangle[n].asTexCoord[0].v) ;
			m_psTriangles[count].avVertex[1].vTex.set(pMesh->psTriangle[n].asTexCoord[1].u, pMesh->psTriangle[n].asTexCoord[1].v) ;
			m_psTriangles[count].avVertex[2].vTex.set(pMesh->psTriangle[n].asTexCoord[2].u, pMesh->psTriangle[n].asTexCoord[2].v) ;

			//TRACE("[%03d] Triangle 0(%07.03f %07.03f %07.03f) 1(%07.03f %07.03f %07.03f) 2(%07.03f %07.03f %07.03f)\r\n", count,
			//	enumVector(psTriangle[count].avVertex[0].vPos), enumVector(psTriangle[count].avVertex[1].vPos), enumVector(psTriangle[count].avVertex[2].vPos)) ;
		}
	}

    CSimpleQuadTree *pcQuadTree = new CSimpleQuadTree((float)m_nWidth, (float)m_nHeight) ;
	pcQuadTree->Build(m_psTriangles, count) ;
	return pcQuadTree ;
}

void MakeTangent(SMeshVertex *psVertices, int nNumTriangle, SMeshTriangle *psTriangles)
{
	int n ;
	for(n=0 ; n<nNumTriangle ; n++)
	{
		ComputeTangent(psVertices[psTriangles->anVertexIndex[0]], psVertices[psTriangles->anVertexIndex[1]], psVertices[psTriangles->anVertexIndex[2]]) ;
		ComputeTangent(psVertices[psTriangles->anVertexIndex[1]], psVertices[psTriangles->anVertexIndex[0]], psVertices[psTriangles->anVertexIndex[2]]) ;
		ComputeTangent(psVertices[psTriangles->anVertexIndex[2]], psVertices[psTriangles->anVertexIndex[0]], psVertices[psTriangles->anVertexIndex[1]]) ;
		psTriangles++ ;
	}
}

#include "D3Ddef.h"

struct SThreadPack_BuildHorizon
{
	STerrain_FragmentInfo *psFragmentInfo ;
	float x, z ;
    int nWidth, nHeight ;
	float fStep ;

	DWORD dwThreadID ;
	HANDLE hThread ;
} ;

DWORD HorizonBuildFragments(SThreadPack_BuildHorizon *psPack) ;

void CASEData::BuildTerrainFile(char *pszFileName, CSimpleQuadTree *pcQuadTree)
{
	int nWidth=m_nWidth, nHeight=m_nHeight ;
    float fnumerator   = 1 ;
	float fdenominator = 1 ;
	float fStep = fnumerator/fdenominator ;
	geo::STriangle sTriangle ;
	geo::SLine sLine ;
	STerrain_FragmentInfo *psFragmentInfo = NULL ;
	float x, z ;
	Vector3 vPos, vBaryCenter ;
	int nNumFragments = ((int)(nWidth/fStep)+1) * ((int)(nHeight/fStep)+1) ;
	int nCount=0 ;

	psFragmentInfo = new STerrain_FragmentInfo[nNumFragments] ;

	SThreadPack_BuildHorizon *psPacks = new SThreadPack_BuildHorizon[nHeight*2] ;
	int nThreadCount=0 ;

	for(z=(float)nHeight/2.0f; z>=(float)(-nHeight/2.0f); z-=fStep)
	{
		TRACE("Z=%g\r\n", z) ;
		for(x=(float)(-nWidth/2.0f); x<=(float)nWidth/2.0f; x+=fStep, nCount++)
		{
			vPos.set(x, 0, z) ;
			pcQuadTree->GetIntersectedPos(&vPos, 0) ;

			psFragmentInfo[nCount].fElevation = vPos.y ;

			//TRACE("[%03d]pos(%07.03f %07.03f %07.03f)\r\n", nCount, enumVector(vPos)) ;
			//TRACE("\r\n") ;

			psFragmentInfo[nCount].u = x/nWidth + 0.5f ;
			psFragmentInfo[nCount].v = 0.5f - z/nHeight ;
			psFragmentInfo[nCount].nMaterialID = 0 ;
		}

		//psPacks[nThreadCount].z = z ;
		//psPacks[nThreadCount].x = 0 ;
		//psPacks[nThreadCount].fStep = fStep ;
		//psPacks[nThreadCount].nWidth = nWidth ;
		//psPacks[nThreadCount].nHeight = nHeight ;
		//psPacks[nThreadCount].pcOctTree = m_pcOctTree ;
		//psPacks[nThreadCount].psFragmentInfo = &psFragmentInfo[(nThreadCount*nWidth)] ;
		//psPacks[nThreadCount].psTerrainMesh = &m_sTerrainMesh ;
		//psPacks[nThreadCount].hThread = NULL ;

  //      psPacks[nThreadCount].hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HorizonBuildFragments, &psPacks[nThreadCount], 0, &psPacks[nThreadCount].dwThreadID) ;

		//nThreadCount++ ;

		//Sleep(10) ;
	}

	//bool bAllThreadEnd ;
	//while(true)
	//{
	//	bAllThreadEnd = true ;

 //       for(int i=0 ; i<nHeight ; i++)
	//	{
	//		if(psPacks[i].hThread != NULL)
	//			bAllThreadEnd = false ;
	//	}

	//	if(bAllThreadEnd)
	//		break ;

	//	Sleep(10) ;
	//}


	STerrain_FileHeader *psFileHeader=NULL ;
	psFileHeader = new STerrain_FileHeader ;
	
	psFileHeader->dwType = 't' + 'r' + 'n' ;
	psFileHeader->lNumMaterial = 0 ;
	psFileHeader->lWidth = nWidth ;
	psFileHeader->lHeight = nHeight ;
	psFileHeader->nNumerator = (int)fnumerator ;
	psFileHeader->nDenominator = (int)fdenominator ;

	STerrain_File sTerrainFile ;
	sTerrainFile.psFileHeader = psFileHeader ;
	sTerrainFile.psMaterialInfo = NULL ;
	sTerrainFile.psFragmentInfo = psFragmentInfo ;

    //SaveTerrainFile(psFileHeader, NULL, psFragmentInfo, pszFileName) ;
	SaveTerrainFile(&sTerrainFile, pszFileName) ;

	SAFE_DELETE(psFileHeader) ;

	SAFE_DELETEARRAY(psPacks) ;
}

DWORD HorizonBuildFragments(SThreadPack_BuildHorizon *psPack)
{
	float x ;
	int nCount = 0 ;
	Vector3 vPos ;

	TRACE("Horizon Build Thread Start id:%d\r\n", psPack->dwThreadID) ;

	for(x=(float)(-psPack->nWidth/2.0f) ; x<= (float)(psPack->nWidth/2.0f) ; x += psPack->fStep, psPack->psFragmentInfo++)
	{
		vPos.set(x, 0, psPack->z) ;
		//if(psPack->pcOctTree->GetIntersectedPos(&vPos, psPack->psTerrainMesh->psVertices) == 0)
		//	vPos.y = psPack->psTerrainMesh->vMinPos.y ;

		psPack->psFragmentInfo->fElevation = vPos.y ;
		psPack->psFragmentInfo->u = x/psPack->nWidth + 0.5f ;
		psPack->psFragmentInfo->v = 0.5f - psPack->z/psPack->nHeight ;
		psPack->psFragmentInfo->nMaterialID = 0 ;

		Sleep(1) ;
	}

	DWORD id = psPack->dwThreadID ;
	TRACE("Horizon Build Thread End id:%d\r\n", id) ;
	CloseHandle(psPack->hThread) ;
	psPack->hThread = NULL ;
    return 0 ;
}

Vector3 avVisibleVectors[] =//481
{
Vector3(  0.000000f,   1.000000f,   0.000000f),
Vector3(  0.000000f,   0.994525f,   0.104503f),
Vector3( -0.020400f,   0.994524f,   0.102502f),
Vector3( -0.040001f,   0.994519f,   0.096602f),
Vector3( -0.058101f,   0.994521f,   0.086902f),
Vector3( -0.073902f,   0.994524f,   0.073902f),
Vector3( -0.086902f,   0.994521f,   0.058101f),
Vector3( -0.096602f,   0.994519f,   0.040001f),
Vector3( -0.102502f,   0.994524f,   0.020400f),
Vector3( -0.104503f,   0.994525f,   0.000000f),
Vector3( -0.102502f,   0.994524f,  -0.020400f),
Vector3( -0.096602f,   0.994519f,  -0.040001f),
Vector3( -0.086902f,   0.994521f,  -0.058101f),
Vector3( -0.073902f,   0.994524f,  -0.073902f),
Vector3( -0.058101f,   0.994521f,  -0.086902f),
Vector3( -0.040001f,   0.994519f,  -0.096602f),
Vector3( -0.020400f,   0.994524f,  -0.102502f),
Vector3(  0.000000f,   0.994525f,  -0.104503f),
Vector3(  0.020400f,   0.994524f,  -0.102502f),
Vector3(  0.040001f,   0.994519f,  -0.096602f),
Vector3(  0.058101f,   0.994521f,  -0.086902f),
Vector3(  0.073902f,   0.994524f,  -0.073902f),
Vector3(  0.086902f,   0.994521f,  -0.058101f),
Vector3(  0.096602f,   0.994519f,  -0.040001f),
Vector3(  0.102502f,   0.994524f,  -0.020400f),
Vector3(  0.104503f,   0.994525f,   0.000000f),
Vector3(  0.102502f,   0.994524f,   0.020400f),
Vector3(  0.096602f,   0.994519f,   0.040001f),
Vector3(  0.086902f,   0.994521f,   0.058101f),
Vector3(  0.073902f,   0.994524f,   0.073902f),
Vector3(  0.058101f,   0.994521f,   0.086902f),
Vector3(  0.040001f,   0.994519f,   0.096602f),
Vector3(  0.020400f,   0.994524f,   0.102502f),
Vector3(  0.000000f,   0.978148f,   0.207910f),
Vector3( -0.040602f,   0.978147f,   0.203910f),
Vector3( -0.079603f,   0.978140f,   0.192108f),
Vector3( -0.115505f,   0.978142f,   0.172907f),
Vector3( -0.147008f,   0.978150f,   0.147008f),
Vector3( -0.172907f,   0.978142f,   0.115505f),
Vector3( -0.192108f,   0.978140f,   0.079603f),
Vector3( -0.203910f,   0.978147f,   0.040602f),
Vector3( -0.207910f,   0.978148f,   0.000000f),
Vector3( -0.203910f,   0.978147f,  -0.040602f),
Vector3( -0.192108f,   0.978140f,  -0.079603f),
Vector3( -0.172907f,   0.978142f,  -0.115505f),
Vector3( -0.147008f,   0.978150f,  -0.147008f),
Vector3( -0.115505f,   0.978142f,  -0.172907f),
Vector3( -0.079603f,   0.978140f,  -0.192108f),
Vector3( -0.040602f,   0.978147f,  -0.203910f),
Vector3(  0.000000f,   0.978148f,  -0.207910f),
Vector3(  0.040602f,   0.978147f,  -0.203910f),
Vector3(  0.079603f,   0.978140f,  -0.192108f),
Vector3(  0.115505f,   0.978142f,  -0.172907f),
Vector3(  0.147008f,   0.978150f,  -0.147008f),
Vector3(  0.172907f,   0.978142f,  -0.115505f),
Vector3(  0.192108f,   0.978140f,  -0.079603f),
Vector3(  0.203910f,   0.978147f,  -0.040602f),
Vector3(  0.207910f,   0.978148f,   0.000000f),
Vector3(  0.203910f,   0.978147f,   0.040602f),
Vector3(  0.192108f,   0.978140f,   0.079603f),
Vector3(  0.172907f,   0.978142f,   0.115505f),
Vector3(  0.147008f,   0.978150f,   0.147008f),
Vector3(  0.115505f,   0.978142f,   0.172907f),
Vector3(  0.079603f,   0.978140f,   0.192108f),
Vector3(  0.040602f,   0.978147f,   0.203910f),
Vector3(  0.000000f,   0.951066f,   0.308989f),
Vector3( -0.060297f,   0.951054f,   0.303085f),
Vector3( -0.118294f,   0.951054f,   0.285486f),
Vector3( -0.171694f,   0.951067f,   0.256891f),
Vector3( -0.218492f,   0.951064f,   0.218492f),
Vector3( -0.256891f,   0.951067f,   0.171694f),
Vector3( -0.285486f,   0.951054f,   0.118294f),
Vector3( -0.303085f,   0.951054f,   0.060297f),
Vector3( -0.308989f,   0.951066f,   0.000000f),
Vector3( -0.303085f,   0.951054f,  -0.060297f),
Vector3( -0.285486f,   0.951054f,  -0.118294f),
Vector3( -0.256891f,   0.951067f,  -0.171694f),
Vector3( -0.218492f,   0.951064f,  -0.218492f),
Vector3( -0.171694f,   0.951067f,  -0.256891f),
Vector3( -0.118294f,   0.951054f,  -0.285486f),
Vector3( -0.060297f,   0.951054f,  -0.303085f),
Vector3(  0.000000f,   0.951066f,  -0.308989f),
Vector3(  0.060297f,   0.951054f,  -0.303085f),
Vector3(  0.118294f,   0.951054f,  -0.285486f),
Vector3(  0.171694f,   0.951067f,  -0.256891f),
Vector3(  0.218492f,   0.951064f,  -0.218492f),
Vector3(  0.256891f,   0.951067f,  -0.171694f),
Vector3(  0.285486f,   0.951054f,  -0.118294f),
Vector3(  0.303085f,   0.951054f,  -0.060297f),
Vector3(  0.308989f,   0.951066f,   0.000000f),
Vector3(  0.303085f,   0.951054f,   0.060297f),
Vector3(  0.285486f,   0.951054f,   0.118294f),
Vector3(  0.256891f,   0.951067f,   0.171694f),
Vector3(  0.218492f,   0.951064f,   0.218492f),
Vector3(  0.171694f,   0.951067f,   0.256891f),
Vector3(  0.118294f,   0.951054f,   0.285486f),
Vector3(  0.060297f,   0.951054f,   0.303085f),
Vector3(  0.000000f,   0.913552f,   0.406723f),
Vector3( -0.079404f,   0.913542f,   0.398918f),
Vector3( -0.155704f,   0.913523f,   0.375809f),
Vector3( -0.226007f,   0.913529f,   0.338211f),
Vector3( -0.287613f,   0.913541f,   0.287613f),
Vector3( -0.338211f,   0.913529f,   0.226007f),
Vector3( -0.375809f,   0.913523f,   0.155704f),
Vector3( -0.398918f,   0.913542f,   0.079404f),
Vector3( -0.406723f,   0.913552f,   0.000000f),
Vector3( -0.398918f,   0.913542f,  -0.079404f),
Vector3( -0.375809f,   0.913523f,  -0.155704f),
Vector3( -0.338211f,   0.913529f,  -0.226007f),
Vector3( -0.287613f,   0.913541f,  -0.287613f),
Vector3( -0.226007f,   0.913529f,  -0.338211f),
Vector3( -0.155704f,   0.913523f,  -0.375809f),
Vector3( -0.079404f,   0.913542f,  -0.398918f),
Vector3(  0.000000f,   0.913552f,  -0.406723f),
Vector3(  0.079404f,   0.913542f,  -0.398918f),
Vector3(  0.155704f,   0.913523f,  -0.375809f),
Vector3(  0.226007f,   0.913529f,  -0.338211f),
Vector3(  0.287613f,   0.913541f,  -0.287613f),
Vector3(  0.338211f,   0.913529f,  -0.226007f),
Vector3(  0.375809f,   0.913523f,  -0.155704f),
Vector3(  0.398918f,   0.913542f,  -0.079404f),
Vector3(  0.406723f,   0.913552f,   0.000000f),
Vector3(  0.398918f,   0.913542f,   0.079404f),
Vector3(  0.375809f,   0.913523f,   0.155704f),
Vector3(  0.338211f,   0.913529f,   0.226007f),
Vector3(  0.287613f,   0.913541f,   0.287613f),
Vector3(  0.226007f,   0.913529f,   0.338211f),
Vector3(  0.155704f,   0.913523f,   0.375809f),
Vector3(  0.079404f,   0.913542f,   0.398918f),
Vector3(  0.000000f,   0.866019f,   0.500011f),
Vector3( -0.097502f,   0.866020f,   0.490411f),
Vector3( -0.191309f,   0.866042f,   0.461922f),
Vector3( -0.277809f,   0.866028f,   0.415713f),
Vector3( -0.353596f,   0.865991f,   0.353596f),
Vector3( -0.415713f,   0.866028f,   0.277809f),
Vector3( -0.461922f,   0.866042f,   0.191309f),
Vector3( -0.490411f,   0.866020f,   0.097502f),
Vector3( -0.500011f,   0.866019f,   0.000000f),
Vector3( -0.490411f,   0.866020f,  -0.097502f),
Vector3( -0.461922f,   0.866042f,  -0.191309f),
Vector3( -0.415713f,   0.866028f,  -0.277809f),
Vector3( -0.353596f,   0.865991f,  -0.353596f),
Vector3( -0.277809f,   0.866028f,  -0.415713f),
Vector3( -0.191309f,   0.866042f,  -0.461922f),
Vector3( -0.097502f,   0.866020f,  -0.490411f),
Vector3(  0.000000f,   0.866019f,  -0.500011f),
Vector3(  0.097502f,   0.866020f,  -0.490411f),
Vector3(  0.191309f,   0.866042f,  -0.461922f),
Vector3(  0.277809f,   0.866028f,  -0.415713f),
Vector3(  0.353596f,   0.865991f,  -0.353596f),
Vector3(  0.415713f,   0.866028f,  -0.277809f),
Vector3(  0.461922f,   0.866042f,  -0.191309f),
Vector3(  0.490411f,   0.866020f,  -0.097502f),
Vector3(  0.500011f,   0.866019f,   0.000000f),
Vector3(  0.490411f,   0.866020f,   0.097502f),
Vector3(  0.461922f,   0.866042f,   0.191309f),
Vector3(  0.415713f,   0.866028f,   0.277809f),
Vector3(  0.353596f,   0.865991f,   0.353596f),
Vector3(  0.277809f,   0.866028f,   0.415713f),
Vector3(  0.191309f,   0.866042f,   0.461922f),
Vector3(  0.097502f,   0.866020f,   0.490411f),
Vector3(  0.000000f,   0.809004f,   0.587803f),
Vector3( -0.114701f,   0.809004f,   0.576503f),
Vector3( -0.224910f,   0.809036f,   0.543024f),
Vector3( -0.326604f,   0.809010f,   0.488706f),
Vector3( -0.415615f,   0.809029f,   0.415615f),
Vector3( -0.488706f,   0.809010f,   0.326604f),
Vector3( -0.543024f,   0.809036f,   0.224910f),
Vector3( -0.576503f,   0.809004f,   0.114701f),
Vector3( -0.587803f,   0.809004f,   0.000000f),
Vector3( -0.576503f,   0.809004f,  -0.114701f),
Vector3( -0.543024f,   0.809036f,  -0.224910f),
Vector3( -0.488706f,   0.809010f,  -0.326604f),
Vector3( -0.415615f,   0.809029f,  -0.415615f),
Vector3( -0.326604f,   0.809010f,  -0.488706f),
Vector3( -0.224910f,   0.809036f,  -0.543024f),
Vector3( -0.114701f,   0.809004f,  -0.576503f),
Vector3(  0.000000f,   0.809004f,  -0.587803f),
Vector3(  0.114701f,   0.809004f,  -0.576503f),
Vector3(  0.224910f,   0.809036f,  -0.543024f),
Vector3(  0.326604f,   0.809010f,  -0.488706f),
Vector3(  0.415615f,   0.809029f,  -0.415615f),
Vector3(  0.488706f,   0.809010f,  -0.326604f),
Vector3(  0.543024f,   0.809036f,  -0.224910f),
Vector3(  0.576503f,   0.809004f,  -0.114701f),
Vector3(  0.587803f,   0.809004f,   0.000000f),
Vector3(  0.576503f,   0.809004f,   0.114701f),
Vector3(  0.543024f,   0.809036f,   0.224910f),
Vector3(  0.488706f,   0.809010f,   0.326604f),
Vector3(  0.415615f,   0.809029f,   0.415615f),
Vector3(  0.326604f,   0.809010f,   0.488706f),
Vector3(  0.224910f,   0.809036f,   0.543024f),
Vector3(  0.114701f,   0.809004f,   0.576503f),
Vector3(  0.000000f,   0.743140f,   0.669136f),
Vector3( -0.130503f,   0.743116f,   0.656314f),
Vector3( -0.256106f,   0.743116f,   0.618214f),
Vector3( -0.371711f,   0.743123f,   0.556417f),
Vector3( -0.473137f,   0.743158f,   0.473137f),
Vector3( -0.556417f,   0.743123f,   0.371711f),
Vector3( -0.618214f,   0.743116f,   0.256106f),
Vector3( -0.656314f,   0.743116f,   0.130503f),
Vector3( -0.669136f,   0.743140f,   0.000000f),
Vector3( -0.656314f,   0.743116f,  -0.130503f),
Vector3( -0.618214f,   0.743116f,  -0.256106f),
Vector3( -0.556417f,   0.743123f,  -0.371711f),
Vector3( -0.473137f,   0.743158f,  -0.473137f),
Vector3( -0.371711f,   0.743123f,  -0.556417f),
Vector3( -0.256106f,   0.743116f,  -0.618214f),
Vector3( -0.130503f,   0.743116f,  -0.656314f),
Vector3(  0.000000f,   0.743140f,  -0.669136f),
Vector3(  0.130503f,   0.743116f,  -0.656314f),
Vector3(  0.256106f,   0.743116f,  -0.618214f),
Vector3(  0.371711f,   0.743123f,  -0.556417f),
Vector3(  0.473137f,   0.743158f,  -0.473137f),
Vector3(  0.556417f,   0.743123f,  -0.371711f),
Vector3(  0.618214f,   0.743116f,  -0.256106f),
Vector3(  0.656314f,   0.743116f,  -0.130503f),
Vector3(  0.669136f,   0.743140f,   0.000000f),
Vector3(  0.656314f,   0.743116f,   0.130503f),
Vector3(  0.618214f,   0.743116f,   0.256106f),
Vector3(  0.556417f,   0.743123f,   0.371711f),
Vector3(  0.473137f,   0.743158f,   0.473137f),
Vector3(  0.371711f,   0.743123f,   0.556417f),
Vector3(  0.256106f,   0.743116f,   0.618214f),
Vector3(  0.130503f,   0.743116f,   0.656314f),
Vector3(  0.000000f,   0.669136f,   0.743140f),
Vector3( -0.144999f,   0.669095f,   0.728895f),
Vector3( -0.284400f,   0.669101f,   0.686601f),
Vector3( -0.412904f,   0.669106f,   0.617906f),
Vector3( -0.525501f,   0.669102f,   0.525501f),
Vector3( -0.617906f,   0.669106f,   0.412904f),
Vector3( -0.686601f,   0.669101f,   0.284400f),
Vector3( -0.728895f,   0.669095f,   0.144999f),
Vector3( -0.743140f,   0.669136f,   0.000000f),
Vector3( -0.728895f,   0.669095f,  -0.144999f),
Vector3( -0.686601f,   0.669101f,  -0.284400f),
Vector3( -0.617906f,   0.669106f,  -0.412904f),
Vector3( -0.525501f,   0.669102f,  -0.525501f),
Vector3( -0.412904f,   0.669106f,  -0.617906f),
Vector3( -0.284400f,   0.669101f,  -0.686601f),
Vector3( -0.144999f,   0.669095f,  -0.728895f),
Vector3(  0.000000f,   0.669136f,  -0.743140f),
Vector3(  0.144999f,   0.669095f,  -0.728895f),
Vector3(  0.284400f,   0.669101f,  -0.686601f),
Vector3(  0.412904f,   0.669106f,  -0.617906f),
Vector3(  0.525501f,   0.669102f,  -0.525501f),
Vector3(  0.617906f,   0.669106f,  -0.412904f),
Vector3(  0.686601f,   0.669101f,  -0.284400f),
Vector3(  0.728895f,   0.669095f,  -0.144999f),
Vector3(  0.743140f,   0.669136f,   0.000000f),
Vector3(  0.728895f,   0.669095f,   0.144999f),
Vector3(  0.686601f,   0.669101f,   0.284400f),
Vector3(  0.617906f,   0.669106f,   0.412904f),
Vector3(  0.525501f,   0.669102f,   0.525501f),
Vector3(  0.412904f,   0.669106f,   0.617906f),
Vector3(  0.284400f,   0.669101f,   0.686601f),
Vector3(  0.144999f,   0.669095f,   0.728895f),
Vector3(  0.000000f,   0.587803f,   0.809004f),
Vector3( -0.157796f,   0.587785f,   0.793479f),
Vector3( -0.309605f,   0.587810f,   0.747412f),
Vector3( -0.449481f,   0.587775f,   0.672672f),
Vector3( -0.572070f,   0.587769f,   0.572070f),
Vector3( -0.672672f,   0.587775f,   0.449481f),
Vector3( -0.747412f,   0.587810f,   0.309605f),
Vector3( -0.793479f,   0.587785f,   0.157796f),
Vector3( -0.809004f,   0.587803f,   0.000000f),
Vector3( -0.793479f,   0.587785f,  -0.157796f),
Vector3( -0.747412f,   0.587810f,  -0.309605f),
Vector3( -0.672672f,   0.587775f,  -0.449481f),
Vector3( -0.572070f,   0.587769f,  -0.572070f),
Vector3( -0.449481f,   0.587775f,  -0.672672f),
Vector3( -0.309605f,   0.587810f,  -0.747412f),
Vector3( -0.157796f,   0.587785f,  -0.793479f),
Vector3(  0.000000f,   0.587803f,  -0.809004f),
Vector3(  0.157796f,   0.587785f,  -0.793479f),
Vector3(  0.309605f,   0.587810f,  -0.747412f),
Vector3(  0.449481f,   0.587775f,  -0.672672f),
Vector3(  0.572070f,   0.587769f,  -0.572070f),
Vector3(  0.672672f,   0.587775f,  -0.449481f),
Vector3(  0.747412f,   0.587810f,  -0.309605f),
Vector3(  0.793479f,   0.587785f,  -0.157796f),
Vector3(  0.809004f,   0.587803f,   0.000000f),
Vector3(  0.793479f,   0.587785f,   0.157796f),
Vector3(  0.747412f,   0.587810f,   0.309605f),
Vector3(  0.672672f,   0.587775f,   0.449481f),
Vector3(  0.572070f,   0.587769f,   0.572070f),
Vector3(  0.449481f,   0.587775f,   0.672672f),
Vector3(  0.309605f,   0.587810f,   0.747412f),
Vector3(  0.157796f,   0.587785f,   0.793479f),
Vector3(  0.000000f,   0.500011f,   0.866019f),
Vector3( -0.168997f,   0.499990f,   0.849382f),
Vector3( -0.331402f,   0.500004f,   0.800106f),
Vector3( -0.481100f,   0.500000f,   0.720100f),
Vector3( -0.612379f,   0.499983f,   0.612379f),
Vector3( -0.720100f,   0.500000f,   0.481100f),
Vector3( -0.800106f,   0.500004f,   0.331402f),
Vector3( -0.849382f,   0.499990f,   0.168997f),
Vector3( -0.866019f,   0.500011f,   0.000000f),
Vector3( -0.849382f,   0.499990f,  -0.168997f),
Vector3( -0.800106f,   0.500004f,  -0.331402f),
Vector3( -0.720100f,   0.500000f,  -0.481100f),
Vector3( -0.612379f,   0.499983f,  -0.612379f),
Vector3( -0.481100f,   0.500000f,  -0.720100f),
Vector3( -0.331402f,   0.500004f,  -0.800106f),
Vector3( -0.168997f,   0.499990f,  -0.849382f),
Vector3(  0.000000f,   0.500011f,  -0.866019f),
Vector3(  0.168997f,   0.499990f,  -0.849382f),
Vector3(  0.331402f,   0.500004f,  -0.800106f),
Vector3(  0.481100f,   0.500000f,  -0.720100f),
Vector3(  0.612379f,   0.499983f,  -0.612379f),
Vector3(  0.720100f,   0.500000f,  -0.481100f),
Vector3(  0.800106f,   0.500004f,  -0.331402f),
Vector3(  0.849382f,   0.499990f,  -0.168997f),
Vector3(  0.866019f,   0.500011f,   0.000000f),
Vector3(  0.849382f,   0.499990f,   0.168997f),
Vector3(  0.800106f,   0.500004f,   0.331402f),
Vector3(  0.720100f,   0.500000f,   0.481100f),
Vector3(  0.612379f,   0.499983f,   0.612379f),
Vector3(  0.481100f,   0.500000f,   0.720100f),
Vector3(  0.331402f,   0.500004f,   0.800106f),
Vector3(  0.168997f,   0.499990f,   0.849382f),
Vector3(  0.000000f,   0.406723f,   0.913552f),
Vector3( -0.178202f,   0.406705f,   0.896011f),
Vector3( -0.349607f,   0.406708f,   0.844016f),
Vector3( -0.507512f,   0.406709f,   0.759618f),
Vector3( -0.645988f,   0.406692f,   0.645988f),
Vector3( -0.759618f,   0.406709f,   0.507512f),
Vector3( -0.844016f,   0.406708f,   0.349607f),
Vector3( -0.896011f,   0.406705f,   0.178202f),
Vector3( -0.913552f,   0.406723f,   0.000000f),
Vector3( -0.896011f,   0.406705f,  -0.178202f),
Vector3( -0.844016f,   0.406708f,  -0.349607f),
Vector3( -0.759618f,   0.406709f,  -0.507512f),
Vector3( -0.645988f,   0.406692f,  -0.645988f),
Vector3( -0.507512f,   0.406709f,  -0.759618f),
Vector3( -0.349607f,   0.406708f,  -0.844016f),
Vector3( -0.178202f,   0.406705f,  -0.896011f),
Vector3(  0.000000f,   0.406723f,  -0.913552f),
Vector3(  0.178202f,   0.406705f,  -0.896011f),
Vector3(  0.349607f,   0.406708f,  -0.844016f),
Vector3(  0.507512f,   0.406709f,  -0.759618f),
Vector3(  0.645988f,   0.406692f,  -0.645988f),
Vector3(  0.759618f,   0.406709f,  -0.507512f),
Vector3(  0.844016f,   0.406708f,  -0.349607f),
Vector3(  0.896011f,   0.406705f,  -0.178202f),
Vector3(  0.913552f,   0.406723f,   0.000000f),
Vector3(  0.896011f,   0.406705f,   0.178202f),
Vector3(  0.844016f,   0.406708f,   0.349607f),
Vector3(  0.759618f,   0.406709f,   0.507512f),
Vector3(  0.645988f,   0.406692f,   0.645988f),
Vector3(  0.507512f,   0.406709f,   0.759618f),
Vector3(  0.349607f,   0.406708f,   0.844016f),
Vector3(  0.178202f,   0.406705f,   0.896011f),
Vector3(  0.000000f,   0.308989f,   0.951066f),
Vector3( -0.185499f,   0.308999f,   0.932797f),
Vector3( -0.363984f,   0.308986f,   0.878660f),
Vector3( -0.528386f,   0.308992f,   0.790779f),
Vector3( -0.672502f,   0.309001f,   0.672502f),
Vector3( -0.790779f,   0.308992f,   0.528386f),
Vector3( -0.878660f,   0.308986f,   0.363984f),
Vector3( -0.932797f,   0.308999f,   0.185499f),
Vector3( -0.951066f,   0.308989f,   0.000000f),
Vector3( -0.932797f,   0.308999f,  -0.185499f),
Vector3( -0.878660f,   0.308986f,  -0.363984f),
Vector3( -0.790779f,   0.308992f,  -0.528386f),
Vector3( -0.672502f,   0.309001f,  -0.672502f),
Vector3( -0.528386f,   0.308992f,  -0.790779f),
Vector3( -0.363984f,   0.308986f,  -0.878660f),
Vector3( -0.185499f,   0.308999f,  -0.932797f),
Vector3(  0.000000f,   0.308989f,  -0.951066f),
Vector3(  0.185499f,   0.308999f,  -0.932797f),
Vector3(  0.363984f,   0.308986f,  -0.878660f),
Vector3(  0.528386f,   0.308992f,  -0.790779f),
Vector3(  0.672502f,   0.309001f,  -0.672502f),
Vector3(  0.790779f,   0.308992f,  -0.528386f),
Vector3(  0.878660f,   0.308986f,  -0.363984f),
Vector3(  0.932797f,   0.308999f,  -0.185499f),
Vector3(  0.951066f,   0.308989f,   0.000000f),
Vector3(  0.932797f,   0.308999f,   0.185499f),
Vector3(  0.878660f,   0.308986f,   0.363984f),
Vector3(  0.790779f,   0.308992f,   0.528386f),
Vector3(  0.672502f,   0.309001f,   0.672502f),
Vector3(  0.528386f,   0.308992f,   0.790779f),
Vector3(  0.363984f,   0.308986f,   0.878660f),
Vector3(  0.185499f,   0.308999f,   0.932797f),
Vector3(  0.000000f,   0.207910f,   0.978148f),
Vector3( -0.190793f,   0.207892f,   0.959364f),
Vector3( -0.374301f,   0.207900f,   0.903702f),
Vector3( -0.543410f,   0.207904f,   0.813315f),
Vector3( -0.691658f,   0.207888f,   0.691658f),
Vector3( -0.813315f,   0.207904f,   0.543410f),
Vector3( -0.903702f,   0.207900f,   0.374301f),
Vector3( -0.959364f,   0.207892f,   0.190793f),
Vector3( -0.978148f,   0.207910f,   0.000000f),
Vector3( -0.959364f,   0.207892f,  -0.190793f),
Vector3( -0.903702f,   0.207900f,  -0.374301f),
Vector3( -0.813315f,   0.207904f,  -0.543410f),
Vector3( -0.691658f,   0.207888f,  -0.691658f),
Vector3( -0.543410f,   0.207904f,  -0.813315f),
Vector3( -0.374301f,   0.207900f,  -0.903702f),
Vector3( -0.190793f,   0.207892f,  -0.959364f),
Vector3(  0.000000f,   0.207910f,  -0.978148f),
Vector3(  0.190793f,   0.207892f,  -0.959364f),
Vector3(  0.374301f,   0.207900f,  -0.903702f),
Vector3(  0.543410f,   0.207904f,  -0.813315f),
Vector3(  0.691658f,   0.207888f,  -0.691658f),
Vector3(  0.813315f,   0.207904f,  -0.543410f),
Vector3(  0.903702f,   0.207900f,  -0.374301f),
Vector3(  0.959364f,   0.207892f,  -0.190793f),
Vector3(  0.978148f,   0.207910f,   0.000000f),
Vector3(  0.959364f,   0.207892f,   0.190793f),
Vector3(  0.903702f,   0.207900f,   0.374301f),
Vector3(  0.813315f,   0.207904f,   0.543410f),
Vector3(  0.691658f,   0.207888f,   0.691658f),
Vector3(  0.543410f,   0.207904f,   0.813315f),
Vector3(  0.374301f,   0.207900f,   0.903702f),
Vector3(  0.190793f,   0.207892f,   0.959364f),
Vector3(  0.000000f,   0.104503f,   0.994525f),
Vector3( -0.194004f,   0.104502f,   0.975419f),
Vector3( -0.380606f,   0.104502f,   0.918814f),
Vector3( -0.552517f,   0.104503f,   0.826925f),
Vector3( -0.703235f,   0.104505f,   0.703235f),
Vector3( -0.826925f,   0.104503f,   0.552517f),
Vector3( -0.918814f,   0.104502f,   0.380606f),
Vector3( -0.975419f,   0.104502f,   0.194004f),
Vector3( -0.994525f,   0.104503f,   0.000000f),
Vector3( -0.975419f,   0.104502f,  -0.194004f),
Vector3( -0.918814f,   0.104502f,  -0.380606f),
Vector3( -0.826925f,   0.104503f,  -0.552517f),
Vector3( -0.703235f,   0.104505f,  -0.703235f),
Vector3( -0.552517f,   0.104503f,  -0.826925f),
Vector3( -0.380606f,   0.104502f,  -0.918814f),
Vector3( -0.194004f,   0.104502f,  -0.975419f),
Vector3(  0.000000f,   0.104503f,  -0.994525f),
Vector3(  0.194004f,   0.104502f,  -0.975419f),
Vector3(  0.380606f,   0.104502f,  -0.918814f),
Vector3(  0.552517f,   0.104503f,  -0.826925f),
Vector3(  0.703235f,   0.104505f,  -0.703235f),
Vector3(  0.826925f,   0.104503f,  -0.552517f),
Vector3(  0.918814f,   0.104502f,  -0.380606f),
Vector3(  0.975419f,   0.104502f,  -0.194004f),
Vector3(  0.994525f,   0.104503f,   0.000000f),
Vector3(  0.975419f,   0.104502f,   0.194004f),
Vector3(  0.918814f,   0.104502f,   0.380606f),
Vector3(  0.826925f,   0.104503f,   0.552517f),
Vector3(  0.703235f,   0.104505f,   0.703235f),
Vector3(  0.552517f,   0.104503f,   0.826925f),
Vector3(  0.380606f,   0.104502f,   0.918814f),
Vector3(  0.194004f,   0.104502f,   0.975419f),
Vector3(  0.000000f,   0.000000f,   1.000000f),
Vector3( -0.195097f,   0.000000f,   0.980784f),
Vector3( -0.382690f,   0.000000f,   0.923877f),
Vector3( -0.555577f,   0.000000f,   0.831465f),
Vector3( -0.707107f,   0.000000f,   0.707107f),
Vector3( -0.831465f,   0.000000f,   0.555577f),
Vector3( -0.923877f,   0.000000f,   0.382690f),
Vector3( -0.980784f,   0.000000f,   0.195097f),
Vector3( -1.000000f,   0.000000f,   0.000000f),
Vector3( -0.980784f,   0.000000f,  -0.195097f),
Vector3( -0.923877f,   0.000000f,  -0.382690f),
Vector3( -0.831465f,   0.000000f,  -0.555577f),
Vector3( -0.707107f,   0.000000f,  -0.707107f),
Vector3( -0.555577f,   0.000000f,  -0.831465f),
Vector3( -0.382690f,   0.000000f,  -0.923877f),
Vector3( -0.195097f,   0.000000f,  -0.980784f),
Vector3(  0.000000f,   0.000000f,  -1.000000f),
Vector3(  0.195097f,   0.000000f,  -0.980784f),
Vector3(  0.382690f,   0.000000f,  -0.923877f),
Vector3(  0.555577f,   0.000000f,  -0.831465f),
Vector3(  0.707107f,   0.000000f,  -0.707107f),
Vector3(  0.831465f,   0.000000f,  -0.555577f),
Vector3(  0.923877f,   0.000000f,  -0.382690f),
Vector3(  0.980784f,   0.000000f,  -0.195097f),
Vector3(  1.000000f,   0.000000f,   0.000000f),
Vector3(  0.980784f,   0.000000f,   0.195097f),
Vector3(  0.923877f,   0.000000f,   0.382690f),
Vector3(  0.831465f,   0.000000f,   0.555577f),
Vector3(  0.707107f,   0.000000f,   0.707107f),
Vector3(  0.555577f,   0.000000f,   0.831465f),
Vector3(  0.382690f,   0.000000f,   0.923877f),
Vector3(  0.195097f,   0.000000f,   0.980784f),
} ;
