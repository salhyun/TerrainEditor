#include "SecretSceneManager.h"
#include "D3DMathLib.h"
#include "SecretMeshObject.h"
#include "SecretQuadTree.h"
#include <list>

STrueRenderingObject::STrueRenderingObject() :
//DISTANCE_LOD1(60.0f),
//DISTANCE_LOD2(100.0f),
//DISTANCE_LOD3(400.0f)
DISTANCE_LOD1(60.0f),
DISTANCE_LOD2(80.0f),
DISTANCE_LOD3(400.0f)
{
	int i ;
	dwFlags = 0 ;
	dwAcquiredFlag = 0 ;
	nCurLevel = 0 ;
	for(i=0; i<MAXNUM_LODOBJECT; i++) { apcObject[i]=NULL; }
	nNumTreeItem = nNumQuadTree = 0 ;
	fDistfromCamera = 0 ;
	nEigenIndex = -1 ;
	fCoefConstantScaling = 1.0f ;
	for(i=0; i<12; i++) { apcQuadTrees[i]=NULL; }

	D3DXMatrixIdentity(&matWorld) ;
	D3DXMatrixIdentity(&matTrans) ;
}
STrueRenderingObject::~STrueRenderingObject()
{
}
void STrueRenderingObject::ResetVariable()
{
	dwFlags = 0 ;
	dwAcquiredFlag = 0 ;
	nCurLevel = 0 ;
	nNumTreeItem = nNumQuadTree = 0 ;
	fDistfromCamera = 0 ;
	nEigenIndex = -1 ;
	D3DXMatrixIdentity(&matWorld) ;
	D3DXMatrixIdentity(&matTrans) ;
}
void STrueRenderingObject::SetEnable(bool enable)
{
	if(enable)
		dwFlags |= FLAG_ENABLE ;
	else
		dwFlags &= (~FLAG_ENABLE) ;
}
bool STrueRenderingObject::IsEnable()
{
	if(dwFlags&FLAG_ENABLE)	return true ;
	else return false ;
}
void STrueRenderingObject::SetSelected(bool selected)
{
	if(selected)
		dwFlags |= FLAG_SELECTED ;
	else
		dwFlags &= (~FLAG_SELECTED) ;
}
bool STrueRenderingObject::IsSelected()
{
	if(dwFlags&FLAG_SELECTED) return true ;
	else return false ;
}
void STrueRenderingObject::SetInFrustum(bool in)
{
	if(in)
		dwFlags |= FLAG_INFRUSTUM ;
	else
		dwFlags &= (~FLAG_INFRUSTUM) ;
}
bool STrueRenderingObject::IsInFrustum()
{
	if(dwFlags&FLAG_INFRUSTUM) return true ;
	else return false ;
}
void STrueRenderingObject::SetLevelofDetail(bool lod)
{
	if(lod)
		dwFlags |= FLAG_LODENABLE ;
	else
		dwFlags &= (~FLAG_LODENABLE) ;
}
bool STrueRenderingObject::IsLODEnable()
{
	if(dwFlags&FLAG_LODENABLE) return true ;
	else return false ;
}
void STrueRenderingObject::SetFlag(DWORD dwFlag, bool bEnable)
{
	if(bEnable)
		dwFlags |= dwFlag ;
	else
		dwFlags &= (~dwFlag) ;
}
bool STrueRenderingObject::IsFlag(DWORD dwFlag)
{
	if(dwFlags & dwFlag) return true ;
	else return false ;
}
void STrueRenderingObject::SetAcquiredFlag(DWORD dwFlag, bool bEnable)
{
	if(bEnable)
		dwAcquiredFlag |= dwFlag ;
	else
		dwAcquiredFlag &= (~dwFlag) ;
}
bool STrueRenderingObject::IsAcquiredFlag(DWORD dwFlag)
{
	if(dwAcquiredFlag & dwFlag) return true ;
	else return false ;
}
bool STrueRenderingObject::IsBillboardObject(int nLevel)
{
	if((apcObject[nLevel]->GetNumMesh() == 1) && (fDistfromCamera > DISTANCE_LOD2) && ( ((CSecretMesh *)apcObject[nLevel]->GetMesh(0))->GetNodeKind() == SECRETNODEKIND_BILLBOARD ))
		return true ;

	return false ;
}
void STrueRenderingObject::SetDistance(float d)
{
	fDistfromCamera = d ;
	SetFlag(FLAG_SHADOWENABLE) ;
	SetFlag(FLAG_ALPHATESTBYLOD, false) ;

	if(!IsLODEnable())
		return ;

	if(IsAcquiredFlag(ACQUIREDFLAG_ALWAYSBILLBOARD))
	{
		nCurLevel = 1 ;
		return ;
	}

	char *pszObjName = apcObject[0]->GetObjName() ;
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
		if(fDistfromCamera >= DISTANCE_LOD2)//lod 에 의한 알파테스트
		{
			nCurLevel = 0 ;
			SetFlag(FLAG_ALPHATESTBYLOD) ;
		}
		else
			nCurLevel = 0 ;

		return ;
	}

	//여기에서 거리에 맞는 LOD를 계산해준다.
	if(fDistfromCamera >= DISTANCE_LOD3)//그림자없는 빌보드
	{
		nCurLevel = 1 ;
		SetFlag(FLAG_SHADOWENABLE, false) ;
	}
	else if(fDistfromCamera >= DISTANCE_LOD2)//그림자있는 빌보드
	{
		//if(!strcmp(pszObjName, "04_bud_s1_20m_13920")
		//	|| !strcmp(pszObjName, "04_bud_s2_20m_13920")
		//	|| !strcmp(pszObjName, "04_bud_y1_17m_12134")
		//	|| !strcmp(pszObjName, "04_bud_y2_25m_13280")
		//	|| !strcmp(pszObjName, "04_bud_y3_12m_06494"))
		//{
		//	if(fDistfromCamera > 300.0f)
		//		nCurLevel = 1 ;
		//	else
		//		nCurLevel = 0 ;
		//}
		//else
		{
			nCurLevel = 1 ;
			if(!strcmp(apcObject[0]->GetCatalogName(), "flowers"))//no shadow flowers
				SetFlag(FLAG_SHADOWENABLE, false) ;
		}
	}
	//else if(fDistfromCamera >= DISTANCE_LOD1)//lod 에 의한 알파테스트
	//{
	//	nCurLevel = 0 ;
	//	SetFlag(FLAG_ALPHATESTBYLOD) ;
	//}
	else
	{
		nCurLevel = 0 ;
		//if(fDistfromCamera >= DISTANCE_LOD1)
		//	nCurLevel = 1 ;
		//else
		//	nCurLevel = 0 ;
	}
}
void STrueRenderingObject::SetDistance(float d, Vector3 *pvCamera)
{
	fDistfromCamera = d ;
	SetFlag(FLAG_SHADOWENABLE) ;
	SetFlag(FLAG_ALPHATESTBYLOD, false) ;

	if(!IsLODEnable())
		return ;

	if(IsAcquiredFlag(ACQUIREDFLAG_ALWAYSBILLBOARD))
	{
		nCurLevel = 1 ;
		return ;
	}

	char *pszObjName = apcObject[0]->GetObjName() ;
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
		if(fDistfromCamera >= DISTANCE_LOD2)//lod 에 의한 알파테스트
		{
			nCurLevel = 0 ;
			SetFlag(FLAG_ALPHATESTBYLOD) ;
		}
		else
			nCurLevel = 0 ;
		return ;
	}

	//여기에서 거리에 맞는 LOD를 계산해준다.
	//여기에서 거리에 맞는 LOD를 계산해준다.
	if(fDistfromCamera >= DISTANCE_LOD3)//그림자없는 빌보드
	{
		DecideLevelfromTwoBillboard(pvCamera) ;
		SetFlag(FLAG_SHADOWENABLE, false) ;
	}
	else if(fDistfromCamera >= DISTANCE_LOD2)//그림자있는 빌보드
	{
		//if(!strcmp(pszObjName, "04_bud_s1_20m_13920")
		//	|| !strcmp(pszObjName, "04_bud_s2_20m_13920")
		//	|| !strcmp(pszObjName, "04_bud_y1_17m_12134")
		//	|| !strcmp(pszObjName, "04_bud_y2_25m_13280")
		//	|| !strcmp(pszObjName, "04_bud_y3_12m_06494"))
		//{
		//	if(fDistfromCamera > 300.0f)
		//		nCurLevel = 1 ;
		//	else
		//		nCurLevel = 0 ;
		//}
		//else
		{
			DecideLevelfromTwoBillboard(pvCamera) ;
			if(!strcmp(apcObject[0]->GetCatalogName(), "flowers"))//no shadow flowers
				SetFlag(FLAG_SHADOWENABLE, false) ;
		}
	}
	//else if(fDistfromCamera >= DISTANCE_LOD1)//lod 에 의한 알파테스트
	//{
	//	nCurLevel = 0 ;
	//	SetFlag(FLAG_ALPHATESTBYLOD) ;
	//}
	else
	{
		nCurLevel = 0 ;
	}

	/*
	if(fDistfromCamera >= DISTANCE_LOD2)
	{
		Vector3 pos(matWorld._41, matWorld._42, matWorld._43) ;
		Vector3 vXAxis(matWorld._11, matWorld._12, matWorld._13) ;

		float angle = fabs(IncludedAngle(((*pvCamera)-pos).Normalize(), vXAxis.Normalize())) ;
		if(float_less_eq(angle, quarter_pi) || float_greater_eq(angle, 2.356194f))
			nCurLevel = 2 ;
		else
			nCurLevel = 1 ;

		SetFlag(FLAG_SHADOWENABLE, false) ;
	}
	else
	{
		if(fDistfromCamera >= DISTANCE_LOD1)
		{
			Vector3 pos(matWorld._41, matWorld._42, matWorld._43) ;
			Vector3 vXAxis(matWorld._11, matWorld._12, matWorld._13) ;

			float angle = fabs(IncludedAngle(((*pvCamera)-pos).Normalize(), vXAxis.Normalize())) ;
			if(float_less_eq(angle, quarter_pi) || float_greater_eq(angle, 2.356194f))
				nCurLevel = 2 ;
			else
				nCurLevel = 1 ;
		}
		else
			nCurLevel = 0 ;
	}
	*/
}
void STrueRenderingObject::DecideLevelfromTwoBillboard(Vector3 *pvCamera)
{
	Vector3 pos(matWorld._41, matWorld._42, matWorld._43) ;
	Vector3 vXAxis(matWorld._11, matWorld._12, matWorld._13) ;

	float angle = fabs(IncludedAngle(((*pvCamera)-pos).Normalize(), vXAxis.Normalize())) ;
	if(float_less_eq(angle, quarter_pi) || float_greater_eq(angle, 2.356194f))
		nCurLevel = 2 ;
	else
		nCurLevel = 1 ;
}
void STrueRenderingObject::CalculateConstantScaling(D3DXMATRIX *pmatVP)
{
	D3DXMATRIX matS, matOriS, matOriR, matOriT ;
	D3DXVECTOR4 vResult ;

	D3DXMatrixScaling(&matOriS, 0.1f, 0.1f, 0.1f) ;
	D3DXMatrixDecomposeRotation(&matOriR, &matWorld) ;
	D3DXMatrixDecomposeTranslation(&matOriT, &matWorld) ;

	D3DXVECTOR3 vPos(matWorld._41, matWorld._42, matWorld._43) ;
	D3DXVec3Transform(&vResult, &vPos, pmatVP) ;

	float scale = vResult.w * fCoefConstantScaling ;
	D3DXMatrixScaling(&matS, scale, scale, scale) ;
	matWorld = matS * matOriS * matOriR * matOriT ;
}
void STrueRenderingObject::DecomposeMatrix()
{
	D3DXMatrixScaling(&matOriginScale, 0.1f, 0.1f, 0.1f) ;
	D3DXMatrixDecomposeRotation(&matOriginRot, &matWorld) ;
	D3DXMatrixDecomposeTranslation(&matOriginTrans, &matWorld) ;
}
void STrueRenderingObject::ClearLinktoQuadTree()
{
	int nFindCount ;
	std::list<STrueRenderingObject *>::iterator it, it_erase[8] ;
	for(int n=0 ; n<nNumQuadTree ; n++)
	{
		//traveling quadtree
		nFindCount = 0 ;
		for(it=apcQuadTrees[n]->m_sTRObjects.begin(); it!=apcQuadTrees[n]->m_sTRObjects.end(); it++)
		{
			if(nEigenIndex == (*it)->nEigenIndex)
				it_erase[nFindCount++] = it ;
		}
		if(nFindCount)
		{
			assert(nFindCount == 1) ;
			for(int erase_count=0 ; erase_count<nFindCount ; erase_count++)
				apcQuadTrees[n]->m_sTRObjects.erase(it_erase[erase_count]) ;
		}
		else
			assert(false && "picked object found out more that one in quadtree") ;
	}
	nNumQuadTree=0 ;
}
Vector3 STrueRenderingObject::GetPos()
{
	return Vector3(matWorld._41, matWorld._42, matWorld._43) ;
}

#include "coldet.h"

int STrueRenderingObject::getDistancefrom(Vector3 pos, Vector3 *pvIntersect)
{
	int i ;
	float m[16], ori[3], dir[3] ;
	CSecretMeshObject *pcMeshObject = apcObject[0] ;
	CSecretMesh *pcMesh ;
	CollisionModel3D *pcCollision3d ;
	geo::SLine sline ;
	Vector3 a ;
	for(i=0 ; i<pcMeshObject->GetNumMesh() ; i++)
	{
		pcMesh = pcMeshObject->GetMesh(i) ;
		pcCollision3d = pcMesh->GetCollisionModel() ;
		if(pcCollision3d == NULL)
			continue ;

		m[0] =  matWorld._11 ; m[1]  = matWorld._12 ; m[2]  = matWorld._13 ; m[3]  = matWorld._14 ;
		m[4] =  matWorld._21 ; m[5]  = matWorld._22 ; m[6]  = matWorld._23 ; m[7]  = matWorld._24 ;
		m[8] =  matWorld._31 ; m[9]  = matWorld._32 ; m[10] = matWorld._33 ; m[11] = matWorld._34 ;
		m[12] = matWorld._41 ; m[13] = matWorld._42 ; m[14] = matWorld._43 ; m[15] = matWorld._44 ;
		pcCollision3d->setTransform(m) ;

		a.set(matWorld._41-pos.x, matWorld._42-pos.y, matWorld._43-pos.z) ;
		sline.set(pos, a, (a-pos).Magnitude()) ;

		ori[0] = pos.x ;
		ori[1] = pos.y ;
		ori[2] = pos.z ;

		dir[0] = sline.v.x ;
		dir[1] = sline.v.y ;
		dir[2] = sline.v.z ;

		if(pcCollision3d->rayCollision(ori, dir, true))
		{
			pcCollision3d->getCollisionPoint(ori, false) ;
			pvIntersect->set(ori[0], ori[1], ori[2]) ;
			return 1 ;
		}
	}
	return 0 ;
}
int STrueRenderingObject::getIntersectPoswithPolygons(Vector3 vStart, Vector3 vDir, Vector3 *pvIntersect)
{
	int i ;
	float m[16], ori[3], dir[3] ;
	CSecretMeshObject *pcMeshObject = apcObject[0] ;
	CSecretMesh *pcMesh ;
	CollisionModel3D *pcCollision3d ;
	for(i=0 ; i<pcMeshObject->GetNumMesh() ; i++)
	{
		pcMesh = pcMeshObject->GetMesh(i) ;
		pcCollision3d = pcMesh->GetCollisionModel() ;
		if(pcCollision3d == NULL)
			continue ;

		m[0] =  matWorld._11 ; m[1]  = matWorld._12 ; m[2]  = matWorld._13 ; m[3]  = matWorld._14 ;
		m[4] =  matWorld._21 ; m[5]  = matWorld._22 ; m[6]  = matWorld._23 ; m[7]  = matWorld._24 ;
		m[8] =  matWorld._31 ; m[9]  = matWorld._32 ; m[10] = matWorld._33 ; m[11] = matWorld._34 ;
		m[12] = matWorld._41 ; m[13] = matWorld._42 ; m[14] = matWorld._43 ; m[15] = matWorld._44 ;
		pcCollision3d->setTransform(m) ;

		ori[0] = vStart.x ;
		ori[1] = vStart.y ;
		ori[2] = vStart.z ;

		dir[0] = vDir.x ;
		dir[1] = vDir.y ;
		dir[2] = vDir.z ;

		if(pcCollision3d->rayCollision(ori, dir, true))
		{
			pcCollision3d->getCollisionPoint(ori, false) ;
			pvIntersect->set(ori[0], ori[1], ori[2]) ;
			return 1 ;
		}
	}
	return 0 ;
}
int STrueRenderingObject::getIntersectPoswithBoundingBox(geo::SLine &sline, Vector3 *pvIntersect)
{
	if(cBBox.IntersectLine(&sline, pvIntersect))
		return 1 ;
	return 0 ;
}