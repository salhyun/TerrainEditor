#pragma once

#include "def.h"
#include "D3Ddef.h"
#include "Vector3.h"
#include "GeoLib.h"
#include "SecretPicking.h"
#include "SecretDecal.h"
#include "SecretTimer.h"

class CSecretTerrain ;
class CSecretToolBrush ;
class CSecretTile ;
class CSecretTileSubTree ;
class CD3DEnvironment ;

#define MAXNUM_INTERSECTEDTRIANGLES 32768

struct SThreadParameters
{
	int nIndex ;
	bool bIntersected ;
	bool bExitThread ;
	CSecretToolBrush *pcToolBrush ;
	HANDLE hThread ;
    DWORD dwThreadID ;
	int nStart, nEnd ;

	SThreadParameters() ;
	SThreadParameters(int _nStart, int _nEnd) ;
	~SThreadParameters() ;
	void EndThread() ;
} ;

class CSecretToolBrush
{
public :
	enum BRUSHTYPE{SQUARE=0, CIRCLE} ;
	enum BRUSHATTR { DRAWBRUSH=0x01, SNAPGRID=0x02 } ;
	enum BRUSHING_STATUS { NO_BRUSHING=0, START_BRUSHING, BRUSHING, END_BRUSHING } ;
	enum BRUSHING_DECISION { IDLEING=0, COLORING=1, ERASING=2 } ;

public :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nBrushType ;
	float m_fRadius ;
	Vector3 m_vCenterPos, *m_pvBoundaryPos1 ;
	Matrix4 *m_pmatView ;
	D3DXMATRIX *m_pmatProj ;
    D3DXVECTOR3 *m_pvBoundaryPos2 ;
	LPD3DXLINE m_pLine ;
	CSecretPicking *m_pcPicking ;

	SPickingVertex m_vtCurPicking, m_vtPrevPicking ;

	//bool m_bExitThread ;
    //bool m_bBuildDecal ;

	//HANDLE m_hThread ;
	//DWORD m_dwThreadID ;
	//int m_nNumThread ;
	//SThreadParameters *m_psThreadParameters ;
	int m_nBrushingStatus ;
	int m_nSelectedTexNum ;
	int m_nBrushDecision ;
	int m_nAttr ;

    float m_fBrushHasCounted, m_fElapsedTime, m_fBrushMustCount, m_fBrushCountPerSec ;
	int m_nBrushCurCount ;

	geo::STriangle *m_psIntersectedTriangles ;

public :
	int m_nNumBoundary ;
    CSecretTerrain *m_pcTerrain ;
	//CSecretDecal m_cDecal ;
	bool m_abIntersected[32] ;
	bool m_bIntersectedAll, m_bEnable, m_bProcessed ;

public :
	CSecretToolBrush() ;
	~CSecretToolBrush() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CD3DEnvironment *pd3dEnvironment, CSecretPicking *pcPicking, int nBrushType=CSecretToolBrush::CIRCLE, float fRadius=1) ;
	void PreComputeBrushPos(Matrix4 *pmatView, D3DXMATRIX *pmatProj) ;
	void ComputeBrushPos(float fTimeSinceLastFrame, Vector3 *pvCamera, char *pszMsg) ;
	void Render() ;//do not use in hardware shader, use in fixed-pile

	void Release() ;

	//void EndThread() ;

	void ResetIntersected() ;
	bool IntersectedQuadTreeTile(Vector3 vTileCenter, float fTileRadius, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, int nTriangles) ;
	bool IntersectedQuadTreeTile(Vector3 vTileCenter, float fTileRadius, int nPatchType, CSecretTile *pcTile) ;
	bool IntersectedQuadTreeTile(Vector3 vTileCenter, float fTileRadius, int nNumLevel, int nPatchType, int tl, CSecretTileSubTree *pcSubTree) ;
	void SettleRemainPoint() ;

	void SetBrushDecision(int nDecision) ;
	int GetBrushDecision() ;

	void SetRadius(float fRadius) {m_fRadius=fRadius;}
	void SetSize(float fRadius) ;
	void SetStrength(int nStrength) ;
	float GetRadius() {return m_fRadius;}
	void SetEnable(bool bEnable=true) ;
	bool IsEnable() {return m_bEnable;}
	void SetTerrain(CSecretTerrain *pcTerrain) ;
	void AddAttr(int nAttr, bool bEnable=true) ;
	int GetAttr() ;
	bool IsAttr(int nAttr) ;
} ;