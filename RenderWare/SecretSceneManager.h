#pragma once

#include <stdio.h>
#include "D3Ddef.h"
#include "GeoLib.h"
#include "DataStructLib.h"
#include "SecretBoundingBox.h"

class CSecretMeshObject ;
class CSecretQuadTree ;

#define MAXNUM_LODOBJECT 3
#define MAXNUM_ITEMTEXT 16
#define MAXNUM_INCLUDEDTILES 64
//#define DISTANCE_LOD1 80.0f
//#define DISTANCE_LOD2 160.0f

struct STrueRenderingObject
{ 
	enum FLAGS
	{
		FLAG_ENABLE=0x01,
		FLAG_SELECTED=0x02,
		FLAG_INFRUSTUM=0x04,
		FLAG_LODENABLE=0x08,
		FLAG_TWOBILLBOARD=0x10,
		FLAG_SHADOWENABLE=0x20,
		FLAG_ALPHATESTBYLOD=0x40,
		FLAG_HIDE=0x80,
		FLAG_ALPHABLENDINGBYFORCE=0x100 //강제적인 알파블렌딩
	} ;
	enum ACQUIREDFLAG { ACQUIREDFLAG_ALWAYSBILLBOARD=0x01, ACQUIREDFLAG_CONSTANTSCALE=0x02, ACQUIREDFLAG_COLLISIONVOLUME=0x04 } ;

	const float DISTANCE_LOD1, DISTANCE_LOD2, DISTANCE_LOD3 ;

	DWORD dwFlags ;
	DWORD dwAcquiredFlag ;
	D3DXMATRIX matWorld ;//픽킹이 되면 그 상태를 matOrigin에 저장하고 기즈모에 의해 변환되는 것을 matWorld에 저장 한다. 선택해제 되면 matWorld에 저장.
	D3DXMATRIX matTrans ;//transforming in constant scaling
	D3DXMATRIX matOriginRot, matOriginTrans, matOriginScale ;
	int nCategory ;
	int nReference ;
	int nNumTreeItem ;
	data::SCString asCStrTreeItem[MAXNUM_ITEMTEXT] ;
	int nNumLOD, nCurLevel ;
	CSecretMeshObject *apcObject[MAXNUM_LODOBJECT] ;
	CSecretBoundingBox cBBox ;
	geo::SSphere sBoundingSphere ;
	float fDistfromCamera ;
	int nEigenIndex ;
	float fCoefConstantScaling ;

	int nNumQuadTree ;
	CSecretQuadTree *apcQuadTrees[MAXNUM_INCLUDEDTILES] ;

	SContactSurfaceInfo sContactSurfaceInfo ;

	STrueRenderingObject() ;
	~STrueRenderingObject() ;

	void SetEnable(bool enable) ;
	bool IsEnable() ;
	void SetSelected(bool selected) ;
	bool IsSelected() ;
	void SetInFrustum(bool include) ;
	bool IsInFrustum() ;
	void SetLevelofDetail(bool lod) ;
	bool IsLODEnable() ;

	void SetFlag(DWORD dwFlag, bool bEnable=true) ;
	bool IsFlag(DWORD dwFlag) ;

	void SetAcquiredFlag(DWORD dwFlag, bool bEnable=true) ;
	bool IsAcquiredFlag(DWORD dwFlag) ;
	bool IsBillboardObject(int nLevel) ;//one billboard mesh

	void SetDistance(float d) ;
	void SetDistance(float d, Vector3 *pvCamera) ;
    void DecideLevelfromTwoBillboard(Vector3 *pvCamera) ;
	void CalculateConstantScaling(D3DXMATRIX *pmatVP) ;
	void DecomposeMatrix() ;
	void ResetVariable() ;
	Vector3 GetPos() ;
	int getDistancefrom(Vector3 pos, Vector3 *pvIntersect) ;//오브젝트의 중심점에서 충돌된 삼각형에서의 거리(매우 디테일함)
	int getIntersectPoswithPolygons(Vector3 vStart, Vector3 vDir, Vector3 *pvIntersect) ;//직선과 오브젝트의 충돌지점
	int getIntersectPoswithBoundingBox(geo::SLine &sline, Vector3 *pvIntersect) ;//직선과 오브젝트의 충돌지점


	void ClearLinktoQuadTree() ;
} ;

class CSecretSceneManager
{
private :
	char m_szTerrainFileName[256] ;

	int m_nNumObjKind ;
	CSecretMeshObject *m_psObjects ;

    int m_nNumTrueRenderingObj ;
    STrueRenderingObject *m_psTrueRenderingObj ;

public :
	CSecretSceneManager() ;
	~CSecretSceneManager() ;

	bool Initialize() ;

	bool Export() ;
	bool Import() ;
} ;