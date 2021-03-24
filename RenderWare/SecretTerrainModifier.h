#pragma once

#include "def.h"
#include "D3Ddef.h"
#include "Vector3.h"
#include "GeoLib.h"
#include "SecretPicking.h"
#include "SecretDecal.h"
#include "SecretTimer.h"


class CSecretTerrain ;
class CSecretTile ;
class CSecretTileSubTree ;
struct STerrainTilePos ;

class CSecretTerrainModifier
{
public :
	enum KIND_MODIFY {KIND_RAISE=1, KIND_LOWER, KIND_SMOOTH, KIND_FLATTEN, KIND_RUGGED, KIND_HOLECUP} ;
	enum STRENGTHTYPE {STRENGTHTYPE_DIRECTLY=0, STRENGTHTYPE_GAUSSIAN} ;
	enum ATTR {SNAPGRID=0x01 } ;
	enum FLATTEN_ALIGN { LOWEST=0, MIDDLE, HIGHEST} ;

	const int MAXNUM_DISALLOW ;

private :
	bool m_bEnable, m_bProcessed ;
	int m_nKindofModify, m_nStrengthType, m_nAttr, m_nFlattenAlign ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	CSecretTerrain *m_pcTerrain ;
	CSecretPicking *m_pcPicking ;
	SPickingVertex m_vtCurPicking, m_vtPrevPicking ;

	Vector3 *m_pvBoundaryPos1 ;
	Matrix4 *m_pmatView ;
	D3DXMATRIX *m_pmatProj ;
    D3DXVECTOR3 *m_pvBoundaryPos2 ;
	LPD3DXLINE m_pLine ;

	int m_nDisallowCount ;
    STerrainTilePos *m_psDisallowPos ;

	void _SmoothAround(STerrainTilePos *psStartPos, STerrainTilePos *psEndPos, STerrainTilePos *psDisallowPos, int nDisallowCount) ;
	float _AvgHeight(int x, int z, int nSize) ;

	void _ModifyTerrain_LButton(int nKind) ;
	void _ModifyTerrain_RButton(int nKind) ;

public :
	int m_nNumBoundary ;
	float m_fRadius ;
	Vector3 m_vCenter ;
	float m_fStrength, m_fArea ;
	float m_fRuggedRange ;
	float m_fSmoothStrength ;

public :
	CSecretTerrainModifier() ;
	~CSecretTerrainModifier() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretPicking *pcPicking) ;
	void Process(bool bLButtonPressed, bool bRButtonPressed, Matrix4 *pmatView, D3DXMATRIX *pmatProj) ;
	void Render() ;

	void SetModifyKind(int nKind) ;
	int GetModifyKind() {return m_nKindofModify;}

	void SetEnable(bool bEnable=true) {m_bEnable=bEnable;}
	bool IsEnable() {return m_bEnable;}

	void SetStrength(float strength) ;
	void SetSize(float fRadius) ;
	void Release() ;
	void SetTerrain(CSecretTerrain *pcTerrain) {m_pcTerrain=pcTerrain;}
	void SetStrengthType(int nType) ;

	void AddAttr(int nAttr, bool bEnable=true) ;
	int GetAttr() ;
	bool IsAttr(int nAttr) ;

	void CreateHolecup(Vector3 vPos) ;

	void SetFlattenAlign(int nAlign) {m_nFlattenAlign=nAlign;}
	int GetFlattenAlign() {return m_nFlattenAlign;}

	void SetSmoothStr(int nStr) ;
	float GetSmoothStr() ;
} ;