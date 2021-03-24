#pragma once

#include "def.h"
#include "D3Ddef.h"
#include "GeoLib.h"
#include "Spline.h"
#include "DataStructLib.h"

struct TERRAINVERTEX ;
struct TERRAININDEX ;
struct STerrainIndex1 ;
class CSecretTerrain ;
class CSecretPicking ;
class CSecretTextureContainer ;

//로드 충돌
//이게 지형에 딱 달라붙은 데칼으므로 평면적인 관점으루다가 충돌처리함.
//2차원적으로 x-z평면에서 충돌을 확인

struct SRoadRectPlane
{
	geo::SLine avLines[4] ;
	bool Intersect(Vector3 &p) ;
	bool getDropPosition(Vector3 &p, Vector3 &vTargetPos, Vector3 *pvDropPos) ;
} ;

struct SRoadCirclePlane
{
	geo::SSphere sCircle ;
	int nNumRectPlane ;
	SRoadRectPlane *psRectPlanes ;

	SRoadCirclePlane() ;
	~SRoadCirclePlane() ;

	bool Intersect(Vector3 &vPos) ;
} ;

struct SRoadBoundingPlane
{
	int nNumCirclePlane ;
	SRoadCirclePlane *psCirclePlanes ;

	SRoadBoundingPlane() ;
	~SRoadBoundingPlane() ;

	bool Intersect(Vector3 &vPos) ;
	bool getDropPosition(Vector3 &vPos, Vector3 &vTargetPos, Vector3 *pvDropPos) ;
} ;

class CSecretRoad
{
public :
	enum STATUS {IDLING=0, BUILDING} ;
	enum ATTR {ATTR_ONTERRAIN=0x01 } ;
	int MAXNUM_POSITION ;

private :
	int m_nStatus, m_nAttr ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nNumPost, m_nNumPosition, m_nNumCVertex ;
	Vector3 *m_pvPosts ;
    D3DXVECTOR3 *m_pvPositions ;
	SCVertex *m_psCVertices ;
    CSecretTerrain *m_pcTerrain ;
	CSecretPicking *m_pcPicking ;
	CSpline m_cSpline ;

    TERRAINVERTEX *m_pTerrainVertices ;
	WORD *m_pwTerrainIndex ;
	int m_nNumTerrainVertex, m_nNumTerrainIndex ;
    int m_nTerrainRoadIndex ;

	SRoadBoundingPlane *m_psBoundingPlane ;

	bool _AddPosition(Vector3 vPos) ;
	void _BoxRender(geo::SAACube *psCube) ;
	void _WeldingVertex(TERRAINVERTEX **ppsVertices, int &nNumVertex, STerrainIndex1 *psIndices, int &nNumIndex) ;
	void _BuildBoundingPlane() ;

public :
	float m_fWidth, m_fBoxSize, m_fWeight, m_fTexFactor ;
	int m_nDivision ;
	DWORD m_dwVertexColor ;
	char m_szName[128], m_szRoadKind[128] ;
	LPDIRECT3DTEXTURE9 m_pTex ;

public :
	CSecretRoad() ;
	~CSecretRoad() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTerrain *pcTerrain, CSecretPicking *pcPicking, char *pszName, float width, int nDivision, LPDIRECT3DTEXTURE9 pTex, char *pszRoadKind) ;
	void Process(bool bPressedLButton) ;
	void Render() ;
	void AssistantRender() ;
	void Release() ;

    void BuildAssistant() ;
	void AlignOnTerrain() ;

	void SetStatus(int nStatus) ;
	int GetStatus() {return m_nStatus;}
	CSpline *GetSpline() {return &m_cSpline;}
	void Reset(float fWidth, int nDivision, LPDIRECT3DTEXTURE9 pTex, char *pszRoadKind) ;
	bool Intersect(Vector3 &vPos) ;
	bool getDropPosition(Vector3 &vPos, Vector3 &vTargetPos, Vector3 *pvDropPos) ;
} ;

struct SRoadKind
{
	char szName[128] ;
} ;

class CSecretRoadManager
{
public :
	const int MAXNUM_ROAD ;

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	data::SKeepingArray<CSecretRoad> m_cRoads ;
	CSecretTerrain *m_pcTerrain ;
	CSecretPicking *m_pcPicking ;
	CSecretTextureContainer *m_pcTexContainer ;
	CSecretRoad *m_pcCurrentRoad ;

public :
	bool m_bRender ;
	int m_nNumRoadKind ;
	SRoadKind *m_psRoadKinds ;

public :
	CSecretRoadManager() ;
	~CSecretRoadManager() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretPicking *pcPicking) ;
	void Process(bool bPressedLButton) ;
	void Render() ;
	void Release() ;

	void CreateRoad(char *pszName, float width, int nDivision, char *pszRoadKind) ;
	void ResetCurrentRoad(float fWidth, int nDivision, char *pszRoadKind) ;
	CSecretRoad *SelectRoad(int nNum) ;
	void DeleteRoad(int nNum) ;
	CSecretRoad *GetCurrentRoad() {return m_pcCurrentRoad;}
	data::SKeepingArray<CSecretRoad> *GetRoads() {return &m_cRoads;}
	CSecretTextureContainer *GetTexContainer() {return m_pcTexContainer;}
	void ImportRoad(char *pszName, float width, int nDivision, char *pszRoadKind, SPLINEDATA *psSplineData, int nNumSplineData) ;

	void AlignOnTerrain() ;
	bool Intersect(Vector3 vPos) ;
	bool getDropPosition(Vector3 vPos, Vector3 vTargetPos, Vector3 *pvDropPos) ;

	void SetTerrain(CSecretTerrain *pcTerrain) ;
	void ResetRoads() ;
} ;